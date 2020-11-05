
#include "QDeviceControlTab.h"

#include <Doulos/MainWindow.h>
#include <Doulos/QStreamTab.h>
#include <Doulos/QOperationTab.h>
#include <Doulos/QVisualizationTab.h>

#include <Doulos/Dialog/FlimCalibDlg.h>
#include <Doulos/Viewer/QImageView.h>

#include <DeviceControl/FLImControl/PmtGainControl.h>
#include <DeviceControl/FLImControl/FlimTrigger.h>
#include <DeviceControl/ElforlightLaser/ElforlightLaser.h>
#include <DeviceControl/GalvoScan/GalvoScan.h>
#include <DeviceControl/ZaberStage/ZaberStage.h>

#include <MemoryBuffer/MemoryBuffer.h>

#include <Common/Array.h>

#include <iostream>
#include <thread>


QDeviceControlTab::QDeviceControlTab(QWidget *parent) :
    QDialog(parent), m_pPmtGainControl(nullptr), m_pFlimTrigControl(nullptr),
    m_pElforlightLaser(nullptr), m_pFlimCalibDlg(nullptr), m_pGalvoScan(nullptr), m_pZaberStage(nullptr)
{
	// Set main window objects
    m_pStreamTab = dynamic_cast<QStreamTab*>(parent);
    m_pConfig = m_pStreamTab->getMainWnd()->m_pConfiguration;

    // Create layout
    m_pVBoxLayout = new QVBoxLayout;
    m_pVBoxLayout->setSpacing(0);

	m_pVBoxLayout_FlimControl = new QVBoxLayout;
	m_pVBoxLayout_FlimControl->setSpacing(3);
    m_pGroupBox_FlimControl = new QGroupBox;

    createPmtGainControl();
    createFlimTriggeringControl();
	createFlimLaserPowerControl();
    createFLimCalibControl();
    createGalvoScanControl();
	createZaberStageControl();
	
    // Set layout
    setLayout(m_pVBoxLayout);
}

QDeviceControlTab::~QDeviceControlTab()
{
    if (m_pCheckBox_PmtGainControl->isChecked()) m_pCheckBox_PmtGainControl->setChecked(false);
    if (m_pCheckBox_FlimLaserTrigControl->isChecked()) m_pCheckBox_FlimLaserTrigControl->setChecked(false);
    if (m_pCheckBox_FlimLaserPowerControl->isChecked()) m_pCheckBox_FlimLaserPowerControl->setChecked(false);
	if (m_pCheckBox_GalvoScanControl->isChecked()) m_pCheckBox_GalvoScanControl->setChecked(false);
	if (m_pCheckBox_ZaberStageControl->isChecked()) m_pCheckBox_ZaberStageControl->setChecked(false);
}

void QDeviceControlTab::closeEvent(QCloseEvent* e)
{
    e->accept();
}

void QDeviceControlTab::setControlsStatus(bool enabled)
{
	if (!enabled)
	{
		if (m_pCheckBox_PmtGainControl->isChecked()) m_pCheckBox_PmtGainControl->setChecked(false);
		if (m_pCheckBox_FlimLaserTrigControl->isChecked()) m_pCheckBox_FlimLaserTrigControl->setChecked(false);
		if (m_pCheckBox_FlimLaserPowerControl->isChecked()) m_pCheckBox_FlimLaserPowerControl->setChecked(false);
		if (m_pCheckBox_GalvoScanControl->isChecked()) m_pCheckBox_GalvoScanControl->setChecked(false);
		if (m_pCheckBox_ZaberStageControl->isChecked()) m_pCheckBox_ZaberStageControl->setChecked(false);
	}
	else
	{
		if (!m_pCheckBox_PmtGainControl->isChecked()) m_pCheckBox_PmtGainControl->setChecked(true);
		if (!m_pCheckBox_FlimLaserTrigControl->isChecked()) m_pCheckBox_FlimLaserTrigControl->setChecked(true);
		if (!m_pCheckBox_FlimLaserPowerControl->isChecked()) m_pCheckBox_FlimLaserPowerControl->setChecked(true);
		if (!m_pCheckBox_GalvoScanControl->isChecked()) m_pCheckBox_GalvoScanControl->setChecked(true);
		if (!m_pCheckBox_ZaberStageControl->isChecked()) m_pCheckBox_ZaberStageControl->setChecked(true);
	}
}

void QDeviceControlTab::createPmtGainControl()
{
    // Create widgets for PMT gain control
    QHBoxLayout *pHBoxLayout_PmtGainControl = new QHBoxLayout;
    pHBoxLayout_PmtGainControl->setSpacing(3);

    m_pCheckBox_PmtGainControl = new QCheckBox(m_pGroupBox_FlimControl);
    m_pCheckBox_PmtGainControl->setText("Apply PMT Gain Voltage");
    m_pCheckBox_PmtGainControl->setFixedWidth(200);

    m_pLineEdit_PmtGainVoltage = new QLineEdit(m_pGroupBox_FlimControl);
    m_pLineEdit_PmtGainVoltage->setFixedWidth(35);
    m_pLineEdit_PmtGainVoltage->setText(QString::number(m_pConfig->pmtGainVoltage, 'f', 2));
    m_pLineEdit_PmtGainVoltage->setAlignment(Qt::AlignCenter);
    m_pLineEdit_PmtGainVoltage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_pLineEdit_PmtGainVoltage->setEnabled(true);

    m_pLabel_PmtGainVoltage = new QLabel("V", m_pGroupBox_FlimControl);
    m_pLabel_PmtGainVoltage->setBuddy(m_pLineEdit_PmtGainVoltage);
    m_pLabel_PmtGainVoltage->setEnabled(true);

    pHBoxLayout_PmtGainControl->addWidget(m_pCheckBox_PmtGainControl);
    pHBoxLayout_PmtGainControl->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
    pHBoxLayout_PmtGainControl->addWidget(m_pLineEdit_PmtGainVoltage);
    pHBoxLayout_PmtGainControl->addWidget(m_pLabel_PmtGainVoltage);

    m_pVBoxLayout_FlimControl->addItem(pHBoxLayout_PmtGainControl);

    // Connect signal and slot
    connect(m_pCheckBox_PmtGainControl, SIGNAL(toggled(bool)), this, SLOT(applyPmtGainVoltage(bool)));
    connect(m_pLineEdit_PmtGainVoltage, SIGNAL(textChanged(const QString &)), this, SLOT(changePmtGainVoltage(const QString &)));
}

