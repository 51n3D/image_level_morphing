#include "action/TestCase.h"

#include "DemoApp.h"

#include <utils/ImageProcessor.h>

#include <iostream>
#include <fstream>
#include <filesystem>

using namespace app::action;

namespace fs = std::filesystem;

TestCase::TestCase(QObject *parent) : QObject(parent), matcher(new matching::Matcher())
{
}

TestCase::~TestCase()
{
}

void TestCase::runProcessing()
{
	fs::create_directory("Results");
	fs::create_directory("Results\\processing");
	
	std::stringstream ss; ss << "Results\\processing\\" << DemoApp::sensors[DemoApp::currentSensor].toStdString();
	fs::create_directory(ss.str());
	fs::create_directory(QString(ss.str().c_str()).append("\\1_normalized").toStdString());
	fs::create_directory(QString(ss.str().c_str()).append("\\2_orientations").toStdString());
	fs::create_directory(QString(ss.str().c_str()).append("\\3_enhanced").toStdString());
	fs::create_directory(QString(ss.str().c_str()).append("\\4_tracing").toStdString());
	fs::create_directory(QString(ss.str().c_str()).append("\\5_minutiae").toStdString());

	ss.str(""); ss << "Resources\\Fingerprints\\" << DemoApp::sensors[DemoApp::currentSensor].toStdString();
	const std::string input = ss.str();
	ss.str(""); ss << "Results\\processing\\" << DemoApp::sensors[DemoApp::currentSensor].toStdString() << "\\";
	const std::string output = ss.str();

	runProcessing(input, output);
}

void TestCase::runAdaptiveStaticMorphing()
{
	fs::create_directory("Results");
	fs::create_directory("Results\\morphing");
	fs::create_directory("Results\\morphing\\adaptive_static");

	std::stringstream ss; ss << "Results\\morphing\\adaptive_static\\" << DemoApp::sensors[DemoApp::currentSensor].toStdString();
	fs::create_directory(ss.str().c_str());
	fs::create_directory(QString(ss.str().c_str()).append("\\1_aligned").toStdString());
	fs::create_directory(QString(ss.str().c_str()).append("\\2_cutline").toStdString());
	fs::create_directory(QString(ss.str().c_str()).append("\\3_morphed").toStdString());

	// setup entries for morphing
	ss.str(""); ss << "Resources\\Fingerprints\\" << DemoApp::sensors[DemoApp::currentSensor].toStdString();
	const auto path = ss.str();
	auto fingerprints = fs::directory_iterator(path);
	std::vector<fs::directory_entry> entries;
	for (auto& entry : fingerprints)
	{
		entries.emplace_back(entry);
	}

	// setup processors
	const auto blockSize = 12;
	const auto windowSize = 30;
	const auto trashHold_segmentation = 0.003;
	processing::storage::Fingerprint f(cv::Mat::zeros(10, 10, CV_32F));
	f.setBlockSize(blockSize).setWindowSize(windowSize);

	auto processor = setUpFingerprintProcessor(f);
	auto morpher = setUpMorphingProcessor(f, processor, false, true);
	ss.str(""); ss << "Results\\morphing\\adaptive_static\\" << DemoApp::sensors[DemoApp::currentSensor].toStdString() << "\\";
	morphing::MorphingProcessor::path = ss.str();

	// clear output statistics file
	ss.str(""); ss << morphing::MorphingProcessor::path << "match_results.txt";
	std::ofstream file(ss.str(), std::ofstream::out | std::ofstream::trunc);
	if (file.is_open())
	{
		file.close();
	}
	else { std::cerr << "Failed to clean the statistics file!" << std::endl; exit(-1); }

	morpher.setTemplateBackground(255);

	runMorphing(morpher, entries, blockSize, windowSize, trashHold_segmentation);
}

