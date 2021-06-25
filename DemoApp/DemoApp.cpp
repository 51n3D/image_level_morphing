#include "DemoApp.h"
#include "utils/OpenCVToQtConvertor.h"

#include <opencv2/opencv.hpp>

using namespace app::widgets;
using namespace app::utils;
using namespace app;
using namespace cv;

const std::string DemoApp::data = "Resources\\Fingerprints\\";
 std::map<int, QString> DemoApp::sensors = {
			{SensorTypes::S1, "BERGDATA"},
			{SensorTypes::S2, "SECUGEN"},
			{SensorTypes::S3, "SAGEMMSO"},
			{SensorTypes::SYNTHETIC, "SYNTHETIC"},
};
int DemoApp::currentSensor = SensorTypes::SYNTHETIC;

DemoApp::DemoApp(QWidget *parent) : QMainWindow(parent), manager(new action::ActionManager(this)), testing(new action::TestCase(this))
{
	this->ui.setupUi(this);

	this->setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
	this->statusBar()->setSizeGripEnabled(false);
}

void DemoApp::setUp()
{
	this->connect();

	emit preset();
	emit init();
}

void DemoApp::connect()
{
	this->connectPresets();

	this->connectMenu();
	
	this->connectMorphingTools();
	
	QObject::connect(this, SIGNAL(sensorChanged(int)), this->ui.selectFingerprint1, SLOT(loadData(int)));
	QObject::connect(this, SIGNAL(sensorChanged(int)), this->ui.selectFingerprint2, SLOT(loadData(int)));
	QObject::connect(this, SIGNAL(sensorChanged(int)), this->ui.Morphed, SLOT(clear()));
	QObject::connect(this, SIGNAL(sensorChanged(int)), this, SLOT(disableButtons()));
	QObject::connect(this, SIGNAL(sensorChanged(int)), this->ui.BackgroundColor, SLOT(setDefaultValue()));

	QObject::connect(this->ui.selectFingerprint1, SIGNAL(currentIndexChanged(QString)), this->ui.Fingerprint1, SLOT(setImage(QString)));
	QObject::connect(this->ui.selectFingerprint2, SIGNAL(currentIndexChanged(QString)), this->ui.Fingerprint2, SLOT(setImage(QString)));

	QObject::connect(this, SIGNAL(reportProgress(QString)), this->ui.Logger, SLOT(log(QString)));
	QObject::connect(this->ui.Fingerprint1, SIGNAL(reportProgress(QString)), this->ui.Logger, SLOT(log(QString)));
	QObject::connect(this->ui.Fingerprint2, SIGNAL(reportProgress(QString)), this->ui.Logger, SLOT(log(QString)));
	QObject::connect(this->manager, SIGNAL(reportProgress(QString)), this->ui.Logger, SLOT(log(QString)));
	QObject::connect(this->testing, SIGNAL(reportProgress(QString)), this->ui.Logger, SLOT(log(QString)));

	QObject::connect(this->ui.Fingerprint1StepsButton, SIGNAL(clicked()), this->manager, SLOT(displayFingerprint1Steps()));
	QObject::connect(this->ui.Fingerprint2StepsButton, SIGNAL(clicked()), this->manager, SLOT(displayFingerprint2Steps()));
	QObject::connect(this->ui.MorphingStepsButton, SIGNAL(clicked()), this->manager, SLOT(displayMorphingSteps()));
}

void DemoApp::connectPresets()
{
	// zarovnanie
	QObject::connect(this, SIGNAL(preset()), this->ui.AlignmentThreshold, SLOT(setDefaultValue()));

	// rezna linia
	QObject::connect(this, SIGNAL(preset()), this->ui.Ow, SLOT(setDefaultValue()));
	QObject::connect(this, SIGNAL(preset()), this->ui.Vw, SLOT(setDefaultValue()));
	QObject::connect(this, SIGNAL(preset()), this->ui.Mw, SLOT(setDefaultValue()));
	QObject::connect(this, SIGNAL(preset()), this->ui.IntersectionAreaWidth, SLOT(setDefaultValue()));
	QObject::connect(this, SIGNAL(preset()), this->ui.RotationStep, SLOT(setDefaultValue()));
	QObject::connect(this, SIGNAL(preset()), this->ui.DynamicCutline, SLOT(setDefaultValue()));
	QObject::connect(this, SIGNAL(preset()), this->ui.SearchArea, SLOT(setDefaultValue()));

	// generator
	QObject::connect(this, SIGNAL(preset()), this->ui.BorderWidth, SLOT(setDefaultValue()));
	QObject::connect(this, SIGNAL(preset()), this->ui.BackgroundColor, SLOT(setDefaultValue()));
	
	QObject::connect(this, SIGNAL(preset()), this->ui.Morphed, SLOT(clear()));
	QObject::connect(this, SIGNAL(preset()), this, SLOT(disableButtons()));

	QObject::connect(this, SIGNAL(init()), this, SLOT(setDefaultSensor()));
}