void QDeviceControlTab::createFlimTriggeringControl()
{
    // Create widgets for FLIM laser control
    QHBoxLayout *pHBoxLayout_FlimLaserTrigControl = new QHBoxLayout;
    pHBoxLayout_FlimLaserTrigControl->setSpacing(3);

    m_pCheckBox_FlimLaserTrigControl = new QCheckBox(m_pGroupBox_FlimControl);
    m_pCheckBox_FlimLaserTrigControl->setText("Start FLIm Laser Triggering");

    m_pLineEdit_RepetitionRate = new QLineEdit(m_pGroupBox_FlimControl);
    m_pLineEdit_RepetitionRate->setFixedWidth(25);
    m_pLineEdit_RepetitionRate->setText(QString::number(m_pConfig->flimLaserRepRate));
    m_pLineEdit_RepetitionRate->setAlignment(Qt::AlignCenter);
    m_pLineEdit_RepetitionRate->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_pLabel_Hertz = new QLabel("kHz", m_pGroupBox_FlimControl);
    m_pLabel_Hertz->setBuddy(m_pLineEdit_RepetitionRate);

    pHBoxLayout_FlimLaserTrigControl->addWidget(m_pCheckBox_FlimLaserTrigControl);
    pHBoxLayout_FlimLaserTrigControl->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
    pHBoxLayout_FlimLaserTrigControl->addWidget(m_pLineEdit_RepetitionRate);
    pHBoxLayout_FlimLaserTrigControl->addWidget(m_pLabel_Hertz);

    m_pVBoxLayout_FlimControl->addItem(pHBoxLayout_FlimLaserTrigControl);

    // Connect signal and slot
    connect(m_pCheckBox_FlimLaserTrigControl, SIGNAL(toggled(bool)), this, SLOT(startFlimTriggering(bool)));
    connect(m_pLineEdit_RepetitionRate, SIGNAL(textChanged(const QString &)), this, SLOT(changeFlimLaserRepRate(const QString &)));
}

void QDeviceControlTab::createFlimLaserPowerControl()
{
    // Create widgets for FLIM laser power control
    QGridLayout *pGridLayout_FlimLaserPowerControl = new QGridLayout;
    pGridLayout_FlimLaserPowerControl->setSpacing(3);

    m_pCheckBox_FlimLaserPowerControl = new QCheckBox(m_pGroupBox_FlimControl);
    m_pCheckBox_FlimLaserPowerControl->setText("Connect to FLIm Laser for Power Control");

	m_pSpinBox_FlimLaserPowerControl = new QSpinBox(m_pGroupBox_FlimControl);
	m_pSpinBox_FlimLaserPowerControl->setValue(0);
	m_pSpinBox_FlimLaserPowerControl->setRange(-10000, 10000);
	m_pSpinBox_FlimLaserPowerControl->setFixedWidth(15);
	m_pSpinBox_FlimLaserPowerControl->setDisabled(true);
	static int flim_laser_power_level = 0;

    pGridLayout_FlimLaserPowerControl->addWidget(m_pCheckBox_FlimLaserPowerControl, 0, 0);
    pGridLayout_FlimLaserPowerControl->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed), 0, 1);
    pGridLayout_FlimLaserPowerControl->addWidget(m_pSpinBox_FlimLaserPowerControl, 0, 2);
	
	m_pVBoxLayout_FlimControl->addItem(pGridLayout_FlimLaserPowerControl);

	// Connect signal and slot
    connect(m_pCheckBox_FlimLaserPowerControl, SIGNAL(toggled(bool)), this, SLOT(connectFlimLaser(bool)));
	connect(m_pSpinBox_FlimLaserPowerControl, SIGNAL(valueChanged(int)), this, SLOT(adjustLaserPower(int)));
}

void QDeviceControlTab::createFLimCalibControl()
{
    // Create widgets for FLIm Calibration
    QGridLayout *pGridLayout_FlimCalibration = new QGridLayout;
    pGridLayout_FlimCalibration->setSpacing(3);

    m_pCheckBox_PX14DigitizerControl = new QCheckBox(m_pGroupBox_FlimControl);
    m_pCheckBox_PX14DigitizerControl->setText("Connect to PX14 Digitizer");
    m_pCheckBox_PX14DigitizerControl->setDisabled(true);

    m_pPushButton_FlimCalibration = new QPushButton(this);
    m_pPushButton_FlimCalibration->setText("FLIm Pulse View and Calibration...");
    m_pPushButton_FlimCalibration->setFixedWidth(200);
    m_pPushButton_FlimCalibration->setDisabled(true);

    pGridLayout_FlimCalibration->addWidget(m_pCheckBox_PX14DigitizerControl, 0, 0, 1, 3);
    pGridLayout_FlimCalibration->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed), 1, 0);
