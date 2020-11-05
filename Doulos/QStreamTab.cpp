
#include "QStreamTab.h"

#include <Doulos/MainWindow.h>
#include <Doulos/QOperationTab.h>
#include <Doulos/QDeviceControlTab.h>
#include <Doulos/QVisualizationTab.h>

#include <Doulos/Dialog/FlimCalibDlg.h>

#include <DataAcquisition/DataAcquisition.h>
#include <DataAcquisition/ThreadManager.h>

#include <DataAcquisition/FLImProcess/FLImProcess.h>

#include <DeviceControl/GalvoScan/GalvoScan.h>
#include <DeviceControl/ZaberStage/ZaberStage.h>

#include <MemoryBuffer/MemoryBuffer.h>

#include <iostream>
#include <mutex>
#include <condition_variable>


QStreamTab::QStreamTab(QWidget *parent) :
    QDialog(parent)
{
	// Set main window objects
    m_pMainWnd = dynamic_cast<MainWindow*>(parent);
    m_pConfig = m_pMainWnd->m_pConfiguration;

	// Create message window
	m_pListWidget_MsgWnd = new QListWidget(this);
	m_pListWidget_MsgWnd->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	m_pListWidget_MsgWnd->setStyleSheet("font: 7pt");
	m_pConfig->msgHandle += [&](const char* msg) {
		QString qmsg = QString::fromUtf8(msg);
		emit sendStatusMessage(qmsg, false);
	};
	connect(this, SIGNAL(sendStatusMessage(QString, bool)), this, SLOT(processMessage(QString, bool)));

    // Create streaming objects
    m_pOperationTab = new QOperationTab(this);
    m_pDeviceControlTab = new QDeviceControlTab(this);
    m_pVisualizationTab = new QVisualizationTab(m_pConfig, this);

    // Create image size widgets
    m_pLabel_ImageSize = new QLabel(this);
    m_pLabel_ImageSize->setText("Image Size");

    m_pRadioButton_128by128 = new QRadioButton(this);
    m_pRadioButton_128by128->setText("128 X 128");
    m_pRadioButton_256by256 = new QRadioButton(this);
    m_pRadioButton_256by256->setText("256 X 256");
    m_pRadioButton_512by512 = new QRadioButton(this);
    m_pRadioButton_512by512->setText("512 X 512");
    m_pRadioButton_1024by1024 = new QRadioButton(this);
    m_pRadioButton_1024by1024->setText("1024 X 1024");

    m_pButtonGroup_ImageSize = new QButtonGroup(this);
    m_pButtonGroup_ImageSize->addButton(m_pRadioButton_128by128, IMAGE_SIZE_128);
    m_pButtonGroup_ImageSize->addButton(m_pRadioButton_256by256, IMAGE_SIZE_256);
    m_pButtonGroup_ImageSize->addButton(m_pRadioButton_512by512, IMAGE_SIZE_512);
    m_pButtonGroup_ImageSize->addButton(m_pRadioButton_1024by1024, IMAGE_SIZE_1024);

    switch (m_pConfig->imageSize)
    {
    case IMAGE_SIZE_128:
        m_pRadioButton_128by128->setChecked(true);
        break;
    case IMAGE_SIZE_256:
        m_pRadioButton_256by256->setChecked(true);
        break;
    case IMAGE_SIZE_512:
        m_pRadioButton_512by512->setChecked(true);
        break;
    case IMAGE_SIZE_1024:
        m_pRadioButton_1024by1024->setChecked(true);
        break;
    default:
        m_pRadioButton_128by128->setChecked(true);
        break;
    }

	// Create averaing widgets
	m_pLabel_Averaging = new QLabel(this);
	m_pLabel_Averaging->setText("The Number of Frames for Averaging");
	m_pLabel_Averaging->setFixedWidth(200);

	m_pLineEdit_Averaging = new QLineEdit(this);
	m_pLineEdit_Averaging->setFixedWidth(25);
	m_pLineEdit_Averaging->setText(QString::number(m_pConfig->imageAveragingFrames));
	m_pLineEdit_Averaging->setAlignment(Qt::AlignCenter);
	m_pLineEdit_Averaging->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	m_pLabel_AcquisitionStatus = new QLabel("[Acquisition Status]", this);
	m_pLabel_AcquisitionStatusMsg = new QLabel(this);
	QString str; str.sprintf("Written Samples : %7d / %7d,   Average : %3d / %3d", 0, m_pConfig->imageSize * m_pConfig->imageSize, 0, m_pConfig->imageAveragingFrames);
	m_pLabel_AcquisitionStatusMsg->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	m_pLabel_AcquisitionStatusMsg->setText(str);

	// Image size & averaging initialization
	changeImageSize(m_pConfig->imageSize);

	// Image stitching mode
	m_pCheckBox_StitchingMode = new QCheckBox(this);
	m_pCheckBox_StitchingMode->setText("Enable Stitching Mode");
	m_pCheckBox_StitchingMode->setDisabled(true);

	m_pLabel_XStep = new QLabel("X Step", this);
	m_pLabel_XStep->setDisabled(true);
	m_pLineEdit_XStep = new QLineEdit(this);
	m_pLineEdit_XStep->setFixedWidth(25);
	m_pLineEdit_XStep->setText(QString::number(m_pConfig->imageStichingXStep));
	m_pLineEdit_XStep->setAlignment(Qt::AlignCenter);
	m_pLineEdit_XStep->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_pLineEdit_XStep->setDisabled(true);

	m_pLabel_YStep = new QLabel("  Y Step", this);
	m_pLabel_YStep->setDisabled(true);
	m_pLineEdit_YStep = new QLineEdit(this);
	m_pLineEdit_YStep->setFixedWidth(25);
	m_pLineEdit_YStep->setText(QString::number(m_pConfig->imageStichingYStep));
	m_pLineEdit_YStep->setAlignment(Qt::AlignCenter);
	m_pLineEdit_YStep->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_pLineEdit_YStep->setDisabled(true);

	m_pLabel_MisSyncPos = new QLabel("Mis-Sync Position", this);
	m_pLabel_MisSyncPos->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	m_pLabel_MisSyncPos->setDisabled(true);
	m_pLineEdit_MisSyncPos = new QLineEdit(this);
	m_pLineEdit_MisSyncPos->setFixedWidth(25);
	m_pLineEdit_MisSyncPos->setText(QString::number(m_pConfig->imageStichingMisSyncPos));
	m_pLineEdit_MisSyncPos->setAlignment(Qt::AlignCenter);
	m_pLineEdit_MisSyncPos->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	m_pLineEdit_MisSyncPos->setDisabled(true);
	

    QGridLayout *pGridLayout_ImageSize = new QGridLayout;
    pGridLayout_ImageSize->setSpacing(2);

    pGridLayout_ImageSize->addWidget(m_pLabel_ImageSize, 0, 0);		
    pGridLayout_ImageSize->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed), 0, 1);
    pGridLayout_ImageSize->addWidget(m_pRadioButton_128by128, 0, 2);
    pGridLayout_ImageSize->addWidget(m_pRadioButton_256by256, 0, 3);

    pGridLayout_ImageSize->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed), 1, 0, 1, 2);
    pGridLayout_ImageSize->addWidget(m_pRadioButton_512by512, 1, 2);
    pGridLayout_ImageSize->addWidget(m_pRadioButton_1024by1024, 1, 3);
	
	QHBoxLayout *pHBoxLayout_Averaging = new QHBoxLayout;
	pHBoxLayout_Averaging->setSpacing(2);

	pHBoxLayout_Averaging->addWidget(m_pLabel_Averaging);
	pHBoxLayout_Averaging->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
	pHBoxLayout_Averaging->addWidget(m_pLineEdit_Averaging);

	pGridLayout_ImageSize->addItem(pHBoxLayout_Averaging, 2, 0, 1, 4);

	pGridLayout_ImageSize->addWidget(m_pLabel_AcquisitionStatus, 3, 0, 1, 4);
	pGridLayout_ImageSize->addWidget(m_pLabel_AcquisitionStatusMsg, 4, 0, 1, 4);

	QGridLayout *pGridLayout_ImageStitching = new QGridLayout;
	pGridLayout_ImageStitching->setSpacing(2);

	pGridLayout_ImageStitching->addWidget(m_pCheckBox_StitchingMode, 0, 0);
	pGridLayout_ImageStitching->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed), 0, 1);
	pGridLayout_ImageStitching->addWidget(m_pLabel_XStep, 0, 2);
	pGridLayout_ImageStitching->addWidget(m_pLineEdit_XStep, 0, 3);
	pGridLayout_ImageStitching->addWidget(m_pLabel_YStep, 0, 4);
	pGridLayout_ImageStitching->addWidget(m_pLineEdit_YStep, 0, 5);

	pGridLayout_ImageStitching->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed), 1, 0, 1, 2);
	pGridLayout_ImageStitching->addWidget(m_pLabel_MisSyncPos, 1, 2, 1, 3);
	pGridLayout_ImageStitching->addWidget(m_pLineEdit_MisSyncPos, 1, 5);

	pGridLayout_ImageSize->addItem(pGridLayout_ImageStitching, 5, 0, 1, 4);


    // Create group boxes for streaming objects
    m_pGroupBox_OperationTab = new QGroupBox();
    m_pGroupBox_OperationTab->setLayout(m_pOperationTab->getLayout());
    m_pGroupBox_OperationTab->setStyleSheet("QGroupBox{padding-top:15px; margin-top:-15px}");
    m_pGroupBox_OperationTab->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

    m_pGroupBox_DeviceControlTab = new QGroupBox();
    m_pGroupBox_DeviceControlTab->setLayout(m_pDeviceControlTab->getLayout());
    m_pGroupBox_DeviceControlTab->setStyleSheet("QGroupBox{padding-top:15px; margin-top:-15px}");
    m_pGroupBox_DeviceControlTab->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_pGroupBox_DeviceControlTab->setFixedWidth(332);

    m_pGroupBox_VisualizationTab = new QGroupBox();
    m_pGroupBox_VisualizationTab->setLayout(m_pVisualizationTab->getLayout());
    m_pGroupBox_VisualizationTab->setStyleSheet("QGroupBox{padding-top:15px; margin-top:-15px}");
    m_pGroupBox_VisualizationTab->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_pGroupBox_ImageSizeTab = new QGroupBox();
    m_pGroupBox_ImageSizeTab->setLayout(pGridLayout_ImageSize);
    m_pGroupBox_ImageSizeTab->setStyleSheet("QGroupBox{padding-top:15px; margin-top:-15px}");
    m_pGroupBox_ImageSizeTab->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    m_pGroupBox_ImageSizeTab->setFixedWidth(332);
	
    // Create thread managers for data processing
    m_pThreadFlimProcess = new ThreadManager("FLIm image process");
    m_pThreadVisualization = new ThreadManager("Visualization process");

    // Create buffers for threading operation
    m_syncFlimProcessing.allocate_queue_buffer(m_pConfig->flimScans, m_pConfig->flimAlines, PROCESSING_BUFFER_SIZE); // FLIm Processing
    m_syncFlimVisualization.allocate_queue_buffer(11, m_pConfig->flimAlines, PROCESSING_BUFFER_SIZE); // FLIm Visualization
	
    // Set signal object
    setFlimAcquisitionCallback();
    setFlimProcessingCallback();
    setVisualizationCallback();

	
    // Create layout
    QGridLayout* pGridLayout = new QGridLayout;
    pGridLayout->setSpacing(2);
	
    // Set layout
    pGridLayout->addWidget(m_pGroupBox_VisualizationTab, 0, 0, 5, 1);
    pGridLayout->addWidget(m_pGroupBox_OperationTab, 0, 1);
    pGridLayout->addWidget(m_pVisualizationTab->getVisualizationWidgetsBox(), 1, 1);
    pGridLayout->addWidget(m_pGroupBox_ImageSizeTab, 2, 1);
    pGridLayout->addWidget(m_pGroupBox_DeviceControlTab, 3, 1);
    pGridLayout->addWidget(m_pListWidget_MsgWnd, 4, 1);

    this->setLayout(pGridLayout);


    // Connect signal and slot
    connect(m_pButtonGroup_ImageSize, SIGNAL(buttonClicked(int)), this, SLOT(changeImageSize(int)));
	connect(m_pLineEdit_Averaging, SIGNAL(textChanged(const QString &)), this, SLOT(changeAveragingFrame(const QString &)));
	connect(m_pCheckBox_StitchingMode, SIGNAL(toggled(bool)), this, SLOT(enableStitchingMode(bool)));
	connect(m_pLineEdit_XStep, SIGNAL(textChanged(const QString &)), this, SLOT(changeStitchingXStep(const QString &)));
	connect(m_pLineEdit_YStep, SIGNAL(textChanged(const QString &)), this, SLOT(changeStitchingYStep(const QString &)));
	connect(m_pLineEdit_MisSyncPos, SIGNAL(textChanged(const QString &)), this, SLOT(changeStitchingMisSyncPos(const QString &)));
}