void DemoApp::connectMenu()
{
	// mennu senzorov
	QObject::connect(this->ui.BERGDATA, SIGNAL(triggered()), this, SLOT(setSensor()));
	QObject::connect(this->ui.SECUGEN, SIGNAL(triggered()), this, SLOT(setSensor()));
	QObject::connect(this->ui.SAGEMMSO, SIGNAL(triggered()), this, SLOT(setSensor()));
	QObject::connect(this->ui.SYNTHETIC, SIGNAL(triggered()), this, SLOT(setSensor()));

	// testovacie menu
	// testy
	QObject::connect(this->ui.testingProcessing, SIGNAL(triggered()), this->testing, SLOT(runProcessing()));
	QObject::connect(this->ui.testingStatic, SIGNAL(triggered()), this->testing, SLOT(runStaticMorphing()));
	QObject::connect(this->ui.testingDynamic, SIGNAL(triggered()), this->testing, SLOT(runDynamicMorphing()));
	QObject::connect(this->ui.testingAdaptiveStatic, SIGNAL(triggered()), this->testing, SLOT(runAdaptiveStaticMorphing()));
	QObject::connect(this->ui.testingAdaptiveDynamic, SIGNAL(triggered()), this->testing, SLOT(runAdaptiveDynamicMorphing()));
	// vysledky
	QObject::connect(this->ui.testingResultsStatic, SIGNAL(triggered()), this->testing, SLOT(showMatchingStats()));	
	QObject::connect(this->ui.testingResultsDynamic, SIGNAL(triggered()), this->testing, SLOT(showMatchingStats()));
	QObject::connect(this->ui.testingResultsAdaptiveStatic, SIGNAL(triggered()), this->testing, SLOT(showMatchingStats()));
	QObject::connect(this->ui.testingResultsAdaptiveDynamic, SIGNAL(triggered()), this->testing, SLOT(showMatchingStats()));

	// zavislosti senzorov a testov
	QObject::connect(this, SIGNAL(enableSynthetic(bool)), this, SLOT(enableTestingResultsTab(bool)));
}

void DemoApp::connectMorphingTools()
{
	QObject::connect(this->ui.MorphingButton, SIGNAL(clicked()), this, SLOT(setupParameters()));

	QObject::connect(this, SIGNAL(sentFingerprint(QString)), this->manager, SLOT(loadFingerprint(QString)));
	QObject::connect(this, SIGNAL(sentAlignedFingerprint(QString)), this->manager, SLOT(loadAlignedFingerprint(QString)));
	
	QObject::connect(this, SIGNAL(sentAlignmentThreshold(int)), this->manager, SLOT(setAlignmentThreshold(int)));
	QObject::connect(this, SIGNAL(sentCutlineWeights(double, double, double)), this->manager, SLOT(setCutlineWeights(double, double, double)));
	QObject::connect(this, SIGNAL(sentCutlineIntersectionAreaWidth(int)), this->manager, SLOT(setCutlineIntersectionAreaWidth(int)));
	QObject::connect(this, SIGNAL(sentCutlineRotationStep(int)), this->manager, SLOT(setCutlineRotationStep(int)));
	QObject::connect(this, SIGNAL(sentUseDynamicCutline(bool)), this->manager, SLOT(setUseDynamicCutline(bool)));
	QObject::connect(this, SIGNAL(sentUseAdaptiveMethod(bool)), this->manager, SLOT(setUseAdaptiveMethod(bool)));
	QObject::connect(this, SIGNAL(sentDynamicCutlineSizeOfSearchArea(int)), this->manager, SLOT(setDynamicCutlineSizeOfSearchArea(int)));
	QObject::connect(this, SIGNAL(sentGeneratorBorderWidth(int)), this->manager, SLOT(setGeneratorBorderWidth(int)));
	QObject::connect(this, SIGNAL(sentBackgroundColor(int)), this->manager, SLOT(setBackgroundColor(int)));
	
	QObject::connect(this, SIGNAL(parametersWereSet()), this->manager, SLOT(startMorphing()));
	QObject::connect(this->manager, SIGNAL(triggered()), this->manager, SLOT(morph()));
	
	QObject::connect(this->ui.ResetButton, SIGNAL(clicked()), this, SIGNAL(preset()));

	QObject::connect(this->manager, SIGNAL(morphingGettingReady()), this, SLOT(disableWidgets()));
	QObject::connect(this->manager, SIGNAL(morphingDone(const cv::Mat&)), this->ui.Morphed, SLOT(setMorphed(const cv::Mat&)));
	QObject::connect(this->manager, SIGNAL(morphingDone(const cv::Mat&)), this->manager, SLOT(matchFingerprintsWithMorphed()));
	QObject::connect(this->manager, SIGNAL(morphingDone(const cv::Mat&)), this, SLOT(enableWidgets()));
}