//    pGridLayout_FlimCalibration->addWidget(m_pLabel_PX14DcOffset, 1, 1);
//    pGridLayout_FlimCalibration->addWidget(m_pSlider_PX14DcOffset, 1, 2);
    pGridLayout_FlimCalibration->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed), 2, 0);
    pGridLayout_FlimCalibration->addWidget(m_pPushButton_FlimCalibration, 2, 1 , 1, 2);

    m_pVBoxLayout_FlimControl->addItem(pGridLayout_FlimCalibration);

    m_pGroupBox_FlimControl->setLayout(m_pVBoxLayout_FlimControl);
    m_pGroupBox_FlimControl->resize(m_pVBoxLayout_FlimControl->minimumSize());
    m_pVBoxLayout->addWidget(m_pGroupBox_FlimControl);

    // Connect signal and slot
    connect(m_pCheckBox_PX14DigitizerControl, SIGNAL(toggled(bool)), this, SLOT(connectPX14Digitizer(bool)));
    connect(m_pPushButton_FlimCalibration, SIGNAL(clicked(bool)), this, SLOT(createFlimCalibDlg()));
}

void QDeviceControlTab::createGalvoScanControl()
{
    // Create widgets for galvano mirror control
    QGroupBox *pGroupBox_GalvanoMirrorControl = new QGroupBox;
    QGridLayout *pGridLayout_GalvanoMirrorControl = new QGridLayout;
    pGridLayout_GalvanoMirrorControl->setSpacing(3);

    m_pCheckBox_GalvoScanControl = new QCheckBox(pGroupBox_GalvanoMirrorControl);
    m_pCheckBox_GalvoScanControl->setText("Connect to Galvano Mirror");

    m_pLineEdit_FastPeakToPeakVoltage = new QLineEdit(pGroupBox_GalvanoMirrorControl);
    m_pLineEdit_FastPeakToPeakVoltage->setFixedWidth(30);
    m_pLineEdit_FastPeakToPeakVoltage->setText(QString::number(m_pConfig->galvoFastScanVoltage, 'f', 1));
    m_pLineEdit_FastPeakToPeakVoltage->setAlignment(Qt::AlignCenter);
    m_pLineEdit_FastPeakToPeakVoltage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_pLineEdit_FastOffsetVoltage = new QLineEdit(pGroupBox_GalvanoMirrorControl);
    m_pLineEdit_FastOffsetVoltage->setFixedWidth(30);
    m_pLineEdit_FastOffsetVoltage->setText(QString::number(m_pConfig->galvoFastScanVoltageOffset, 'f', 1));
    m_pLineEdit_FastOffsetVoltage->setAlignment(Qt::AlignCenter);
    m_pLineEdit_FastOffsetVoltage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_pLabel_FastScanVoltage = new QLabel("Fast Scan Voltage ", pGroupBox_GalvanoMirrorControl);
    m_pLabel_FastScanPlusMinus = new QLabel("+", pGroupBox_GalvanoMirrorControl);
    m_pLabel_FastScanPlusMinus->setBuddy(m_pLineEdit_FastPeakToPeakVoltage);
    m_pLabel_FastGalvanoVoltage = new QLabel("V", pGroupBox_GalvanoMirrorControl);
    m_pLabel_FastGalvanoVoltage->setBuddy(m_pLineEdit_FastOffsetVoltage);

    m_pLineEdit_SlowPeakToPeakVoltage = new QLineEdit(pGroupBox_GalvanoMirrorControl);
    m_pLineEdit_SlowPeakToPeakVoltage->setFixedWidth(30);
    m_pLineEdit_SlowPeakToPeakVoltage->setText(QString::number(m_pConfig->galvoSlowScanVoltage, 'f', 1));
    m_pLineEdit_SlowPeakToPeakVoltage->setAlignment(Qt::AlignCenter);
    m_pLineEdit_SlowPeakToPeakVoltage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_pLineEdit_SlowOffsetVoltage = new QLineEdit(pGroupBox_GalvanoMirrorControl);
    m_pLineEdit_SlowOffsetVoltage->setFixedWidth(30);
    m_pLineEdit_SlowOffsetVoltage->setText(QString::number(m_pConfig->galvoSlowScanVoltageOffset, 'f', 1));
    m_pLineEdit_SlowOffsetVoltage->setAlignment(Qt::AlignCenter);
    m_pLineEdit_SlowOffsetVoltage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_pLabel_SlowScanVoltage = new QLabel("Slow Scan Voltage ", pGroupBox_GalvanoMirrorControl);
    m_pLabel_SlowScanPlusMinus = new QLabel("+", pGroupBox_GalvanoMirrorControl);
    m_pLabel_SlowScanPlusMinus->setBuddy(m_pLineEdit_SlowPeakToPeakVoltage);
    m_pLabel_SlowGalvanoVoltage = new QLabel("V", pGroupBox_GalvanoMirrorControl);
    m_pLabel_SlowGalvanoVoltage->setBuddy(m_pLineEdit_SlowOffsetVoltage);

	m_pLabel_FlyingBack = new QLabel("Flying Back Position  ", pGroupBox_GalvanoMirrorControl);
	m_pLineEdit_FlyingBack = new QLineEdit(pGroupBox_GalvanoMirrorControl);
	m_pLineEdit_FlyingBack->setFixedWidth(30);
	m_pLineEdit_FlyingBack->setText(QString::number(m_pConfig->galvoFlyingBack));
	m_pLineEdit_FlyingBack->setAlignment(Qt::AlignCenter);
	m_pLineEdit_FlyingBack->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	QHBoxLayout *pHBoxLayout_FlyingBack = new QHBoxLayout;
	pHBoxLayout_FlyingBack->setSpacing(3);
	pHBoxLayout_FlyingBack->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	pHBoxLayout_FlyingBack->addWidget(m_pLabel_FlyingBack);
	pHBoxLayout_FlyingBack->addWidget(m_pLineEdit_FlyingBack);
	

    pGridLayout_GalvanoMirrorControl->addWidget(m_pCheckBox_GalvoScanControl, 0, 0, 1, 6);

    pGridLayout_GalvanoMirrorControl->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed), 2, 0);
    pGridLayout_GalvanoMirrorControl->addWidget(m_pLabel_FastScanVoltage, 2, 1);
    pGridLayout_GalvanoMirrorControl->addWidget(m_pLineEdit_FastPeakToPeakVoltage, 2, 2);
    pGridLayout_GalvanoMirrorControl->addWidget(m_pLabel_FastScanPlusMinus, 2, 3);
    pGridLayout_GalvanoMirrorControl->addWidget(m_pLineEdit_FastOffsetVoltage, 2, 4);
    pGridLayout_GalvanoMirrorControl->addWidget(m_pLabel_FastGalvanoVoltage, 2, 5);
    pGridLayout_GalvanoMirrorControl->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed), 3, 0);
    pGridLayout_GalvanoMirrorControl->addWidget(m_pLabel_SlowScanVoltage, 3, 1);
    pGridLayout_GalvanoMirrorControl->addWidget(m_pLineEdit_SlowPeakToPeakVoltage, 3, 2);
    pGridLayout_GalvanoMirrorControl->addWidget(m_pLabel_SlowScanPlusMinus, 3, 3);
    pGridLayout_GalvanoMirrorControl->addWidget(m_pLineEdit_SlowOffsetVoltage, 3, 4);
    pGridLayout_GalvanoMirrorControl->addWidget(m_pLabel_SlowGalvanoVoltage, 3, 5);
	pGridLayout_GalvanoMirrorControl->addItem(pHBoxLayout_FlyingBack, 4, 0, 1, 6);


    pGroupBox_GalvanoMirrorControl->setLayout(pGridLayout_GalvanoMirrorControl);
    m_pVBoxLayout->addWidget(pGroupBox_GalvanoMirrorControl);

    // Connect signal and slot
    connect(m_pCheckBox_GalvoScanControl, SIGNAL(toggled(bool)), this, SLOT(connectGalvanoMirror(bool)));
    connect(m_pLineEdit_FastPeakToPeakVoltage, SIGNAL(textChanged(const QString &)), this, SLOT(changeGalvoFastScanVoltage(const QString &)));
    connect(m_pLineEdit_FastOffsetVoltage, SIGNAL(textChanged(const QString &)), this, SLOT(changeGalvoFastScanVoltageOffset(const QString &)));
    connect(m_pLineEdit_SlowPeakToPeakVoltage, SIGNAL(textChanged(const QString &)), this, SLOT(changeGalvoSlowScanVoltage(const QString &)));
    connect(m_pLineEdit_SlowOffsetVoltage, SIGNAL(textChanged(const QString &)), this, SLOT(changeGalvoSlowScanVoltageOffset(const QString &)));
	connect(m_pLineEdit_FlyingBack, SIGNAL(textChanged(const QString &)), this, SLOT(changeFlyingBack(const QString &)));
}

