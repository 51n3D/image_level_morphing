#include "action/ActionManager.h"

#include "DemoApp.h"
#include "utils/OpenCVToQtConvertor.h"
#include "widgets/FingerprintStepsDialog.h"

#include <utils/ImageProcessor.h>

#include <filesystem>

using namespace app::action;

namespace fs = std::filesystem;

ActionManager::ActionManager(QObject *parent) : QObject(parent), morpher(nullptr), matcher(new matching::Matcher())
{
	this->fingerprint = processing::storage::Fingerprint();
	this->alignedFingerprint = morphing::storage::AlignedFingerprint();

	processing::storage::Fingerprint f(cv::Mat::zeros(10, 10, CV_32F));
	f.setBlockSize(12).setWindowSize(30);

	this->processor = this->setUpFingerprintProcessor(f);
	this->morpher = this->setUpMorpher(*this->processor, f);
}

ActionManager::~ActionManager()
{
}

void ActionManager::startMorphing()
{
	emit reportProgress("======================================");
	emit reportProgress("morphing - start");

	emit morphingGettingReady();
	
	emit triggered();
}

void ActionManager::morph()
{
	this->morphedFingerprint = this->morpher->morph(this->alignedFingerprint, this->fingerprint);

	auto cutline = this->alignedFingerprint.getCutline();
	
	std::stringstream ss; ss << "Orientations: " << cutline.getSO() << " | Frequencies: " << cutline.getSV() << " | Minutiae: " << cutline.getSM();
	emit reportProgress(ss.str().c_str());
	ss.str("");

	ss << "Evaluation: " << cutline.getEval();
	emit reportProgress(ss.str().c_str());
	ss.str("");

	ss << "Separation: " << (cutline.getSeparation() == morphing::utils::storage::Cutline::POS_NEG ? "POS_NEG" : cutline.getSeparation() == morphing::utils::storage::Cutline::NEG_POS ? "NEG_POS" : "NONE") << std::endl;
	ss << "Cardinality F: Pos[" << cutline.getPosFCardinality() << "] Neg[" << cutline.getNegFCardinality() << "]" << std::endl;
	ss << "Cardinality AF: Pos[" << cutline.getPosAfCardinality() << "] Neg[" << cutline.getNegAfCardinality() << "]";
	emit reportProgress(ss.str().c_str());
	
	emit reportProgress("morphing - end");
	emit morphingDone(this->morphedFingerprint);
	emit reportProgress("======================================");
	emit reportProgress("Result: Fingerprints were morphed, new one was generated!");
}

void ActionManager::loadFingerprint(QString filename)
{
	if (filename.isEmpty())
	{
		return;
	}
	
	std::stringstream ss; ss << DemoApp::data.c_str() << DemoApp::sensors[DemoApp::currentSensor].toStdString() << "\\" << filename.toStdString() << ".bmp";
	const auto image = processing::utils::ImageProcessor::read(ss.str());

	this->fingerprint = processing::FingerprintProcessor::getFingerprint(image);
	this->fingerprint.fingerName = filename.toStdString();
	
	processing::FingerprintProcessor::adapt(this->fingerprint, 12, 30, 0.003);
	
	ss.str(""); ss <<  "Fingerprint1: " << filename.toStdString();
	emit reportProgress(QString(ss.str().c_str()));
}

void ActionManager::loadAlignedFingerprint(QString filename)
{
	if (filename.isEmpty())
	{
		return;
	}
	
	std::stringstream ss; ss << DemoApp::data.c_str() << DemoApp::sensors[DemoApp::currentSensor].toStdString() << "\\" << filename.toStdString() << ".bmp";
	const auto image = processing::utils::ImageProcessor::read(ss.str());

	this->alignedFingerprint = morphing::MorphingProcessor::getFingerprint(image);;
	this->alignedFingerprint.fingerName = filename.toStdString();
	
	processing::FingerprintProcessor::adapt(this->alignedFingerprint, 12, 30, 0.003);
	
	ss.str(""); ss << "Fingerprint2: " << filename.toStdString();
	emit reportProgress(QString(ss.str().c_str()));
}

void ActionManager::displayFingerprint1Steps()
{
	this->showFingerprintStepsDialog(this->fingerprint, "Fingerprint1 Steps");
}

void ActionManager::displayFingerprint2Steps()
{
	this->showAlignedFingerprintStepsDialog(this->alignedFingerprint, "Fingerprint2 Steps");
}

void ActionManager::setAlignmentThreshold(const int threshold)
{
	this->morpher->setMinimalAlignment(static_cast<float>(threshold) / 100);
}