QStreamTab::~QStreamTab()
{
    if (m_pThreadVisualization) delete m_pThreadVisualization;
    if (m_pThreadFlimProcess) delete m_pThreadFlimProcess;
}

void QStreamTab::keyPressEvent(QKeyEvent *e)
{
	if (e->key() != Qt::Key_Escape)
		QDialog::keyPressEvent(e);
}

void QStreamTab::setWidgetsText()
{
}

void QStreamTab::setImageSizeWidgets(bool enabled)
{
    m_pLabel_ImageSize->setEnabled(enabled);
	m_pRadioButton_128by128->setEnabled(enabled);
    m_pRadioButton_256by256->setEnabled(enabled);
    m_pRadioButton_512by512->setEnabled(enabled);
    m_pRadioButton_1024by1024->setEnabled(enabled);
}

void QStreamTab::setAveragingWidgets(bool enabled)
{
	m_pLabel_Averaging->setEnabled(enabled);
	m_pLineEdit_Averaging->setEnabled(enabled);
}

void QStreamTab::setFlimAcquisitionCallback()
{
	DataAcquisition* pDataAcq = m_pOperationTab->getDataAcq();
    pDataAcq->ConnectDaqAcquiredFlimData([&](int frame_count, const np::Array<uint16_t, 2>& frame) {
		
        // Data transfer for FLIm processing
        const uint16_t* frame_ptr = frame.raw_ptr();

        // Get buffer from threading queue
        uint16_t* pulse_ptr = nullptr;
        {
            std::unique_lock<std::mutex> lock(m_syncFlimProcessing.mtx);

            if (!m_syncFlimProcessing.queue_buffer.empty())
            {
                pulse_ptr = m_syncFlimProcessing.queue_buffer.front();
                m_syncFlimProcessing.queue_buffer.pop();
            }
        }

        if (pulse_ptr != nullptr)
        {
            // Body
            memcpy(pulse_ptr, frame_ptr, sizeof(uint16_t) * m_pConfig->flimFrameSize);

            // Push the buffer to sync Queue
            m_syncFlimProcessing.Queue_sync.push(pulse_ptr);
        }
		(void)frame_count;
    });
    pDataAcq->ConnectDaqStopFlimData([&]() {
        m_syncFlimProcessing.Queue_sync.push(nullptr);
    });

    pDataAcq->ConnectDaqSendStatusMessage([&](const char * msg, bool is_error) {
        if (is_error) m_pOperationTab->setAcquisitionButton(false);
        QString qmsg = QString::fromUtf8(msg);
        emit sendStatusMessage(qmsg, is_error);
    });
}