void QDeviceControlTab::createZaberStageControl()
{
	// Create widgets for Zaber stage control
    QGroupBox *pGroupBox_ZaberStageControl = new QGroupBox;
    QGridLayout *pGridLayout_ZaberStageControl = new QGridLayout;
    pGridLayout_ZaberStageControl->setSpacing(3);

    m_pCheckBox_ZaberStageControl = new QCheckBox(pGroupBox_ZaberStageControl);
    m_pCheckBox_ZaberStageControl->setText("Enable Zaber Stage Control");

	m_pPushButton_SetStageNumber = new QPushButton(pGroupBox_ZaberStageControl);
	m_pPushButton_SetStageNumber->setText("Stage Number");
	m_pPushButton_SetStageNumber->setDisabled(true);
    m_pPushButton_SetTargetSpeed = new QPushButton(pGroupBox_ZaberStageControl);
    m_pPushButton_SetTargetSpeed->setText("Target Speed");
	m_pPushButton_SetTargetSpeed->setDisabled(true);
    m_pPushButton_MoveRelative = new QPushButton(pGroupBox_ZaberStageControl);
	m_pPushButton_MoveRelative->setText("Move Relative");
	m_pPushButton_MoveRelative->setDisabled(true);
    m_pPushButton_Home = new QPushButton(pGroupBox_ZaberStageControl);
    m_pPushButton_Home->setText("Home");
	m_pPushButton_Home->setFixedWidth(60);
	m_pPushButton_Home->setDisabled(true);
    m_pPushButton_Stop = new QPushButton(pGroupBox_ZaberStageControl);
    m_pPushButton_Stop->setText("Stop");
	m_pPushButton_Stop->setFixedWidth(60);
	m_pPushButton_Stop->setDisabled(true);

	m_pComboBox_StageNumber = new QComboBox(pGroupBox_ZaberStageControl);
	m_pComboBox_StageNumber->addItem("X-axis");
	m_pComboBox_StageNumber->addItem("Y-axis");
	m_pComboBox_StageNumber->setCurrentIndex(0);
	m_pComboBox_StageNumber->setDisabled(true);
    m_pLineEdit_TargetSpeed = new QLineEdit(pGroupBox_ZaberStageControl);
    m_pLineEdit_TargetSpeed->setFixedWidth(25);
    m_pLineEdit_TargetSpeed->setText(QString::number(m_pConfig->zaberPullbackSpeed));
	m_pLineEdit_TargetSpeed->setAlignment(Qt::AlignCenter);
	m_pLineEdit_TargetSpeed->setDisabled(true);
    m_pLineEdit_TravelLength = new QLineEdit(pGroupBox_ZaberStageControl);
    m_pLineEdit_TravelLength->setFixedWidth(25);
    m_pLineEdit_TravelLength->setText(QString::number(m_pConfig->zaberPullbackLength));
	m_pLineEdit_TravelLength->setAlignment(Qt::AlignCenter);
	m_pLineEdit_TravelLength->setDisabled(true);

    m_pLabel_TargetSpeed = new QLabel("mm/s", pGroupBox_ZaberStageControl);
    m_pLabel_TargetSpeed->setBuddy(m_pLineEdit_TargetSpeed);
	m_pLabel_TargetSpeed->setDisabled(true);
    m_pLabel_TravelLength = new QLabel("mm", pGroupBox_ZaberStageControl);
    m_pLabel_TravelLength->setBuddy(m_pLineEdit_TravelLength);
	m_pLabel_TravelLength->setDisabled(true);
	
	m_pLabel_ScanningStatus = new QLabel("scanning", pGroupBox_ZaberStageControl);
	m_pLabel_ScanningStatus->setStyleSheet("font: 12pt; color: red;");
	m_pLabel_ScanningStatus->setAlignment(Qt::AlignCenter);
	m_pLabel_ScanningStatus->setDisabled(true);


    pGridLayout_ZaberStageControl->addWidget(m_pCheckBox_ZaberStageControl, 0, 0, 1, 5);
	
	pGridLayout_ZaberStageControl->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed), 1, 0);
	pGridLayout_ZaberStageControl->addWidget(m_pPushButton_SetStageNumber, 1, 1, 1, 2);
	pGridLayout_ZaberStageControl->addWidget(m_pComboBox_StageNumber, 1, 3, 1, 2);

    //pGridLayout_ZaberStageControl->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed), 2, 0);
    pGridLayout_ZaberStageControl->addWidget(m_pPushButton_SetTargetSpeed, 2, 1, 1, 2);
    pGridLayout_ZaberStageControl->addWidget(m_pLineEdit_TargetSpeed, 2, 3);
    pGridLayout_ZaberStageControl->addWidget(m_pLabel_TargetSpeed, 2, 4);

    //pGridLayout_ZaberStageControl->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed), 3, 0);
    pGridLayout_ZaberStageControl->addWidget(m_pPushButton_MoveRelative, 3, 1, 1, 2);
    pGridLayout_ZaberStageControl->addWidget(m_pLineEdit_TravelLength, 3, 3);
    pGridLayout_ZaberStageControl->addWidget(m_pLabel_TravelLength, 3, 4);

    //pGridLayout_ZaberStageControl->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed), 4, 0);
    pGridLayout_ZaberStageControl->addWidget(m_pPushButton_Home, 4, 1);
    pGridLayout_ZaberStageControl->addWidget(m_pPushButton_Stop, 4, 2);
	pGridLayout_ZaberStageControl->addWidget(m_pLabel_ScanningStatus, 2, 0, 3, 1);
	

	pGroupBox_ZaberStageControl->setLayout(pGridLayout_ZaberStageControl);
	m_pVBoxLayout->addWidget(pGroupBox_ZaberStageControl);

	// Connect signal and slot
	connect(m_pCheckBox_ZaberStageControl, SIGNAL(toggled(bool)), this, SLOT(connectZaberStage(bool)));
	connect(m_pPushButton_MoveRelative, SIGNAL(clicked(bool)), this, SLOT(moveRelative()));
	connect(m_pLineEdit_TargetSpeed, SIGNAL(textChanged(const QString &)), this, SLOT(setTargetSpeed(const QString &)));
	connect(m_pLineEdit_TravelLength, SIGNAL(textChanged(const QString &)), this, SLOT(changeZaberPullbackLength(const QString &)));
	connect(m_pPushButton_Home, SIGNAL(clicked(bool)), this, SLOT(home()));
	connect(m_pPushButton_Stop, SIGNAL(clicked(bool)), this, SLOT(stop()));
}