void TestCase::runAdaptiveDynamicMorphing()
{
	fs::create_directory("Results");
	fs::create_directory("Results\\morphing");
	fs::create_directory("Results\\morphing\\adaptive_dynamic");

	std::stringstream ss; ss << "Results\\morphing\\adaptive_dynamic\\" << DemoApp::sensors[DemoApp::currentSensor].toStdString();
	fs::create_directory(ss.str().c_str());
	fs::create_directory(QString(ss.str().c_str()).append("\\1_aligned").toStdString());
	fs::create_directory(QString(ss.str().c_str()).append("\\2_cutline").toStdString());
	fs::create_directory(QString(ss.str().c_str()).append("\\3_morphed").toStdString());

	// setup entries for morphing
	ss.str(""); ss << "Resources\\Fingerprints\\" << DemoApp::sensors[DemoApp::currentSensor].toStdString();
	const auto path = ss.str();
	auto fingerprints = fs::directory_iterator(path);
	std::vector<fs::directory_entry> entries;
	for (auto& entry : fingerprints)
	{
		entries.emplace_back(entry);
	}

	// setup processors
	const auto blockSize = 12;
	const auto windowSize = 30;
	const auto trashHold_segmentation = 0.003;
	processing::storage::Fingerprint f(cv::Mat::zeros(10, 10, CV_32F));
	f.setBlockSize(blockSize).setWindowSize(windowSize);

	auto processor = setUpFingerprintProcessor(f);
	auto morpher = setUpMorphingProcessor(f, processor, true, true);
	ss.str(""); ss << "Results\\morphing\\adaptive_dynamic\\" << DemoApp::sensors[DemoApp::currentSensor].toStdString() << "\\";
	morphing::MorphingProcessor::path = ss.str();

	// clear output statistics file
	ss.str(""); ss << morphing::MorphingProcessor::path << "match_results.txt";
	std::ofstream file(ss.str(), std::ofstream::out | std::ofstream::trunc);
	if (file.is_open())
	{
		file.close();
	}
	else { std::cerr << "Failed to clean the statistics file!" << std::endl; exit(-1); }

	morpher.setTemplateBackground(255);

	runMorphing(morpher, entries, blockSize, windowSize, trashHold_segmentation);
}

void TestCase::runStaticMorphing()
{
	fs::create_directory("Results");
	fs::create_directory("Results\\morphing");
	fs::create_directory("Results\\morphing\\static");
	
	std::stringstream ss; ss << "Results\\morphing\\static\\" << DemoApp::sensors[DemoApp::currentSensor].toStdString();
	fs::create_directory(ss.str().c_str());
	fs::create_directory(QString(ss.str().c_str()).append("\\1_aligned").toStdString());
	fs::create_directory(QString(ss.str().c_str()).append("\\2_cutline").toStdString());
	fs::create_directory(QString(ss.str().c_str()).append("\\3_morphed").toStdString());

	// setup entries for morphing
	ss.str(""); ss << "Resources\\Fingerprints\\" << DemoApp::sensors[DemoApp::currentSensor].toStdString();
	const auto path = ss.str();
	auto fingerprints = fs::directory_iterator(path);
	std::vector<fs::directory_entry> entries;
	for (auto& entry : fingerprints)
	{
		entries.emplace_back(entry);
	}

	// setup processors
	const auto blockSize = 12;
	const auto windowSize = 30;
	const auto trashHold_segmentation = 0.003;
	processing::storage::Fingerprint f(cv::Mat::zeros(10, 10, CV_32F));
	f.setBlockSize(blockSize).setWindowSize(windowSize);

	auto processor = setUpFingerprintProcessor(f);
	auto morpher = setUpMorphingProcessor(f, processor, false, false);
	ss.str(""); ss << "Results\\morphing\\static\\" << DemoApp::sensors[DemoApp::currentSensor].toStdString() << "\\";
	morphing::MorphingProcessor::path = ss.str();

	// clear output statistics file
	ss.str(""); ss << morphing::MorphingProcessor::path << "match_results.txt";
	std::ofstream file(ss.str(), std::ofstream::out | std::ofstream::trunc);
	if (file.is_open())
	{
		file.close();
	}
	else { std::cerr << "Failed to clean the statistics file!" << std::endl; exit(-1); }

	morpher.setTemplateBackground(255);

	runMorphing(morpher, entries, blockSize, windowSize, trashHold_segmentation);
}

