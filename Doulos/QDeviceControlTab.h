#ifndef QDEVICECONTROLTAB_H
#define QDEVICECONTROLTAB_H

#include <QDialog>
#include <QtWidgets>
#include <QtCore>

#include <Doulos/Configuration.h>

#include <iostream>
#include <mutex>
#include <condition_variable>


class QStreamTab;

class PmtGainControl;
class FlimTrigger;
class ElforlightLaser;
class FlimCalibDlg;

class GalvoScan;
class ZaberStage;

class QImageView;

class QMySpinBox : public QDoubleSpinBox
{
public:
    explicit QMySpinBox(QWidget *parent = nullptr) : QDoubleSpinBox(parent)
    {
        lineEdit()->setReadOnly(true);
    }
    virtual ~QMySpinBox() {}
};


class QDeviceControlTab : public QDialog
{
    Q_OBJECT

// Constructer & Destructer /////////////////////////////
public:
    explicit QDeviceControlTab(QWidget *parent = nullptr);
    virtual ~QDeviceControlTab();

// Methods //////////////////////////////////////////////
protected:
	void closeEvent(QCloseEvent*);

public:
	void setControlsStatus(bool enabled);

public: ////////////////////////////////////////////////////////////////////////////////////////////////
    inline QVBoxLayout* getLayout() const { return m_pVBoxLayout; }
    inline QStreamTab* getStreamTab() const { return m_pStreamTab; }
    inline FlimCalibDlg* getFlimCalibDlg() const { return m_pFlimCalibDlg; }
    inline QCheckBox* getPX14DigitizerControl() const { return m_pCheckBox_PX14DigitizerControl; }
	inline QCheckBox* getFlimLaserTrigControl() const { return m_pCheckBox_FlimLaserTrigControl; }
	inline QCheckBox* getPmtGainControl() const { return m_pCheckBox_PmtGainControl; }
    inline QCheckBox* getGalvoScanControl() const { return m_pCheckBox_GalvoScanControl; }
	inline GalvoScan* getGalvoScan() const { return m_pGalvoScan; }
	inline ZaberStage* getZaberStage() const { return m_pZaberStage; }

private: ////////////////////////////////////////////////////////////////////////////////////////////////
    void createPmtGainControl();
    void createFlimTriggeringControl();
	void createFlimLaserPowerControl();
    void createFLimCalibControl();
    void createGalvoScanControl();
	void createZaberStageControl();

private slots: /////////////////////////////////////////////////////////////////////////////////////////
    // FLIm PMT Gain Control
    void applyPmtGainVoltage(bool);
    void changePmtGainVoltage(const QString &);

    // FLIm Laser Trigger Control
    void startFlimTriggering(bool);
    void changeFlimLaserRepRate(const QString &);

    // FLIm Laser Power Control
    void connectFlimLaser(bool);
	void adjustLaserPower(int);

    // FLIm Calibration Control
    void connectPX14Digitizer(bool);
    void createFlimCalibDlg();
    void deleteFlimCalibDlg();

    // Galvano Mirror
    void connectGalvanoMirror(bool);
    void changeGalvoFastScanVoltage(const QString &);
    void changeGalvoFastScanVoltageOffset(const QString &);
    void changeGalvoSlowScanVoltage(const QString &);
    void changeGalvoSlowScanVoltageOffset(const QString &);
	void changeFlyingBack(const QString &);

	// Zaber Stage Control
	void connectZaberStage(bool);
	void moveRelative();
	void setTargetSpeed(const QString &);
	void changeZaberPullbackLength(const QString &);
	void home();
	void stop();

// Variables ////////////////////////////////////////////
public: // Stage scanning
	std::mutex m_mtxStageScan;
	std::condition_variable m_cvStageScan;

private: ////////////////////////////////////////////////////////////////////////////////////////////////
    // PMT Gain Control & FLIm Laser Triggering
    PmtGainControl* m_pPmtGainControl;
    FlimTrigger* m_pFlimTrigControl;

	// Elforlight Laser Control
	ElforlightLaser* m_pElforlightLaser;

    // Galvo Scanner Control
    GalvoScan* m_pGalvoScan;

	// Zaber Stage Control;
	ZaberStage* m_pZaberStage;
	
private: ////////////////////////////////////////////////////////////////////////////////////////////////
    QStreamTab* m_pStreamTab;
    Configuration* m_pConfig;

    // Layout
    QVBoxLayout *m_pVBoxLayout;

	// FLIM Layout
	QVBoxLayout *m_pVBoxLayout_FlimControl;
    QGroupBox *m_pGroupBox_FlimControl;

    // Widgets for FLIm control	// Gain control
    QCheckBox *m_pCheckBox_PmtGainControl;
    QLineEdit *m_pLineEdit_PmtGainVoltage;
    QLabel *m_pLabel_PmtGainVoltage;

    // Widgets for FLIm control // Laser trig control
    QCheckBox *m_pCheckBox_FlimLaserTrigControl;
    QLineEdit *m_pLineEdit_RepetitionRate;
    QLabel *m_pLabel_Hertz;

    // Widgets for FLIm control // Laser power control
    QCheckBox *m_pCheckBox_FlimLaserPowerControl;
	QSpinBox *m_pSpinBox_FlimLaserPowerControl;

    // Widgets for FLIm control // Calibration
    QCheckBox *m_pCheckBox_PX14DigitizerControl;
    QPushButton *m_pPushButton_FlimCalibration;
    FlimCalibDlg *m_pFlimCalibDlg;

    // Widgets for galvano mirror control
    QCheckBox *m_pCheckBox_GalvoScanControl;
    QLineEdit *m_pLineEdit_FastPeakToPeakVoltage;
    QLineEdit *m_pLineEdit_FastOffsetVoltage;
    QLineEdit *m_pLineEdit_SlowPeakToPeakVoltage;
    QLineEdit *m_pLineEdit_SlowOffsetVoltage;
    QLabel *m_pLabel_FastScanVoltage;
    QLabel *m_pLabel_FastScanPlusMinus;
    QLabel *m_pLabel_FastGalvanoVoltage;
    QLabel *m_pLabel_SlowScanVoltage;
    QLabel *m_pLabel_SlowScanPlusMinus;
    QLabel *m_pLabel_SlowGalvanoVoltage;
	QLabel *m_pLabel_FlyingBack;
	QLineEdit *m_pLineEdit_FlyingBack;
	
	// Widgets for Zaber stage control
	QCheckBox *m_pCheckBox_ZaberStageControl;
	QPushButton *m_pPushButton_SetStageNumber;
	QPushButton *m_pPushButton_SetTargetSpeed;
	QPushButton *m_pPushButton_MoveRelative;
	QPushButton *m_pPushButton_Home;
	QPushButton *m_pPushButton_Stop;
	QComboBox *m_pComboBox_StageNumber;
	QLineEdit *m_pLineEdit_TargetSpeed;
	QLineEdit *m_pLineEdit_TravelLength;
	QLabel *m_pLabel_TargetSpeed;
	QLabel *m_pLabel_TravelLength;
	QLabel *m_pLabel_ScanningStatus;
};

#endif // QDEVICECONTROLTAB_H