void QDeviceControlTab::applyPmtGainVoltage(bool toggled)
{
    if (toggled)
    {
        // Set text
        m_pCheckBox_PmtGainControl->setText("Stop Applying PMT Gain Control");

        // Set enabled false for PMT gain control widgets
        m_pLineEdit_PmtGainVoltage->setEnabled(false);
        m_pLabel_PmtGainVoltage->setEnabled(false);

        // Create PMT gain control objects
        if (!m_pPmtGainControl)
        {
            m_pPmtGainControl = new PmtGainControl;
            m_pPmtGainControl->SendStatusMessage += [&](const char* msg, bool is_error) {
                QString qmsg = QString::fromUtf8(msg);
                emit m_pStreamTab->sendStatusMessage(qmsg, is_error);
            };
        }

        m_pPmtGainControl->voltage = m_pLineEdit_PmtGainVoltage->text().toDouble();
        if (m_pPmtGainControl->voltage > 1.0)
        {
            m_pPmtGainControl->SendStatusMessage(">1.0V Gain cannot be assigned!", true);
            m_pCheckBox_PmtGainControl->setChecked(false);
            return;
        }

        // Initializing
        if (!m_pPmtGainControl->initialize())
        {
            m_pCheckBox_PmtGainControl->setChecked(false);
            return;
        }

        // Generate PMT gain voltage
        m_pPmtGainControl->start();
    }
    else
    {
        // Delete PMT gain control objects
        if (m_pPmtGainControl)
        {
            m_pPmtGainControl->stop();
            delete m_pPmtGainControl;
            m_pPmtGainControl = nullptr;
        }

        // Set enabled true for PMT gain control widgets
        m_pLineEdit_PmtGainVoltage->setEnabled(true);
        m_pLabel_PmtGainVoltage->setEnabled(true);

        // Set text
        m_pCheckBox_PmtGainControl->setText("Apply PMT Gain Control");
    }
}