void TestCase::runDynamicMorphing()
{
	fs::create_directory("Results");
	fs::create_directory("Results\\morphing");
	fs::create_directory("Results\\morphing\\dynamic");
	
	std::stringstream ss; ss << "Results\\morphing\\dynamic\\" << DemoApp::sensors[DemoApp::currentSensor].toStdString();
	fs::create_directory(ss.str().c_str());
	fs::create_directory(QString(ss.str().c_str()).append("\\1_aligned").toStdString());
	fs::create_directory(QString(ss.str().c_str()).append("\\2_cutline").toStdString());
	fs::create_directory(QString(ss.str().c_str()).append("\\3_morphed").toStdString());
	
	// setup entries for morphing
	ss.str(""); ss << "Resources\\Fingerprints\\" << DemoApp::sensors[DemoApp::currentSensor].toStdString();
	const auto path = ss.str();
	auto fingerprints = fs::directory_iterator(path);
	std::vector<fs::directory_entry> entries;
	for (auto& entry : fingerprints)
	{
		entries.emplace_back(entry);
	}

	// setup processors
	const auto blockSize = 12;
	const auto windowSize = 30;
	const auto trashHold_segmentation = 0.003;
	processing::storage::Fingerprint f(cv::Mat::zeros(10, 10, CV_32F));
	f.setBlockSize(blockSize).setWindowSize(windowSize);
	
	auto processor = setUpFingerprintProcessor(f);
	auto morpher = setUpMorphingProcessor(f, processor, true, false);
	ss.str(""); ss << "Results\\morphing\\dynamic\\" << DemoApp::sensors[DemoApp::currentSensor].toStdString() << "\\";
	morphing::MorphingProcessor::path = ss.str();

	// clear output statistics file
	ss.str(""); ss << morphing::MorphingProcessor::path << "match_results.txt";
	std::ofstream file(ss.str(), std::ofstream::out | std::ofstream::trunc);
	if (file.is_open())
	{
		file.close();
	}
	else { std::cerr << "Failed to clean the statistics file!" << std::endl; exit(-1); }

	morpher.setTemplateBackground(255);
	
	runMorphing(morpher, entries, blockSize, windowSize, trashHold_segmentation);
}

void TestCase::runProcessing(const std::string& input, const std::string& output)
{
	const auto blockSize = 12;
	const auto windowSize = 30;
	const auto trashHold_segmentation = 0.003;

	processing::storage::Fingerprint f(cv::Mat::zeros(10, 10, CV_32F));
	f.setBlockSize(blockSize).setWindowSize(windowSize);

	auto processor = setUpFingerprintProcessor(f);

	auto doJob = true;

	const auto path = input;
	processing::FingerprintProcessor::path = output;
	for (const auto& entry : fs::directory_iterator(path))
	{
		const auto filename = entry.path().stem().string();

		if (doJob)
		{
			processing::storage::Fingerprint fingerprint;
			try
			{
				const auto img = processing::utils::ImageProcessor::read(entry.path().string());

				fingerprint = processing::FingerprintProcessor::getFingerprint(img);
			}
			catch (std::exception& e)
			{
				std::cout << e.what() << std::endl;
				exit(-1);
			}

			processing::FingerprintProcessor::adapt(fingerprint, blockSize, windowSize, trashHold_segmentation);
			processor.normalize(fingerprint);
			processor.estimateOrientations(fingerprint);
			processor.estimateFrequencies(fingerprint);
			processor.applyRegionMask(fingerprint);
			processor.filterFingerprint(fingerprint);
			processor.findMinutiaesW(fingerprint);

			processor.writeNormalized(fingerprint, filename);
			processor.writeOrientations(fingerprint, filename);
			processor.writeEnhanced(fingerprint, filename);
			processor.writeMinutiaes(fingerprint, filename);
		}

		if (!doJob && filename == "105")
		{
			doJob = true;
		}
	}
}

