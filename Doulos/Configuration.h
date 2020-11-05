#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#define VERSION						"2.0.0"

#define POWER_2(x)					(1 << x)
#define NEAR_2_POWER(x)				(int)(1 << (int)ceil(log2(x)))

//////////////////////// FLIM Setup /////////////////////////
#define PX14_ADC_RATE               400 // MHz

#define PX14_VOLTAGE_OFFSET			0.220
#define PX14_VOLTAGE_RATIO      	1.122018
#define PX14_VOLTAGE_RANGE          24

#define PX14_BOOTBUF_IDX            3

#define FLIM_SCANS                  512
#define FLIM_ALINES                 1024

#define NI_PMT_GAIN_CHANNEL		    "Dev1/ao2"
#define NI_FLIM_TRIG_CHANNEL		"Dev1/ctr0"

#define ELFORLIGHT_PORT				"COM1"

//////////////////////// Scan Setup /////////////////////////
#define NI_GALVO_CHANNEL			"Dev1/ao0:1"
#define NI_GAVLO_SOURCE				"/Dev1/PFI12"

#define ZABER_PORT					"\\\\.\\COM13"
#define ZABER_MAX_MICRO_RESOLUTION  128 // BENCHTOP_MODE ? 128 : 64; 
#define ZABER_MICRO_RESOLUTION		32 
#define ZABER_CONVERSION_FACTOR		1 / 9.375
#define ZABER_MICRO_STEPSIZE		0.099218755 // micro-meter ///


//////////////// Thread & Buffer Processing /////////////////
#define PROCESSING_BUFFER_SIZE		100


///////////////////// FLIm Processing ///////////////////////
#define FLIM_CH_START_5				35
#define GAUSSIAN_FILTER_WIDTH		200
#define GAUSSIAN_FILTER_STD			48
#define FLIM_SPLINE_FACTOR			20
#define INTENSITY_THRES				0.05f

/////////////////////// Visualization ///////////////////////
#define INTENSITY_COLORTABLE		6 // fire

#define RENEWAL_COUNT				1


template <typename T>
struct Range
{
	T min = 0;
	T max = 0;
};

enum voltage_range
{
    v0_220 = 1, v0_247, v0_277, v0_311, v0_349,
    v0_391, v0_439, v0_493, v0_553, v0_620,
    v0_696, v0_781, v0_876, v0_983, v1_103,
    v1_237, v1_388, v1_557, v1_748, v1_961,
    v2_200, v2_468, v2_770, v3_108, v3_487
};

#include <QString>
#include <QSettings>
#include <QDateTime>

#include <Common/callback.h>


class Configuration
{
public:
    explicit Configuration() : imageAveragingFrames(1) {}
	~Configuration() {}

public:
    void getConfigFile(QString inipath)
	{
		QSettings settings(inipath, QSettings::IniFormat);
		settings.beginGroup("configuration");

        // Image size
        flimScans = settings.value("flimScans").toInt();
        flimAlines = settings.value("flimAlines").toInt();
        flimFrameSize = flimScans * flimAlines;
        imageSize = settings.value("imageSize").toInt();
		if (imageSize == 0) imageSize = 128;
		
		// Image averaging
		imageAveragingFrames = settings.value("imageAveragingFrames").toInt();

		// Image stitching
		imageStichingXStep = settings.value("imageStichingXStep").toInt();
		imageStichingYStep = settings.value("imageStichingYStep").toInt();
		imageStichingMisSyncPos = settings.value("imageStichingMisSyncPos").toInt();

        // FLIm processing
		flimBg = settings.value("flimBg").toFloat();
		flimWidthFactor = settings.value("flimWidthFactor").toFloat();
		for (int i = 0; i < 4; i++)
		{
			flimChStartInd[i] = settings.value(QString("flimChStartInd_%1").arg(i)).toInt();
			if (i != 0)
				flimDelayOffset[i - 1] = settings.value(QString("flimDelayOffset_%1").arg(i)).toFloat();
        }

        // Visualization
        flimEmissionChannel = settings.value("flimEmissionChannel").toInt();
        flimLifetimeColorTable = settings.value("flimLifetimeColorTable").toInt();
		for (int i = 0; i < 3; i++)
		{
			flimIntensityRange[i].max = settings.value(QString("flimIntensityRangeMax_Ch%1").arg(i + 1)).toFloat();
			flimIntensityRange[i].min = settings.value(QString("flimIntensityRangeMin_Ch%1").arg(i + 1)).toFloat();
			flimLifetimeRange[i].max = settings.value(QString("flimLifetimeRangeMax_Ch%1").arg(i + 1)).toFloat();
			flimLifetimeRange[i].min = settings.value(QString("flimLifetimeRangeMin_Ch%1").arg(i + 1)).toFloat();
		}

		// Device control
        pmtGainVoltage = settings.value("pmtGainVoltage").toFloat();
        flimLaserRepRate = settings.value("flimLaserRepRate").toInt();
        galvoFastScanVoltage = settings.value("galvoFastScanVoltage").toFloat();
        galvoFastScanVoltageOffset = settings.value("galvoFastScanVoltageOffset").toFloat();
        galvoSlowScanVoltage = settings.value("galvoSlowScanVoltage").toFloat();
        galvoSlowScanVoltageOffset = settings.value("galvoSlowScanVoltageOffset").toFloat();
        galvoSlowScanIncrement = settings.value("galvoSlowScanIncrement").toFloat();
		galvoFlyingBack = settings.value("galvoFlyingBack").toInt();
		zaberPullbackSpeed = settings.value("zaberPullbackSpeed").toInt();
		zaberPullbackLength = settings.value("zaberPullbackLength").toInt();

		settings.endGroup();
	}