void QDeviceControlTab::changePmtGainVoltage(const QString &vol)
{
    m_pConfig->pmtGainVoltage = vol.toFloat();
}

void QDeviceControlTab::startFlimTriggering(bool toggled)
{
    if (toggled)
    {
        // Set text
        m_pCheckBox_FlimLaserTrigControl->setText("Stop FLIm Laser Triggering");

        // Set enabled false for FLIm laser sync control widgets
        m_pLineEdit_RepetitionRate->setEnabled(false);
        m_pLabel_Hertz->setEnabled(false);

        // Create FLIm laser sync control objects
        if (!m_pFlimTrigControl)
        {
            m_pFlimTrigControl = new FlimTrigger;
            m_pFlimTrigControl->SendStatusMessage += [&](const char* msg, bool is_error) {
                QString qmsg = QString::fromUtf8(msg);
                emit m_pStreamTab->sendStatusMessage(qmsg, is_error);
            };
        }

        if ((m_pConfig->flimLaserRepRate > 50) || (m_pConfig->flimLaserRepRate < 5))
        {
            m_pFlimTrigControl->SendStatusMessage("[FLIm Triggering] Invalid repetition rate.", true);
            m_pCheckBox_FlimLaserTrigControl->setChecked(false);
            return;
        }

        //m_pFlimTrigControl->sourceTerminal = "20MHzTimeBase";
        m_pFlimTrigControl->slow = 20000 / m_pConfig->flimLaserRepRate;

        // Initializing
        if (!m_pFlimTrigControl->initialize())
        {
            m_pCheckBox_FlimLaserTrigControl->setChecked(false);
            return;
        }

        // Generate FLIm laser sync
        m_pFlimTrigControl->start();
    }
    else
    {
        // Delete FLIm laser sync control objects
        if (m_pFlimTrigControl)
        {
            m_pFlimTrigControl->stop();
            delete m_pFlimTrigControl;
            m_pFlimTrigControl = nullptr;
        }

        // Set enabled true for FLIm laser sync control widgets
        m_pLineEdit_RepetitionRate->setEnabled(true);
        m_pLabel_Hertz->setEnabled(true);


        // Set text
        m_pCheckBox_FlimLaserTrigControl->setText("Start FLIm Laser Triggering");
    }
}

void QDeviceControlTab::changeFlimLaserRepRate(const QString &rep_rate)
{
    m_pConfig->flimLaserRepRate = rep_rate.toInt();
}

void QDeviceControlTab::connectFlimLaser(bool toggled)
{
	if (toggled)
	{
		// Set text
        m_pCheckBox_FlimLaserPowerControl->setText("Disconnect from FLIm Laser");

		// Create FLIM laser power control objects
		if (!m_pElforlightLaser)
		{
			m_pElforlightLaser = new ElforlightLaser;
			m_pElforlightLaser->SendStatusMessage += [&](const char* msg, bool is_error) {
				QString qmsg = QString::fromUtf8(msg); 
				qmsg.replace('\n', ' ');
				emit m_pStreamTab->sendStatusMessage(qmsg, is_error);
			};
		}

		// Connect the laser
		if (!(m_pElforlightLaser->ConnectDevice()))
		{
			m_pCheckBox_FlimLaserPowerControl->setChecked(false);
			return;
		}

		// Set enabled true for FLIM laser power control widgets
		m_pSpinBox_FlimLaserPowerControl->setEnabled(true);
	}
	else
	{
		// Set enabled false for FLIM laser power control widgets
		m_pSpinBox_FlimLaserPowerControl->setEnabled(false);

		if (m_pElforlightLaser)
		{
			// Disconnect the laser
			m_pElforlightLaser->DisconnectDevice();

			// Delete FLIM laser power control objects
			delete m_pElforlightLaser;
            m_pElforlightLaser = nullptr;
		}
		
		// Set text
        m_pCheckBox_FlimLaserPowerControl->setText("Connect to FLIm Laser for Power Control");
	}
}

void QDeviceControlTab::adjustLaserPower(int level)
{
	static int flim_laser_power_level = 0;

	if (level > flim_laser_power_level)
	{
		m_pElforlightLaser->IncreasePower();
		flim_laser_power_level++;
	}
	else
	{
		m_pElforlightLaser->DecreasePower();
		flim_laser_power_level--;
	}
}