void TestCase::runMorphing(morphing::MorphingProcessor morpher, std::vector<fs::directory_entry> entries
                           , int blockSize, int windowSize, float trashHoldSegmentation)
{
	// start test morphing
	for (auto entry = entries.begin(); entry != entries.end(); ++entry)
	{
		const auto fingerName1 = entry->path().string();
		const auto file1 = entry->path().stem().string();

		auto i = 1;
		auto other = entry;
		for (++other; i < 4 && other != entries.end(); ++i, ++other)
		{
			const auto fingerName2 = other->path().string();
			const auto file2 = other->path().stem().string();

			std::stringstream ss;
			processing::storage::Fingerprint fingerprint;
			processing::storage::Fingerprint tmp;
			try
			{
				const auto img1 = processing::utils::ImageProcessor::read(fingerName1);
				const auto img2 = processing::utils::ImageProcessor::read(fingerName2);

				fingerprint = processing::storage::Fingerprint(img1);
				tmp = processing::storage::Fingerprint(img2);
			}
			catch (std::exception& e)
			{
				std::cout << e.what() << std::endl;
				exit(-1);
			}

			morphing::storage::AlignedFingerprint aFingerprint;
			*static_cast<cv::Mat*>(&aFingerprint) = tmp.get();
			
			processing::FingerprintProcessor::adapt(fingerprint, blockSize, windowSize, trashHoldSegmentation);
			processing::FingerprintProcessor::adapt(aFingerprint, blockSize, windowSize, trashHoldSegmentation);

			const auto morphedFingerprint = morpher.morph(aFingerprint, fingerprint);

			ss.str(""); ss << file1 << "_" << file2;
			const auto morphedFilename = ss.str();

			morpher.writeAligned(aFingerprint, fingerprint, morphedFilename);
			morpher.writeCutline(aFingerprint, fingerprint, morphedFilename);
			morpher.writeMorphed(morphedFingerprint, morphedFilename);

			fs::create_directories("cache");
			const auto tpl1 = "cache\\1.txt"; matching::Matcher::createTxtMinutiaTemplate(fingerprint, tpl1);
			const auto tpl2 = "cache\\2.txt"; matching::Matcher::createTxtMinutiaTemplate(aFingerprint, tpl2);
			const auto tpl3 = "cache\\3.txt"; matching::Matcher::createTxtMinutiaTemplate(morphedFingerprint, tpl3);

			ss.str(""); ss << morphing::MorphingProcessor::path << "match_results.txt";
			std::ofstream file(ss.str(), std::ios_base::app);
			if (file.is_open())
			{
				file << entry->path().stem().string() << ";"
					<< morphedFilename << ";"
					<< this->matcher->match(tpl1, tpl3) << ";"
					<< std::endl;
				file << other->path().stem().string() << ";"
					<< morphedFilename << ";"
					<< this->matcher->match(tpl2, tpl3) << ";"
					<< std::endl;
				file.close();
			}
		}
	}
}

processing::FingerprintProcessor TestCase::setUpFingerprintProcessor(const processing::storage::Fingerprint& f)
{
	auto deviation = 4;

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
		.setDeviation(deviation);

	processing::utils::MinutiaeEstimator minutiaes;
	minutiaes
		.setBlockSize(f.getBlockSize());

	const processing::utils::FakeMinutiaeDetector detector;

	return processing::FingerprintProcessor(orientations, frequencies, filter, minutiaes, detector);
}

morphing::MorphingProcessor TestCase::setUpMorphingProcessor(const processing::storage::Fingerprint& f, processing::FingerprintProcessor& processor, bool dynamic, bool adaptive)
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
		.showMinutiaes(true)
		.useAdaptiveMethod(adaptive);

	if (dynamic)
	{
		cutline.useDynamicCutline(true);
		cutline.setArea(64);
	}

	morphing::utils::TemplateGenerator generator;
	generator
		.setBorder(16);

	return morphing::MorphingProcessor(processor, aligner, cutline, generator);
}

