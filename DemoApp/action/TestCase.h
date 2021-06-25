#pragma once

#include <storage/Fingerprint.h>
#include <storage/AlignedFingerprint.h>
#include <MorphingProcessor.h>
#include <Matcher.h>

#include <QObject>

#include <filesystem>

namespace fs = std::filesystem;

namespace app
{
	namespace action
	{
		class TestCase : public QObject
		{
			Q_OBJECT

		private:
			matching::Matcher* matcher;

			enum ResultsTypes { STATIC_SYNTHETIC, DYNAMIC_SYNTHETIC, ADAPTIVE_STATIC_SYNTHETIC, ADAPTIVE_DYNAMIC_SYNTHETIC };
			std::map<QString, int> results = {
				{ "testingResultsStatic", ResultsTypes::STATIC_SYNTHETIC },
				{ "testingResultsDynamic", ResultsTypes::DYNAMIC_SYNTHETIC },
				{ "testingResultsAdaptiveStatic", ResultsTypes::ADAPTIVE_STATIC_SYNTHETIC },
				{ "testingResultsAdaptiveDynamic", ResultsTypes::ADAPTIVE_DYNAMIC_SYNTHETIC }
			};
			
		private slots:
			static processing::FingerprintProcessor setUpFingerprintProcessor(const processing::storage::Fingerprint& f);
			static morphing::MorphingProcessor setUpMorphingProcessor(const processing::storage::Fingerprint& f, 
				processing::FingerprintProcessor& processor, bool dynamic, bool adaptive);
			void showMatchingStatsForResults(QString path);
			void showFARStats(QString path);
			void showDARStats(QString path);

			void runProcessing(const std::string& input, const std::string& output);
			void runMorphing(morphing::MorphingProcessor morpher, std::vector<fs::directory_entry> entries,
				int blockSize, int windowSize, float trashHoldSegmentation);
			
		public:
			// constructors
			TestCase(QObject* parent);

			// destructors
			~TestCase();

		signals:
			void reportProgress(QString);

		public slots:
			void runProcessing();
			void runStaticMorphing();
			void runDynamicMorphing();
			void runAdaptiveStaticMorphing();
			void runAdaptiveDynamicMorphing();

			void showMatchingStats();
			
		};
	}
}