void QDeviceControlTab::connectPX14Digitizer(bool toggled)
{
    if (toggled)
    {
        // Set text
        m_pCheckBox_PX14DigitizerControl->setText("Disconnect from PX14 Digitizer");

        // Connect the digitizer

        // Set enabled true for PX14 digitizer widgets
        m_pPushButton_FlimCalibration->setEnabled(true);
    }
    else
    {
        // Set disabled true for PX14 digitizer widgets
        m_pPushButton_FlimCalibration->setDisabled(true);

        // Close FLIm calibration window
        if (m_pFlimCalibDlg) m_pFlimCalibDlg->close();

        // Set text
        m_pCheckBox_PX14DigitizerControl->setText("Connect to PX14 Digitizer");
    }
}

void QDeviceControlTab::createFlimCalibDlg()
{
    if (m_pFlimCalibDlg == nullptr)
    {
        m_pFlimCalibDlg = new FlimCalibDlg(this);
        connect(m_pFlimCalibDlg, SIGNAL(finished(int)), this, SLOT(deleteFlimCalibDlg()));
        m_pFlimCalibDlg->show();
//        emit m_pFlimCalibDlg->plotRoiPulse(m_pFLIm, m_pSlider_SelectAline->value() / 4);
    }
    m_pFlimCalibDlg->raise();
    m_pFlimCalibDlg->activateWindow();
}

void QDeviceControlTab::deleteFlimCalibDlg()
{
//    m_pFlimCalibDlg->showWindow(false);
//    m_pFlimCalibDlg->showMeanDelay(false);
//    m_pFlimCalibDlg->showMask(false);

    m_pFlimCalibDlg->deleteLater();
    m_pFlimCalibDlg = nullptr;
}


void QDeviceControlTab::connectGalvanoMirror(bool toggled)
{
    if (toggled)
    {
        // Set text
        m_pCheckBox_GalvoScanControl->setText("Disconnect from Galvano Mirror");

        // Set enabled false for Galvano mirror control widgets
        m_pLabel_FastScanVoltage->setEnabled(false);
        m_pLineEdit_FastPeakToPeakVoltage->setEnabled(false);
        m_pLabel_FastScanPlusMinus->setEnabled(false);
        m_pLineEdit_FastOffsetVoltage->setEnabled(false);
        m_pLabel_FastGalvanoVoltage->setEnabled(false);

        m_pLabel_SlowScanVoltage->setEnabled(false);
        m_pLineEdit_SlowPeakToPeakVoltage->setEnabled(false);
        m_pLabel_SlowScanPlusMinus->setEnabled(false);
        m_pLineEdit_SlowOffsetVoltage->setEnabled(false);
        m_pLabel_SlowGalvanoVoltage->setEnabled(false);

        m_pStreamTab->setImageSizeWidgets(false);

        // Create Galvano mirror control objects
        if (!m_pGalvoScan)
        {
            m_pGalvoScan = new GalvoScan;
            m_pGalvoScan->SendStatusMessage += [&](const char* msg, bool is_error) {
                QString qmsg = QString::fromUtf8(msg);
                emit m_pStreamTab->sendStatusMessage(qmsg, is_error);
            };
        }

        m_pGalvoScan->horizontal_size = m_pConfig->imageSize;
        m_pGalvoScan->pp_voltage_fast = m_pLineEdit_FastPeakToPeakVoltage->text().toDouble();
        m_pGalvoScan->offset_fast = m_pLineEdit_FastOffsetVoltage->text().toDouble();
        m_pGalvoScan->pp_voltage_slow = m_pLineEdit_SlowPeakToPeakVoltage->text().toDouble();
        m_pGalvoScan->offset_slow = m_pLineEdit_SlowOffsetVoltage->text().toDouble();
        m_pGalvoScan->step_slow = m_pGalvoScan->pp_voltage_slow / (double)m_pConfig->imageSize;

        // Initializing
		if (!m_pGalvoScan->initialize())
		{
			m_pCheckBox_GalvoScanControl->setChecked(false);
			return;
		}
		else
			m_pGalvoScan->start();
    }
    else
    {
        // Delete Galvano mirror control objects
        if (m_pGalvoScan)
        {
            m_pGalvoScan->stop();
            delete m_pGalvoScan;
            m_pGalvoScan = nullptr;
        }

        // Set enabled false for Galvano mirror control widgets
        m_pLabel_FastScanVoltage->setEnabled(true);
        m_pLineEdit_FastPeakToPeakVoltage->setEnabled(true);
        m_pLabel_FastScanPlusMinus->setEnabled(true);
        m_pLineEdit_FastOffsetVoltage->setEnabled(true);
        m_pLabel_FastGalvanoVoltage->setEnabled(true);

        m_pLabel_SlowScanVoltage->setEnabled(true);
        m_pLineEdit_SlowPeakToPeakVoltage->setEnabled(true);
        m_pLabel_SlowScanPlusMinus->setEnabled(true);
        m_pLineEdit_SlowOffsetVoltage->setEnabled(true);
        m_pLabel_SlowGalvanoVoltage->setEnabled(true);

		if (!m_pStreamTab->getOperationTab()->isAcquisitionButtonToggled())
			m_pStreamTab->setImageSizeWidgets(true);

        // Set text
        m_pCheckBox_GalvoScanControl->setText("Connect to Galvano Mirror");
    }
}

void QDeviceControlTab::changeGalvoFastScanVoltage(const QString &vol)
{
    m_pConfig->galvoFastScanVoltage = vol.toFloat();
}

void QDeviceControlTab::changeGalvoFastScanVoltageOffset(const QString &vol)
{
    m_pConfig->galvoFastScanVoltageOffset = vol.toFloat();
}

void QDeviceControlTab::changeGalvoSlowScanVoltage(const QString &vol)
{
    m_pConfig->galvoSlowScanVoltage = vol.toFloat();
}