void TestCase::showMatchingStats()
{
	const auto menuButton = static_cast<QAction*>(sender());
	const auto resultsType = menuButton->objectName();

	std::string path;
	std::stringstream ss;
	switch (this->results[resultsType])
	{
		case ResultsTypes::STATIC_SYNTHETIC:
			path = "Results\\morphing\\static\\synthetic\\match_results.txt";
			ss << "=================================================" << "<br>"
				<< "Results for " << path << ":" << "<br>"
				<< "=================================================";

			emit reportProgress(ss.str().c_str());
		
			showMatchingStatsForResults(path.c_str());
			showFARStats(path.c_str());
			showDARStats(path.c_str());
		
			emit reportProgress("=================================================");
			break;
		case ResultsTypes::DYNAMIC_SYNTHETIC:
			path = "Results\\morphing\\dynamic\\synthetic\\match_results.txt";
			ss << "=================================================" << "<br>"
				<< "Results for " << path << ":" << "<br>"
				<< "=================================================";

			emit reportProgress(ss.str().c_str());
		
			showMatchingStatsForResults(path.c_str());
			showFARStats(path.c_str());
			showDARStats(path.c_str());

			emit reportProgress("=================================================");
			break;
		case ResultsTypes::ADAPTIVE_STATIC_SYNTHETIC:
			path = "Results\\morphing\\adaptive_static\\synthetic\\match_results.txt";
			ss << "=================================================" << "<br>"
				<< "Results for " << path << ":" << "<br>"
				<< "=================================================";

			emit reportProgress(ss.str().c_str());

			showMatchingStatsForResults(path.c_str());
			showFARStats(path.c_str());
			showDARStats(path.c_str());

			emit reportProgress("=================================================");
			break;
		case ResultsTypes::ADAPTIVE_DYNAMIC_SYNTHETIC:
			path = "Results\\morphing\\adaptive_dynamic\\synthetic\\match_results.txt";
			ss << "=================================================" << "<br>"
				<< "Results for " << path << ":" << "<br>"
				<< "=================================================";

			emit reportProgress(ss.str().c_str());

			showMatchingStatsForResults(path.c_str());
			showFARStats(path.c_str());
			showDARStats(path.c_str());

			emit reportProgress("=================================================");
			break;
		default:
			break;
	}
}

void TestCase::showMatchingStatsForResults(QString path)
{
	std::map<std::string, int> stats = {
		{"<.05", 0},
		{".05<.1", 0},
		{".1<.15", 0},
		{".15<.2", 0},
		{".2<.25", 0},
		{".25<.3", 0},
		{".3<.35", 0},
		{".35<.4", 0},
		{".4<.45", 0},
		{".45<.5", 0},
		{".5<", 0},
	};

	std::stringstream ss; ss << path.toStdString();
	std::ifstream file(ss.str());

	auto count = .0;
	std::string line;
	while (std::getline(file, line)) {

		std::vector<std::string> result;
		std::istringstream iss(line);

		for (std::string token; std::getline(iss, token, ';'); )
		{
			result.push_back(std::move(token));
		}

		auto score = std::stod(result.at(2));

		if (score < .05)
		{
			stats["<.05"]++;
		}
		else if (score >= .05 && score < .1)
		{
			stats[".05<.1"]++;
		}
		else if (score >= .1 && score < .15)
		{
			stats[".1<.15"]++;
		}
		else if (score >= .15 && score < .2)
		{
			stats[".15<.2"]++;
		}
		else if (score >= .2 && score < .25)
		{
			stats[".2<.25"]++;
		}
		else if (score >= .25 && score < .3)
		{
			stats[".25<.3"]++;
		}
		else if (score >= .3 && score < .35)
		{
			stats[".3<.35"]++;
		}
		else if (score >= .35 && score < .4)
		{
			stats[".35<.4"]++;
		}
		else if (score >= .4 && score < .45)
		{
			stats[".4<.45"]++;
		}
		else if (score >= .45 && score < .5)
		{
			stats[".45<.5"]++;
		}
		else
		{
			stats[".5<"]++;
		}

		count += 1;
	}

	
	ss.str(""); ss << "&lt;0, 0.05): " << (count == .0 ? 0 : stats["<.05"] / count) * 100 << "%<br>"
		<< "&lt;0.05, 0.1): " << (count == .0 ? 0 : stats[".05<.1"] / count) * 100 << "%<br>"
		<< "&lt;0.1, 0.15): " << (count == .0 ? 0 : stats[".1<.15"] / count) * 100 << "%<br>"
		<< "&lt;0.15, 0.2): " << (count == .0 ? 0 : stats[".15<.2"] / count) * 100 << "%<br>"
		<< "&lt;0.2, 0.25): " << (count == .0 ? 0 : stats[".2<.25"] / count) * 100 << "%<br>"
		<< "&lt;0.25, 0.3): " << (count == .0 ? 0 : stats[".25<.3"] / count) * 100 << "%<br>"
		<< "&lt;0.3, 0.35): " << (count == .0 ? 0 : stats[".3<.35"] / count) * 100 << "%<br>"
		<< "&lt;0.35, 0.4): " << (count == .0 ? 0 : stats[".35<.4"] / count) * 100 << "%<br>"
		<< "&lt;0.4, 0.45): " << (count == .0 ? 0 : stats[".4<.45"] / count) * 100 << "%<br>"
		<< "&lt;0.45, 0.5): " << (count == .0 ? 0 : stats[".45<.5"] / count) * 100 << "%<br>"
		<< "&lt;0.5, 1&gt;: " << (count == .0 ? 0 : stats[".5<"] / count) * 100 << "%<br>"
		<< "=================================================";

	emit reportProgress(ss.str().c_str());
}

