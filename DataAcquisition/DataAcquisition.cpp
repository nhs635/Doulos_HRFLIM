
#include "DataAcquisition.h"

#include <DataAcquisition/SignatecDAQ/SignatecDAQ.h>
#include <DataAcquisition/FLImProcess/FLImProcess.h>


DataAcquisition::DataAcquisition(Configuration* pConfig)
    : m_pDaq(nullptr), m_pFLIm(nullptr)
{
    m_pConfig = pConfig;

    // Create SignatecDAQ object
    m_pDaq = new SignatecDAQ;
    m_pDaq->DidStopData += [&]() { m_pDaq->_running = false; };

    // Create FLIm process object
    m_pFLIm = new FLImProcess;
	m_pFLIm->SendStatusMessage += [&](const char* msg) { m_pConfig->msgHandle(msg); };
	m_pFLIm->_resize.SendStatusMessage += [&](const char* msg) { m_pConfig->msgHandle(msg); };
    m_pFLIm->setParameters(m_pConfig);
    m_pFLIm->_resize(np::Uint16Array2(m_pConfig->flimScans, m_pConfig->flimAlines), m_pFLIm->_params);
    m_pFLIm->loadMaskData();
}

DataAcquisition::~DataAcquisition()
{
    if (m_pDaq) delete m_pDaq;
    if (m_pFLIm) delete m_pFLIm;
}


bool DataAcquisition::InitializeAcquistion()
{
    // Set boot-time buffer
    SetBootTimeBufCfg(PX14_BOOTBUF_IDX, sizeof(uint16_t) * m_pConfig->flimScans * m_pConfig->flimAlines);
	 
    // Parameter settings for DAQ & Axsun Capture
    m_pDaq->nScans = m_pConfig->flimScans;
    m_pDaq->nAlines = m_pConfig->flimAlines;
    m_pDaq->BootTimeBufIdx = PX14_BOOTBUF_IDX;
    m_pDaq->VoltRange2 = PX14_VOLTAGE_RANGE;

    // Initialization for DAQ
    if (!(m_pDaq->set_init()))
    {
        StopAcquisition();
        return false;
    }

    return true;
}

bool DataAcquisition::StartAcquisition()
{
    // Start acquisition
    if (!(m_pDaq->startAcquisition()))
    {
        StopAcquisition();
        return false;
    }

    return true;
}

void DataAcquisition::StopAcquisition()
{
    // Stop thread
    m_pDaq->stopAcquisition();
}


void DataAcquisition::GetBootTimeBufCfg(int idx, int& buffer_size)
{
    buffer_size = m_pDaq->getBootTimeBuffer(idx);
}

void DataAcquisition::SetBootTimeBufCfg(int idx, int buffer_size)
{
    m_pDaq->setBootTimeBuffer(idx, buffer_size);
}


void DataAcquisition::ConnectDaqAcquiredFlimData(const std::function<void(int, const np::Array<uint16_t, 2>&)> &slot)
{
    m_pDaq->DidAcquireData += slot;
}

void DataAcquisition::ConnectDaqStopFlimData(const std::function<void(void)> &slot)
{
    m_pDaq->DidStopData += slot;
}

void DataAcquisition::ConnectDaqSendStatusMessage(const std::function<void(const char*, bool)> &slot)
{
    m_pDaq->SendStatusMessage += slot;
}