void QDeviceControlTab::changeGalvoSlowScanVoltageOffset(const QString &vol)
{
    m_pConfig->galvoSlowScanVoltageOffset = vol.toFloat();
}

void QDeviceControlTab::changeFlyingBack(const QString &str)
{
	m_pConfig->galvoFlyingBack = str.toInt();
	if (m_pConfig->galvoFlyingBack >= m_pConfig->imageSize)
	{
		m_pConfig->galvoFlyingBack = m_pConfig->imageSize - 1;
		m_pLineEdit_FlyingBack->setText(QString("%1").arg(m_pConfig->galvoFlyingBack));
	}
	if (m_pConfig->galvoFlyingBack < 0)
	{
		m_pConfig->galvoFlyingBack = 0;
		m_pLineEdit_FlyingBack->setText(QString("%1").arg(m_pConfig->galvoFlyingBack));
	}

	m_pStreamTab->getVisualizationTab()->getImageView()->setVerticalLine(1, m_pConfig->galvoFlyingBack);
	m_pStreamTab->getVisualizationTab()->visualizeImage();
}


void QDeviceControlTab::connectZaberStage(bool toggled)
{
	if (toggled)
	{
		// Set text
		m_pCheckBox_ZaberStageControl->setText("Disconnect from Zaber Stage");

		// Create Zaber stage control objects
		if (!m_pZaberStage)
		{
			m_pZaberStage = new ZaberStage;
			m_pZaberStage->SendStatusMessage += [&](const char* msg, bool is_error) {
				QString qmsg = QString::fromUtf8(msg);
				qmsg.replace('\n', ' ');	
				emit m_pStreamTab->sendStatusMessage(qmsg, is_error);
			};
		}

		// Connect stage
		if (!(m_pZaberStage->ConnectStage()))
		{
			m_pCheckBox_ZaberStageControl->setChecked(false);
			return;
		}

		// Set callback function
		m_pZaberStage->DidMovedRelative += [&]() {
			if (m_pStreamTab->getOperationTab()->m_pMemoryBuffer->m_bIsRecording)
			{
				if (m_pStreamTab->getImageStitchingCheckBox()->isChecked())
				{					
					m_pCheckBox_FlimLaserTrigControl->setChecked(true);

					std::unique_lock<std::mutex> mlock(m_mtxStageScan);
					mlock.unlock();
					m_cvStageScan.notify_one();
				}
			}
		};

		// Set target speed first
		m_pZaberStage->SetTargetSpeed(1, m_pLineEdit_TargetSpeed->text().toDouble());
		m_pZaberStage->SetTargetSpeed(2, m_pLineEdit_TargetSpeed->text().toDouble());

		// Set enable true for Zaber stage control widgets
		m_pPushButton_SetStageNumber->setEnabled(true);
		m_pPushButton_MoveRelative->setEnabled(true);
		m_pPushButton_SetTargetSpeed->setEnabled(true);
		m_pPushButton_Home->setEnabled(true);
		m_pPushButton_Stop->setEnabled(true);
		m_pComboBox_StageNumber->setEnabled(true);
		m_pLineEdit_TravelLength->setEnabled(true);
		m_pLineEdit_TargetSpeed->setEnabled(true);
		m_pLabel_TravelLength->setEnabled(true);
		m_pLabel_TargetSpeed->setEnabled(true);

		m_pStreamTab->getImageStitchingCheckBox()->setEnabled(true);
	}
	else
	{
		// Set enable false for Zaber stage control widgets
		if (m_pStreamTab->getImageStitchingCheckBox()->isChecked())
			m_pStreamTab->getImageStitchingCheckBox()->setChecked(false);
		m_pStreamTab->getImageStitchingCheckBox()->setDisabled(true);

		m_pPushButton_SetStageNumber->setEnabled(false);
		m_pPushButton_MoveRelative->setEnabled(false);
		m_pPushButton_SetTargetSpeed->setEnabled(false);
		m_pPushButton_Home->setEnabled(false);
		m_pPushButton_Stop->setEnabled(false);
		m_pComboBox_StageNumber->setEnabled(false);
		m_pLineEdit_TravelLength->setEnabled(false);
		m_pLineEdit_TargetSpeed->setEnabled(false);
		m_pLabel_TravelLength->setEnabled(false);
		m_pLabel_TargetSpeed->setEnabled(false);

		if (m_pZaberStage)
		{
			// Stop Wait Thread
			m_pZaberStage->StopWaitThread();

			// Disconnect the Stage
			m_pZaberStage->DisconnectStage();

			// Delete Zaber stage control objects
			delete m_pZaberStage;
			m_pZaberStage = nullptr;
		}

		// Set text
		m_pCheckBox_ZaberStageControl->setText("Connect to Zaber Stage");
	}
}

void QDeviceControlTab::moveRelative()
{
	m_pZaberStage->MoveRelative(m_pComboBox_StageNumber->currentIndex() + 1, m_pLineEdit_TravelLength->text().toDouble());
}

void QDeviceControlTab::setTargetSpeed(const QString & str)
{
	m_pZaberStage->SetTargetSpeed(m_pComboBox_StageNumber->currentIndex() + 1, str.toDouble());
	m_pConfig->zaberPullbackSpeed = str.toInt();
}

void QDeviceControlTab::changeZaberPullbackLength(const QString &str)
{
	m_pConfig->zaberPullbackLength = str.toFloat();
}

void QDeviceControlTab::home()
{
	m_pZaberStage->Home(m_pComboBox_StageNumber->currentIndex() + 1);
}

void QDeviceControlTab::stop()
{
	m_pZaberStage->Stop();
}