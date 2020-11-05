#ifndef QSTREAMTAB_H
#define QSTREAMTAB_H

#include <QDialog>
#include <QtWidgets>
#include <QtCore>

#include <Doulos/Configuration.h>

#include <Common/array.h>
#include <Common/SyncObject.h>

class MainWindow;
class QOperationTab;
class QDeviceControlTab;
class QVisualizationTab;

class ThreadManager;
class FLImProcess;

#define IMAGE_SIZE_128  128
#define IMAGE_SIZE_256  256
#define IMAGE_SIZE_512  512
#define IMAGE_SIZE_1024 1024


class QStreamTab : public QDialog
{
    Q_OBJECT

// Constructer & Destructer /////////////////////////////
public:
    explicit QStreamTab(QWidget *parent = nullptr);
	virtual ~QStreamTab();
	
// Methods //////////////////////////////////////////////
protected:
	void keyPressEvent(QKeyEvent *);

public:
	inline MainWindow* getMainWnd() const { return m_pMainWnd; }
    inline QOperationTab* getOperationTab() const { return m_pOperationTab; }
    inline QDeviceControlTab* getDeviceControlTab() const { return m_pDeviceControlTab; }
	inline QVisualizationTab* getVisualizationTab() const { return m_pVisualizationTab; }
	inline QCheckBox* getImageStitchingCheckBox() const { return m_pCheckBox_StitchingMode; }
	inline QLabel* getXStepLabel() const { return m_pLabel_XStep; }
	inline QLabel* getYStepLabel() const { return m_pLabel_YStep; }
	inline QLineEdit* getXStepLineEdit() const { return m_pLineEdit_XStep; }
	inline QLineEdit* getYStepLineEdit() const { return m_pLineEdit_YStep; }
	
public:
    void setWidgetsText();
    void setImageSizeWidgets(bool enabled);
	void setAveragingWidgets(bool enabled);

private:		
// Set thread callback objects
    void setFlimAcquisitionCallback();
    void setFlimProcessingCallback();
    void setVisualizationCallback();

private slots:
    void changeImageSize(int);
	void changeAveragingFrame(const QString &);
	void enableStitchingMode(bool);
	void changeStitchingXStep(const QString &);
	void changeStitchingYStep(const QString &);
	void changeStitchingMisSyncPos(const QString &);
    void processMessage(QString, bool);

signals:
    void sendStatusMessage(QString, bool);

// Variables ////////////////////////////////////////////
private:
    MainWindow* m_pMainWnd;
    Configuration* m_pConfig;

    QOperationTab* m_pOperationTab;
    QDeviceControlTab* m_pDeviceControlTab;
    QVisualizationTab* m_pVisualizationTab;

	// Message Window
	QListWidget *m_pListWidget_MsgWnd;

public:
	// Image buffer
	np::FloatArray2 m_pTempIntensity;
	np::FloatArray2 m_pTempLifetime;
	np::FloatArray2 m_pNonNaNIndex;

public:
    // Thread manager objects
    ThreadManager* m_pThreadFlimProcess;
    ThreadManager* m_pThreadVisualization;

private:
    // Thread synchronization objects
    SyncObject<uint16_t> m_syncFlimProcessing;
    SyncObject<float> m_syncFlimVisualization;

private:
    // Layout
    QHBoxLayout *m_pHBoxLayout;

    // Group Boxes
    QGroupBox *m_pGroupBox_OperationTab;
    QGroupBox *m_pGroupBox_DeviceControlTab;
    QGroupBox *m_pGroupBox_VisualizationTab;
    QGroupBox *m_pGroupBox_ImageSizeTab;

    // Image size radio buttons
    QLabel *m_pLabel_ImageSize;

    QRadioButton *m_pRadioButton_128by128;
    QRadioButton *m_pRadioButton_256by256;
    QRadioButton *m_pRadioButton_512by512;
    QRadioButton *m_pRadioButton_1024by1024;

    QButtonGroup *m_pButtonGroup_ImageSize;

	// Image averaging mode 
	QLabel *m_pLabel_Averaging;
	QLineEdit *m_pLineEdit_Averaging;

	QLabel *m_pLabel_AcquisitionStatus;
	QLabel *m_pLabel_AcquisitionStatusMsg;

	// Image stitching mode
	QCheckBox *m_pCheckBox_StitchingMode;
	QLineEdit *m_pLineEdit_XStep;
	QLineEdit *m_pLineEdit_YStep;
	QLabel *m_pLabel_XStep;
	QLabel *m_pLabel_YStep;
	QLineEdit *m_pLineEdit_MisSyncPos;
	QLabel *m_pLabel_MisSyncPos;
};

#endif // QSTREAMTAB_H