void QStreamTab::setFlimProcessingCallback()
{
    // FLIm Process Signal Objects /////////////////////////////////////////////////////////////////////////////////////////
    FLImProcess *pFLIm = m_pOperationTab->getDataAcq()->getFLIm();
    m_pThreadFlimProcess->DidAcquireData += [&, pFLIm] (int frame_count) {

        // Get the buffer from the previous sync Queue
        uint16_t* pulse_data = m_syncFlimProcessing.Queue_sync.pop();
        if (pulse_data != nullptr)
        {
            // Get buffers from threading queues
            float* flim_ptr = nullptr;
            {
                std::unique_lock<std::mutex> lock(m_syncFlimVisualization.mtx);

                if (!m_syncFlimVisualization.queue_buffer.empty())
                {
                    flim_ptr = m_syncFlimVisualization.queue_buffer.front();
                    m_syncFlimVisualization.queue_buffer.pop();
                }
            }

            if (flim_ptr != nullptr)
            {
                // Body
                np::FloatArray2 intensity (flim_ptr + 0 * m_pConfig->flimAlines, m_pConfig->flimAlines, 4);
                np::FloatArray2 mean_delay(flim_ptr + 4 * m_pConfig->flimAlines, m_pConfig->flimAlines, 4);
                np::FloatArray2 lifetime  (flim_ptr + 8 * m_pConfig->flimAlines, m_pConfig->flimAlines, 3);
                np::Uint16Array2 pulse(pulse_data, m_pConfig->flimScans, m_pConfig->flimAlines);

                (*pFLIm)(intensity, mean_delay, lifetime, pulse);

                // Transfer to FLIm calibration dlg
                if (!(frame_count % RENEWAL_COUNT))
                    if (m_pDeviceControlTab->getFlimCalibDlg())
                        emit m_pDeviceControlTab->getFlimCalibDlg()->plotRoiPulse(pFLIm, 0);

                // Push the buffers to sync Queues
                m_syncFlimVisualization.Queue_sync.push(flim_ptr);

                // Return (push) the buffer to the previous threading queue
                {
                    std::unique_lock<std::mutex> lock(m_syncFlimProcessing.mtx);
                    m_syncFlimProcessing.queue_buffer.push(pulse_data);
                }
            }
        }
        else
            m_pThreadFlimProcess->_running = false;

        (void)frame_count;
    };

    m_pThreadFlimProcess->DidStopData += [&]() {
        m_syncFlimVisualization.Queue_sync.push(nullptr);
    };

    m_pThreadFlimProcess->SendStatusMessage += [&](const char* msg, bool is_error) {
        if (is_error) m_pOperationTab->setAcquisitionButton(false);
        QString qmsg = QString::fromUtf8(msg);
        emit sendStatusMessage(qmsg, is_error);
    };
}