	void setConfigFile(QString inipath)
	{
		QSettings settings(inipath, QSettings::IniFormat);
		settings.beginGroup("configuration");
		
        // Image size
        settings.setValue("flimScans", flimScans);
        settings.setValue("flimAlines", flimAlines);
        settings.setValue("imageSize", imageSize);

		// Image averaging
		settings.setValue("imageAveragingFrames", imageAveragingFrames);

		// Image stitching
		settings.setValue("imageStichingXStep", imageStichingXStep);
		settings.setValue("imageStichingYStep", imageStichingYStep);
		settings.setValue("imageStichingMisSyncPos", imageStichingMisSyncPos);

        // FLIm processing
		settings.setValue("flimBg", QString::number(flimBg, 'f', 2));
		settings.setValue("flimWidthFactor", QString::number(flimWidthFactor, 'f', 2)); 
		for (int i = 0; i < 4; i++)
		{
			settings.setValue(QString("flimChStartInd_%1").arg(i), flimChStartInd[i]);
			if (i != 0)
				settings.setValue(QString("flimDelayOffset_%1").arg(i), QString::number(flimDelayOffset[i - 1], 'f', 3));
		}

        // Visualization
        settings.setValue("flimEmissionChannel", flimEmissionChannel);
		settings.setValue("flimLifetimeColorTable", flimLifetimeColorTable);
		for (int i = 0; i < 3; i++)
		{
			settings.setValue(QString("flimIntensityRangeMax_Ch%1").arg(i + 1), QString::number(flimIntensityRange[i].max, 'f', 1));
			settings.setValue(QString("flimIntensityRangeMin_Ch%1").arg(i + 1), QString::number(flimIntensityRange[i].min, 'f', 1));
			settings.setValue(QString("flimLifetimeRangeMax_Ch%1").arg(i + 1), QString::number(flimLifetimeRange[i].max, 'f', 1));
			settings.setValue(QString("flimLifetimeRangeMin_Ch%1").arg(i + 1), QString::number(flimLifetimeRange[i].min, 'f', 1));
		}

		// Device control
        settings.setValue("pmtGainVoltage", QString::number(pmtGainVoltage, 'f', 2));
        settings.setValue("flimLaserRepRate", flimLaserRepRate);
        settings.setValue("galvoFastScanVoltage", QString::number(galvoFastScanVoltage, 'f', 1));
        settings.setValue("galvoFastScanVoltageOffset", QString::number(galvoFastScanVoltageOffset, 'f', 1));
        settings.setValue("galvoSlowScanVoltage", QString::number(galvoSlowScanVoltage, 'f', 1));
        settings.setValue("galvoSlowScanVoltageOffset", QString::number(galvoSlowScanVoltageOffset, 'f', 1));
        settings.setValue("galvoSlowScanIncrement", QString::number(galvoSlowScanIncrement, 'f', 3));
		settings.setValue("galvoFlyingBack", galvoFlyingBack);
		settings.setValue("zaberPullbackSpeed", zaberPullbackSpeed);
		settings.setValue("zaberPullbackLength", zaberPullbackLength);

		// Current Time
		QDate date = QDate::currentDate();
		QTime time = QTime::currentTime();
		settings.setValue("time", QString("%1-%2-%3 %4-%5-%6")
			.arg(date.year()).arg(date.month(), 2, 10, (QChar)'0').arg(date.day(), 2, 10, (QChar)'0')
			.arg(time.hour(), 2, 10, (QChar)'0').arg(time.minute(), 2, 10, (QChar)'0').arg(time.second(), 2, 10, (QChar)'0'));

		settings.endGroup();
	}
	
public:
    // Image size
    int flimScans, flimAlines, flimFrameSize;
    int imageSize;

	// Image averaging
	int imageAveragingFrames;

	// Image stitching
	int imageStichingXStep;
	int imageStichingYStep;
	int imageStichingMisSyncPos;

    // FLIm processing
	float flimBg;
	float flimWidthFactor;
	int flimChStartInd[4];
    float flimDelayOffset[3];

	// Visualization    
    int flimEmissionChannel;
    int flimLifetimeColorTable;
    Range<float> flimIntensityRange[3];
    Range<float> flimLifetimeRange[3];

	// Device control
    float pmtGainVoltage;
    int flimLaserRepRate;
    float galvoFastScanVoltage;
    float galvoFastScanVoltageOffset;
    float galvoSlowScanVoltage;
    float galvoSlowScanVoltageOffset;
    float galvoSlowScanIncrement;
	int galvoFlyingBack;

	int zaberPullbackSpeed;
	int zaberPullbackLength;

	// Message callback
	callback<const char*> msgHandle;
};


#endif // CONFIGURATION_H