void TestCase::showFARStats(QString path)
{
	std::map<std::string, int> stats = {
		{"1", 0},
		{".1", 0},
		{".01", 0}
	};

	std::stringstream ss; ss << path.toStdString();
	std::ifstream file(ss.str());

	auto count = .0;
	std::string line;
	while (std::getline(file, line)) {

		std::vector<std::string> result;
		std::istringstream iss(line);

		for (std::string token; std::getline(iss, token, ';'); )
		{
			result.push_back(std::move(token));
		}

		auto score = std::stod(result.at(2));

		if (score >= .1083)
		{
			stats["1"]++;
		}

		if (score >= .1205)
		{
			stats[".1"]++;
		}

		if (score >= .1329)
		{
			stats[".01"]++;
		}

		count += 1;
	}

	ss.str(""); ss << "FAR(1): " << (count == .0 ? 0 : stats["1"] / count) * 100 << "%<br>"
		<< "FAR(0,1): " << (count == .0 ? 0 : stats[".1"] / count) * 100 << "%<br>"
		<< "FAR(0,01): " << (count == .0 ? 0 : stats[".01"] / count) * 100 << "%<br>"
		<< "=================================================";

	emit reportProgress(ss.str().c_str());
}

void TestCase::showDARStats(QString path)
{
	std::map<std::string, int> stats = {
		{"1", 0},
		{".1", 0},
		{".01", 0}
	};

	std::stringstream ss; ss << path.toStdString();
	std::ifstream file(ss.str());

	auto count = .0;
	std::string line1;
	std::string line2;
	while (true) {

		if (!std::getline(file, line1)
			|| !std::getline(file, line2))
		{
			break;
		}
		
		std::vector<std::string> result1;
		std::vector<std::string> result2;
		std::istringstream iss1(line1);
		std::istringstream iss2(line2);

		for (std::string token; std::getline(iss1, token, ';'); )
		{
			result1.push_back(std::move(token));
		}
		for (std::string token; std::getline(iss2, token, ';'); )
		{
			result2.push_back(std::move(token));
		}
		
		auto score1 = std::stod(result1.at(2));
		auto score2 = std::stod(result2.at(2));

		if (score1 >= .1083 && score2 >= .1083)
		{
			stats["1"]++;
		}

		if (score1 >= .1205 && score2 >= .1205)
		{
			stats[".1"]++;
		}

		if (score1 >= .1329 && score2 >= .1329)
		{
			stats[".01"]++;
		}

		count += 1;
	}

	ss.str(""); ss << "DAR(1): " << (count == .0 ? 0 : stats["1"] / count) * 100 << "%<br>"
		<< "DAR(0,1): " << (count == .0 ? 0 : stats[".1"] / count) * 100 << "%<br>"
		<< "DAR(0,01): " << (count == .0 ? 0 : stats[".01"] / count) * 100 << "%<br>"
		<< "=================================================";

	emit reportProgress(ss.str().c_str());
}