void QStreamTab::setVisualizationCallback()
{
    // Visualization Signal Objects ///////////////////////////////////////////////////////////////////////////////////////////
    m_pThreadVisualization->DidAcquireData += [&] (int frame_count) {

		MemoryBuffer *pMemBuff = m_pOperationTab->m_pMemoryBuffer;

		static int writtenSamples = 0;
		if (frame_count == 0) writtenSamples = 0;

		static int averageCount = 1;
		if (frame_count == 0) averageCount = 1;

		// Get the buffers from the previous sync Queues
		float* flim_data = m_syncFlimVisualization.Queue_sync.pop();
		if (flim_data != nullptr)
		{
			// Body
			if (m_pOperationTab->isAcquisitionButtonToggled()) // Only valid if acquisition is running 
			{
				// Averaging buffer
				if ((averageCount == 1) && (writtenSamples == 0))
				{
					m_pTempIntensity = np::FloatArray2(m_pConfig->imageSize, 3 * m_pConfig->imageSize);
					memset(m_pTempIntensity, 0, sizeof(float) * m_pTempIntensity.length());
					m_pTempLifetime = np::FloatArray2(m_pConfig->imageSize, 3 * m_pConfig->imageSize);
					memset(m_pTempLifetime, 0, sizeof(float) * m_pTempLifetime.length());
					m_pNonNaNIndex = np::FloatArray2(m_pConfig->imageSize, 3 * m_pConfig->imageSize);
					memset(m_pNonNaNIndex, 0, sizeof(float) * m_pNonNaNIndex.length());
				}

				// Data copy
				np::FloatArray2 intensity(flim_data + 0 * m_pConfig->flimAlines, m_pConfig->flimAlines, 4);
				np::FloatArray2 lifetime(flim_data + 8 * m_pConfig->flimAlines, m_pConfig->flimAlines, 3);

				for (int i = 0; i < 3; i++)
				{
					ippsAdd_32f_I(&intensity(0, i + 1), &m_pTempIntensity(0, i * m_pConfig->imageSize) + writtenSamples, m_pConfig->flimAlines);
					ippsAdd_32f_I(&lifetime(0, i), &m_pTempLifetime(0, i * m_pConfig->imageSize) + writtenSamples, m_pConfig->flimAlines);
					
					for (int j = 0; j < m_pConfig->flimAlines; j++)
						if (intensity(j, i + 1) != 0.0f)
							(*(&m_pNonNaNIndex(0, i * m_pConfig->imageSize) + writtenSamples + j))++;
				}
				writtenSamples += m_pConfig->flimAlines;
				
				// Averaging
				for (int i = 0; i < 3; i++)
				{
					ippsDiv_32f(&m_pNonNaNIndex(0, i * m_pConfig->imageSize), &m_pTempIntensity(0, i * m_pConfig->imageSize),
						m_pVisualizationTab->m_vecVisIntensity.at(i).raw_ptr(), writtenSamples);
					ippsDiv_32f(&m_pNonNaNIndex(0, i * m_pConfig->imageSize), &m_pTempLifetime(0, i * m_pConfig->imageSize),
						m_pVisualizationTab->m_vecVisLifetime.at(i).raw_ptr(), writtenSamples);

					//ippsDivC_32f(&m_pTempIntensity(0, i * m_pConfig->imageSize), averageCount, m_pVisualizationTab->m_vecVisIntensity.at(i).raw_ptr(), writtenSamples);
					//ippsDivC_32f(&m_pTempLifetime(0, i * m_pConfig->imageSize), averageCount, m_pVisualizationTab->m_vecVisLifetime.at(i).raw_ptr(), writtenSamples);

					//ippsDivC_32f(&m_pTempIntensity(0, i * m_pConfig->imageSize) + writtenSamples, averageCount - 1, 
					//	m_pVisualizationTab->m_vecVisIntensity.at(i).raw_ptr() + writtenSamples, m_pConfig->imageSize * m_pConfig->imageSize - writtenSamples);
					//ippsDivC_32f(&m_pTempLifetime(0, i * m_pConfig->imageSize) + writtenSamples, averageCount - 1, 
					//	m_pVisualizationTab->m_vecVisLifetime.at(i).raw_ptr() + writtenSamples, m_pConfig->imageSize * m_pConfig->imageSize - writtenSamples);
				}
				if (writtenSamples == m_pConfig->imageSize * m_pConfig->imageSize)
					averageCount++;
				
				// Draw Images
				//static int n = 0;
				//n++;

				//if (m_pConfig->imageSize > IMAGE_SIZE_512)
				//{
				//	if (!(n % 20))
				//		emit m_pVisualizationTab->drawImage();
				//}
				//else
				emit m_pVisualizationTab->drawImage();

				// Update Status
				QString str; str.sprintf("Written Samples : %7d / %7d,   Average : %3d / %3d",
					writtenSamples, m_pConfig->imageSize * m_pConfig->imageSize, averageCount, m_pConfig->imageAveragingFrames);
				m_pLabel_AcquisitionStatusMsg->setText(str);


				if (writtenSamples == m_pConfig->imageSize * m_pConfig->imageSize)
				{
					// Recording
					if (averageCount > m_pConfig->imageAveragingFrames)
					{
						// Buffering (When recording)
						if (pMemBuff->m_bIsRecording)
						{
							// Get buffer from writing queue
							float* image_ptr = pMemBuff->m_vectorWritingImageBuffer.at(pMemBuff->m_nRecordedFrame);

							if (image_ptr != nullptr)
							{
								// Body (Copying the frame data)
								for (int i = 0; i < 3; i++)
								{
									memcpy(image_ptr + i * m_pVisualizationTab->m_vecVisIntensity.at(i).length(), m_pVisualizationTab->m_vecVisIntensity.at(i).raw_ptr(),
										sizeof(float) * m_pVisualizationTab->m_vecVisIntensity.at(i).length());
									memcpy(image_ptr + (i + 3) * m_pVisualizationTab->m_vecVisLifetime.at(i).length(), m_pVisualizationTab->m_vecVisLifetime.at(i).raw_ptr(),
										sizeof(float) * m_pVisualizationTab->m_vecVisLifetime.at(i).length());
								}
								pMemBuff->increaseRecordedFrame();
							}

							// Finish recording when the buffer is full
							if (m_pCheckBox_StitchingMode->isChecked())
							{
								getDeviceControlTab()->getFlimLaserTrigControl()->setChecked(false);

								static int cum_x_step = 0;
								static int cum_y_step = 0;

								if (pMemBuff->m_nRecordedFrame == m_pConfig->imageStichingXStep * m_pConfig->imageStichingYStep)
								{
									pMemBuff->setIsRecorded(true);
									pMemBuff->setIsRecording(false);
									m_pOperationTab->setRecordingButton(false);
									getDeviceControlTab()->getFlimLaserTrigControl()->setChecked(true);

									getDeviceControlTab()->getZaberStage()->MoveRelative(1, -cum_x_step);
									getDeviceControlTab()->getZaberStage()->MoveRelative(2, -cum_y_step);
									cum_x_step = 0;
									cum_y_step = 0;
								}
								else
								{
									if (pMemBuff->m_nRecordedFrame % m_pConfig->imageStichingXStep != 0) // x move
									{
										int step = ((pMemBuff->m_nRecordedFrame / m_pConfig->imageStichingXStep + 1) % 2 ? 1 : -1) * m_pConfig->zaberPullbackLength;
										getDeviceControlTab()->getZaberStage()->MoveRelative(1, -step);
										cum_x_step += step;
									}
									else // y move
									{
										getDeviceControlTab()->getZaberStage()->MoveRelative(2, m_pConfig->zaberPullbackLength);
										cum_y_step += m_pConfig->zaberPullbackLength;
									}

									std::unique_lock<std::mutex> mlock(m_pDeviceControlTab->m_mtxStageScan);
									m_pDeviceControlTab->m_cvStageScan.wait(mlock);
								}
							}
							else
							{
								pMemBuff->setIsRecorded(true);
								pMemBuff->setIsRecording(true);
								m_pOperationTab->setRecordingButton(false);
							}
						}

						averageCount = 1;
					}

					// Re-initializing
					writtenSamples = 0;						
				}
			}

			// Return (push) the buffer to the previous threading queue
			{
				std::unique_lock<std::mutex> lock(m_syncFlimVisualization.mtx);
				m_syncFlimVisualization.queue_buffer.push(flim_data);
			}
		}
		else
			m_pThreadVisualization->_running = false;
    };

    m_pThreadVisualization->DidStopData += [&]() {
        // None
    };

    m_pThreadVisualization->SendStatusMessage += [&](const char* msg, bool is_error) {
        if (is_error) m_pOperationTab->setAcquisitionButton(false);
        QString qmsg = QString::fromUtf8(msg);
        emit sendStatusMessage(qmsg, is_error);
    };
}