void DemoApp::setDefaultSensor()
{
	currentSensor = SensorTypes::SYNTHETIC;

	this->ui.selectFingerprint1->clear();
	this->ui.selectFingerprint2->clear();
	
	QString report("Selected sensor: "); report.append(sensors[currentSensor]);
	emit reportProgress(report);

	emit sensorChanged(currentSensor);

	emit enableSynthetic(true);
}


void DemoApp::setSensor()
{
	const auto menuButton = static_cast<QAction*>(sender());
	const auto sensor = menuButton->objectName();

	auto sensorType = SensorTypes::NONE;
	if (sensors[SensorTypes::S1] == sensor)
	{
		sensorType = SensorTypes::S1;

		emit enableSynthetic(false);
	}
	else if (sensors[SensorTypes::S2] == sensor)
	{
		sensorType = SensorTypes::S2;

		emit enableSynthetic(false);
	}
	else if (sensors[SensorTypes::S3] == sensor)
	{
		sensorType = SensorTypes::S3;
		
		emit enableSynthetic(false);
	}
	else if (sensors[SensorTypes::SYNTHETIC] == sensor)
	{
		sensorType = SensorTypes::SYNTHETIC;

		emit enableSynthetic(true);
	}

	this->ui.selectFingerprint1->clear();
	this->ui.selectFingerprint2->clear();

	currentSensor = sensorType;

	QString report("Selected sensor: "); report.append(sensors[currentSensor]);
	emit reportProgress(report);
	
	emit sensorChanged(currentSensor);
}

void DemoApp::disableButtons()
{
	this->ui.Fingerprint1StepsButton->setEnabled(false);
	this->ui.Fingerprint2StepsButton->setEnabled(false);
	this->ui.MorphingStepsButton->setEnabled(false);
}


void DemoApp::disableWidgets()
{
	this->ui.Fingerprint1StepsButton->setEnabled(false);
	this->ui.Fingerprint2StepsButton->setEnabled(false);
	this->ui.MorphingStepsButton->setEnabled(false);

	this->ui.selectFingerprint1->setEnabled(false);
	this->ui.selectFingerprint2->setEnabled(false);

	this->ui.AlignmentThreshold->setEnabled(false);

	this->ui.adaptiveMethod->setEnabled(false);
	this->ui.originalMethod->setEnabled(false);
	this->ui.Ow->setEnabled(false);
	this->ui.Vw->setEnabled(false);
	this->ui.Mw->setEnabled(false);
	this->ui.IntersectionAreaWidth->setEnabled(false);
	this->ui.RotationStep->setEnabled(false);
	this->ui.DynamicCutline->setEnabled(false);

	this->ui.BorderWidth->setEnabled(false);
	this->ui.BackgroundColor->setEnabled(false);
}

void DemoApp::enableWidgets()
{
	this->ui.Fingerprint1StepsButton->setEnabled(true);
	this->ui.Fingerprint2StepsButton->setEnabled(true);
	this->ui.MorphingStepsButton->setEnabled(true);

	this->ui.selectFingerprint1->setEnabled(true);
	this->ui.selectFingerprint2->setEnabled(true);

	this->ui.AlignmentThreshold->setEnabled(true);

	this->ui.adaptiveMethod->setEnabled(true);
	this->ui.originalMethod->setEnabled(true);
	this->ui.Ow->setEnabled(true);
	this->ui.Vw->setEnabled(true);
	this->ui.Mw->setEnabled(true);
	this->ui.IntersectionAreaWidth->setEnabled(true);
	this->ui.RotationStep->setEnabled(true);
	this->ui.DynamicCutline->setEnabled(true);

	this->ui.BorderWidth->setEnabled(true);
	this->ui.BackgroundColor->setEnabled(true);
}

void DemoApp::setupParameters()
{
	emit sentFingerprint(this->ui.selectFingerprint1->currentText());
	emit sentAlignedFingerprint(this->ui.selectFingerprint2->currentText());
	
	emit sentAlignmentThreshold(this->ui.AlignmentThreshold->value());
	emit sentCutlineWeights(this->ui.Ow->value(), this->ui.Vw->value(), this->ui.Mw->value());
	emit sentCutlineIntersectionAreaWidth(this->ui.IntersectionAreaWidth->value());
	emit sentCutlineRotationStep(this->ui.RotationStep->value());
	emit sentUseDynamicCutline(this->ui.DynamicCutline->isChecked());
	if (this->ui.adaptiveMethod->isChecked())
	{
		emit sentUseAdaptiveMethod(true);
	}
	else if (this->ui.originalMethod->isChecked())
	{
		emit sentUseAdaptiveMethod(false);
	}
	emit sentDynamicCutlineSizeOfSearchArea(this->ui.SearchArea->value());
	emit sentGeneratorBorderWidth(this->ui.BorderWidth->value());
	emit sentBackgroundColor(this->ui.BackgroundColor->value());

	emit parametersWereSet();
}

void DemoApp::enableTestingResultsTab(bool enabled)
{
	this->ui.testingResults->setEnabled(enabled);
}