void ActionManager::setCutlineWeights(const double oW, const double vW, const double mW)
{
	this->morpher->setCutlineWeights(oW, vW, mW);
}

void ActionManager::setCutlineIntersectionAreaWidth(const int dMax)
{
	this->morpher->setCutlineDMax(dMax);
}

void ActionManager::setCutlineRotationStep(const int rStep)
{
	this->morpher->setCutlineRotations(CV_PI / rStep);
}

void ActionManager::setUseDynamicCutline(const bool useDynamicCutline)
{
	this->morpher->useDynamicCutline(useDynamicCutline);
}

void ActionManager::setUseAdaptiveMethod(bool useAdaptiveMethod)
{
	this->morpher->useAdaptiveMethod(useAdaptiveMethod);
}

void ActionManager::setDynamicCutlineSizeOfSearchArea(const int dynamicSearchSizeArea)
{
	this->morpher->setDynamicCutlineSearchArea(dynamicSearchSizeArea);
}

void ActionManager::setGeneratorBorderWidth(const int borderWidth)
{
	this->morpher->setBorderWidth(borderWidth);
}

void ActionManager::setBackgroundColor(const int backgroundColor)
{
	this->morpher->setTemplateBackground(backgroundColor);
}

void ActionManager::matchFingerprintsWithMorphed()
{
	fs::create_directories("cache");
	
	const auto tpl1 = "cache\\1.txt";
	matching::Matcher::createTxtMinutiaTemplate(this->fingerprint, tpl1);
	const auto tpl2 = "cache\\2.txt";
	matching::Matcher::createTxtMinutiaTemplate(this->alignedFingerprint, tpl2);
	const auto tpl3 = "cache\\3.txt";
	matching::Matcher::createTxtMinutiaTemplate(this->morphedFingerprint, tpl3);

	std::stringstream ss; ss << "Fingerprint " << this->fingerprint.fingerName << " with morphed; Matching score: " << matcher->match(tpl1, tpl3);
	emit reportProgress(ss.str().c_str());
	ss.str(""); ss << "Fingerprint " << this->alignedFingerprint.fingerName << " with morphed; Matching score: " << matcher->match(tpl2, tpl3);
	emit reportProgress(ss.str().c_str());
}

void ActionManager::showFingerprintStepsDialog(const processing::storage::Fingerprint& fingerprint, const QString& dialogName)
{
	widgets::FingerprintStepsDialog dialog(nullptr);
	dialog.setObjectName(dialogName);

	const auto orientations = this->processor->getOrientationsImage(fingerprint);
	const auto enhanced = this->processor->getEnhancedImage(fingerprint);
	const auto minutiaeTracing = this->processor->getMinutiaeTracing(fingerprint);
	const auto minutiae = this->processor->getMinutiaeImage(fingerprint);
	
	dialog.addImages({
		utils::OpenCVToQtConvertor::matToQtPixmap(utils::OpenCVToQtConvertor::adaptColorRangesToQt(orientations), QSize(300, 400)),
		utils::OpenCVToQtConvertor::matToQtPixmap(utils::OpenCVToQtConvertor::adaptColorRangesToQt(enhanced), QSize(300, 400)),
		utils::OpenCVToQtConvertor::matToQtPixmap(utils::OpenCVToQtConvertor::adaptColorRangesToQt(minutiaeTracing, true), QSize(300, 400), false),
		utils::OpenCVToQtConvertor::matToQtPixmap(utils::OpenCVToQtConvertor::adaptColorRangesToQt(minutiae, true), QSize(300, 400), false),
	});

	dialog.setWindowTitle("Processing Steps (first fingerprint)");
	
	dialog.setModal(true);
	dialog.exec();
}

void ActionManager::showAlignedFingerprintStepsDialog(const morphing::storage::AlignedFingerprint& fingerprint, const QString& dialogName)
{
	widgets::FingerprintStepsDialog dialog(nullptr);
	dialog.setObjectName(dialogName);

	const auto orientations = this->processor->getOrientationsImage(fingerprint);
	const auto enhanced = this->processor->getEnhancedImage(fingerprint);
	const auto minutiaeTracing = this->processor->getMinutiaeTracing(fingerprint);
	const auto minutiae = this->processor->getMinutiaeImage(fingerprint.getAligned(), fingerprint.getMinutiae());

	dialog.addImages({
		utils::OpenCVToQtConvertor::matToQtPixmap(utils::OpenCVToQtConvertor::adaptColorRangesToQt(orientations), QSize(300, 400)),
		utils::OpenCVToQtConvertor::matToQtPixmap(utils::OpenCVToQtConvertor::adaptColorRangesToQt(enhanced), QSize(300, 400)),
		utils::OpenCVToQtConvertor::matToQtPixmap(utils::OpenCVToQtConvertor::adaptColorRangesToQt(minutiaeTracing, true), QSize(300, 400), false),
		utils::OpenCVToQtConvertor::matToQtPixmap(utils::OpenCVToQtConvertor::adaptColorRangesToQt(minutiae, true), QSize(300, 400), false),
	});

	dialog.setWindowTitle("Processing Steps (second fingerprint)");
	
	dialog.setModal(true);
	dialog.exec();
}