void QStreamTab::changeImageSize(int size)
{
    m_pConfig->imageSize = size;
    m_pVisualizationTab->setObjects(size);

	m_pOperationTab->getSaveButton()->setEnabled(false);
	m_pOperationTab->getMemBuff()->m_bIsRecorded = false;

	m_pOperationTab->m_pMemoryBuffer->allocateWritingBuffer();

	QString str; str.sprintf("Written Samples : %7d / %7d,   Average : %3d / %3d", 0, m_pConfig->imageSize * m_pConfig->imageSize, 0, m_pConfig->imageAveragingFrames);
	m_pLabel_AcquisitionStatusMsg->setText(str);
}

void QStreamTab::changeAveragingFrame(const QString &str)
{
	m_pConfig->imageAveragingFrames = str.toInt();

	QString str1; str1.sprintf("Written Samples : %7d / %7d,   Average : %3d / %3d", 0, m_pConfig->imageSize * m_pConfig->imageSize, 0, m_pConfig->imageAveragingFrames);
	m_pLabel_AcquisitionStatusMsg->setText(str1);
}


void QStreamTab::enableStitchingMode(bool toggled)
{
	if (toggled)
	{
		// Set text
		m_pCheckBox_StitchingMode->setText("Disable Stitching Mode");
			
		// Set enabled true for image stitching widgets
		if (!m_pOperationTab->isAcquisitionButtonToggled())
		{
			m_pLabel_XStep->setEnabled(true);
			m_pLineEdit_XStep->setEnabled(true);
			m_pLabel_YStep->setEnabled(true);
			m_pLineEdit_YStep->setEnabled(true);
		}

		m_pLabel_MisSyncPos->setEnabled(true);
		m_pLineEdit_MisSyncPos->setEnabled(true);

		m_pVisualizationTab->getImageView()->setHorizontalLine(1, m_pConfig->imageStichingMisSyncPos);
		m_pVisualizationTab->visualizeImage();
	}
	else
	{
		// Set enabled false for image stitching widgets
		m_pLabel_XStep->setEnabled(false);
		m_pLineEdit_XStep->setEnabled(false);
		m_pLabel_YStep->setEnabled(false);
		m_pLineEdit_YStep->setEnabled(false);
		m_pLabel_MisSyncPos->setEnabled(false);
		m_pLineEdit_MisSyncPos->setEnabled(false);

		m_pVisualizationTab->getImageView()->setHorizontalLine(0);
		m_pVisualizationTab->visualizeImage();

		// Set text
		m_pCheckBox_StitchingMode->setText("Enable Stitching Mode");
	}
}

