
#include "FLImProcess.h"


FLImProcess::FLImProcess()
{
}

FLImProcess::~FLImProcess()
{
}


void FLImProcess::operator() (FloatArray2& intensity, FloatArray2& mean_delay, FloatArray2& lifetime, Uint16Array2& pulse)
{
    // 1. Crop and resize pulse data
    _resize(pulse, _params);

    // 2. Get intensity
    _intensity(_resize);
    memcpy(intensity, _intensity.intensity, sizeof(float) * _intensity.intensity.length());

    // 3. Get lifetime
    _lifetime(_resize, _params, intensity);
    memcpy(mean_delay, _lifetime.mean_delay, sizeof(float) * _lifetime.mean_delay.length());
    memcpy(lifetime, _lifetime.lifetime, sizeof(float) * _lifetime.lifetime.length());
}




void FLImProcess::setParameters(Configuration* pConfig)
{
    _params.bg = pConfig->flimBg;
//    _params.pre_trig = pConfig->preTrigSamps;

    _params.samp_intv = 1000.0f / (float)PX14_ADC_RATE;
    _params.width_factor = 2.0f;

    for (int i = 0; i < 4; i++)
    {
        _params.ch_start_ind[i] = pConfig->flimChStartInd[i];
        if (i != 0)
             _params.delay_offset[i - 1] = pConfig->flimDelayOffset[i - 1];
    }
    _params.ch_start_ind[4] = _params.ch_start_ind[3] + FLIM_CH_START_5;
}

void FLImProcess::saveMaskData(QString maskpath)
{
    qint64 sizeRead;

    // create file (flim mask)
    QFile maskFile(maskpath);
    if (false != maskFile.open(QIODevice::WriteOnly))
    {
        sizeRead = maskFile.write(reinterpret_cast<char*>(_resize.pMask), sizeof(float) * _resize.nx);
        maskFile.close();
    }
}

void FLImProcess::loadMaskData(QString maskpath)
{
}
