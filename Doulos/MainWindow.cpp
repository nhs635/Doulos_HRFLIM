
#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <Doulos/Configuration.h>

#include <Doulos/QStreamTab.h>
#include <Doulos/QOperationTab.h>
#include <Doulos/QDeviceControlTab.h>

#include <Doulos/Dialog/FlimCalibDlg.h>

#include <DataAcquisition/DataAcquisition.h>
#include <DataAcquisition/FLImProcess/FLImProcess.h>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Initialize user interface
    QString windowTitle("Doulos [%1] v%2");
    this->setWindowTitle(windowTitle.arg("High resolution FLIM").arg(VERSION));

    // Create configuration object
    m_pConfiguration = new Configuration;
    m_pConfiguration->getConfigFile("Doulos.ini");

    m_pConfiguration->flimScans = FLIM_SCANS;
    m_pConfiguration->flimAlines = FLIM_ALINES;

    // Set timer for renew configuration
    m_pTimer = new QTimer(this);
    m_pTimer->start(5 * 60 * 1000); // renew per 5 min

    // Create tabs objects
    m_pStreamTab = new QStreamTab(this);

    // Create group boxes and tab widgets
    m_pTabWidget = new QTabWidget(this);
    m_pTabWidget->addTab(m_pStreamTab, tr("Real-Time Data Streaming"));
	
    // Create status bar
    QLabel *pStatusLabel_Temp1 = new QLabel(this); // System Message?
    m_pStatusLabel_ImagePos = new QLabel(QString("(%1, %2)").arg(0000, 4).arg(0000, 4), this);
    QLabel *pStatusLabel_Temp3 = new QLabel(this); // Device status?

    pStatusLabel_Temp1->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    m_pStatusLabel_ImagePos->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    pStatusLabel_Temp3->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    // then add the widget to the status bar
    statusBar()->addPermanentWidget(pStatusLabel_Temp1, 6);
    statusBar()->addPermanentWidget(m_pStatusLabel_ImagePos, 1);
    statusBar()->addPermanentWidget(pStatusLabel_Temp3, 2);

    // Set layout
    m_pGridLayout = new QGridLayout;
    m_pGridLayout->addWidget(m_pTabWidget, 0, 0);

    ui->centralWidget->setLayout(m_pGridLayout);

    this->setFixedSize(1280, 994);

    // Connect signal and slot
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(onTimer()));
    connect(m_pTabWidget, SIGNAL(currentChanged(int)), this, SLOT(changedTab(int)));
}

MainWindow::~MainWindow()
{
	m_pTimer->stop();

    if (m_pConfiguration)
    {
        m_pConfiguration->setConfigFile("Doulos.ini");
        delete m_pConfiguration;
    }

    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    if (m_pStreamTab->getOperationTab()->isAcquisitionButtonToggled())
        m_pStreamTab->getOperationTab()->setAcquisitionButton(false);

	m_pStreamTab->getDeviceControlTab()->setControlsStatus(false);

    if (m_pStreamTab->getDeviceControlTab()->getFlimCalibDlg())
        m_pStreamTab->getDeviceControlTab()->getFlimCalibDlg()->close();

    e->accept();
}


void MainWindow::onTimer()
{
    // Current Time should be added here.
    QDate date = QDate::currentDate();
    QTime time = QTime::currentTime();
    QString current = QString("%1-%2-%3 %4-%5-%6")
        .arg(date.year()).arg(date.month(), 2, 10, (QChar)'0').arg(date.day(), 2, 10, (QChar)'0')
        .arg(time.hour(), 2, 10, (QChar)'0').arg(time.minute(), 2, 10, (QChar)'0').arg(time.second(), 2, 10, (QChar)'0');

	m_pConfiguration->msgHandle(QString("[%1] Configuration data has been renewed!").arg(current).toLocal8Bit().data());

	m_pStreamTab->getOperationTab()->getDataAcq()->getFLIm()->saveMaskData();
    m_pConfiguration->setConfigFile("Doulos.ini");
}

void MainWindow::changedTab(int index)
{
    (void)index;
}