void ActionManager::displayMorphingSteps()
{
	widgets::FingerprintStepsDialog dialog(nullptr);
	dialog.setObjectName("Morphing Steps");

	const auto fingerprint1 = this->fingerprint;
	const auto fingerprint2 = this->alignedFingerprint.getAligned();

	cv::Mat f1(fingerprint1.size(), CV_8UC3);
	processing::utils::ImageProcessor::convertTo(fingerprint1, f1, CV_8U);
	processing::utils::ImageProcessor::addColorBreathToImage(f1, cv::Scalar(1, 1.7, 1, 1));

	cv::Mat f2(fingerprint2.size(), CV_8UC3);
	processing::utils::ImageProcessor::convertTo(fingerprint2, f2, CV_8U);
	processing::utils::ImageProcessor::addColorBreathToImage(f2, cv::Scalar(1.7, 1, 1, 1));

	const auto alignedImage = this->morpher->getAlignedImage(this->alignedFingerprint, this->fingerprint);
	const auto cutlineImage = this->morpher->getCutlineImage(this->alignedFingerprint, this->fingerprint);
	const auto morphedImage = this->morpher->getMorphedImage(this->alignedFingerprint, this->fingerprint);

	dialog.addImages({
		utils::OpenCVToQtConvertor::matToQtPixmap(utils::OpenCVToQtConvertor::adaptColorRangesToQt(f1, true), QSize(300, 400), false),
		utils::OpenCVToQtConvertor::matToQtPixmap(utils::OpenCVToQtConvertor::adaptColorRangesToQt(f2, true), QSize(300, 400), false),
		utils::OpenCVToQtConvertor::matToQtPixmap(utils::OpenCVToQtConvertor::adaptColorRangesToQt(alignedImage, true), QSize(300, 400), false),
		utils::OpenCVToQtConvertor::matToQtPixmap(utils::OpenCVToQtConvertor::adaptColorRangesToQt(cutlineImage, true), QSize(300, 400), false),
		utils::OpenCVToQtConvertor::matToQtPixmap(utils::OpenCVToQtConvertor::adaptColorRangesToQt(morphedImage, true), QSize(300, 400), false),
	});

	dialog.setWindowTitle("Morphing Steps");
	
	dialog.setModal(true);
	dialog.exec();
}

morphing::MorphingProcessor* ActionManager::setUpMorpher(processing::FingerprintProcessor& processor, processing::storage::Fingerprint f)
{
	const auto trashHold_aligner = 0.7f;
	const auto rStep = 9;
	const auto lines = 60;
	const auto dmax = 30;
	
	morphing::utils::FingerprintAligner aligner(processor);
	aligner
		.setTranslationStep(f.getBlockSize())
		.setRotationStep(rStep)
		.setTrashHold(trashHold_aligner)
		.showCommonArea(true);

	morphing::utils::CutlineEstimator cutline(processor);
	cutline
		.setRotationStep(CV_PI / lines)
		.setMaxDistance(dmax)
		.showMinutiaes(true);

	morphing::utils::TemplateGenerator generator;

	return new morphing::MorphingProcessor(processor, aligner, cutline, generator);
}

processing::FingerprintProcessor* ActionManager::setUpFingerprintProcessor(const processing::storage::Fingerprint& f)
{
	processing::utils::OrientationsEstimator orientations;
	orientations
		.setBlockSize(f.getBlockSize())
		.useLowPassFilter();

	processing::utils::FrequenciesEstimator frequencies;
	frequencies
		.setBlockSize(4)
		.setWindowSize(f.getWindowsSize())
		.interpolate();

	processing::utils::GaborFilter filter;
	filter
		.setDeviation(4);

	processing::utils::MinutiaeEstimator minutiaes;
	minutiaes
		.setBlockSize(f.getBlockSize());

	const processing::utils::FakeMinutiaeDetector detector;

	return new processing::FingerprintProcessor(orientations, frequencies, filter, minutiaes, detector);
}