void QStreamTab::changeStitchingXStep(const QString &str)
{
	m_pConfig->imageStichingXStep = str.toInt();
	if (m_pConfig->imageStichingXStep < 1)
		m_pLineEdit_XStep->setText(QString::number(1));

	m_pOperationTab->m_pMemoryBuffer->allocateWritingBuffer();
}

void QStreamTab::changeStitchingYStep(const QString &str)
{
	m_pConfig->imageStichingYStep = str.toInt();
	if (m_pConfig->imageStichingYStep < 1)
		m_pLineEdit_YStep->setText(QString::number(1));

	m_pOperationTab->m_pMemoryBuffer->allocateWritingBuffer();
}

void QStreamTab::changeStitchingMisSyncPos(const QString &str)
{
	m_pConfig->imageStichingMisSyncPos = str.toInt();
	if (m_pConfig->imageStichingMisSyncPos >= m_pConfig->imageSize)
	{
		m_pConfig->imageStichingMisSyncPos = m_pConfig->imageSize - 1;
		m_pLineEdit_MisSyncPos->setText(QString("%1").arg(m_pConfig->imageStichingMisSyncPos));
	}
	if (m_pConfig->imageStichingMisSyncPos < 0)
	{
		m_pConfig->imageStichingMisSyncPos = 0;
		m_pLineEdit_MisSyncPos->setText(QString("%1").arg(m_pConfig->imageStichingMisSyncPos));
	}

	m_pVisualizationTab->getImageView()->setHorizontalLine(1, m_pConfig->imageStichingMisSyncPos);
	m_pVisualizationTab->visualizeImage();
}

void QStreamTab::processMessage(QString qmsg, bool is_error)
{
	m_pListWidget_MsgWnd->addItem(qmsg);
	m_pListWidget_MsgWnd->setCurrentRow(m_pListWidget_MsgWnd->count() - 1);

	if (is_error)
	{
		QMessageBox MsgBox(QMessageBox::Critical, "Error", qmsg);
		MsgBox.exec();
	}
}
