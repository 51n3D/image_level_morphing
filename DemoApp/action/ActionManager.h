#pragma once

#include <storage/Fingerprint.h>
#include <storage/AlignedFingerprint.h>
#include <MorphingProcessor.h>
#include <Matcher.h>

#include <QObject>

namespace app
{
	namespace action
	{
		class ActionManager : public QObject
		{
			Q_OBJECT

		private:
			// members
			/* Nastroje pre pracu s odtlackami */
			processing::storage::Fingerprint fingerprint;
			morphing::storage::AlignedFingerprint alignedFingerprint;
			processing::FingerprintProcessor* processor;
			morphing::MorphingProcessor* morpher;

			/* Porovnavac */
			matching::Matcher* matcher;

			processing::storage::Fingerprint morphedFingerprint;

			// methods
			processing::FingerprintProcessor* setUpFingerprintProcessor(const processing::storage::Fingerprint& f);
			morphing::MorphingProcessor* setUpMorpher(processing::FingerprintProcessor& processor, processing::storage::Fingerprint f);

		private slots:
			void showFingerprintStepsDialog(const processing::storage::Fingerprint& fingerprint, const QString& dialogName);
			void showAlignedFingerprintStepsDialog(const morphing::storage::AlignedFingerprint& fingerprint, const QString& dialogName);
			
		public:
			ActionManager(QObject* parent);
			~ActionManager();

		signals:
			void reportProgress(QString);
			void triggered();
			void morphingGettingReady();
			void morphingDone(const cv::Mat&);
			
		public slots:
			void startMorphing();
			void morph();
			void loadFingerprint(QString filename);
			void loadAlignedFingerprint(QString filename);
			void displayMorphingSteps();
			void displayFingerprint1Steps();
			void displayFingerprint2Steps();

			void setAlignmentThreshold(int threshold);
			void setCutlineWeights(double oW, double vW, double mW);
			void setCutlineIntersectionAreaWidth(int dMax);
			void setCutlineRotationStep(int rStep);
			void setUseDynamicCutline(bool useDynamicCutline);
			void setUseAdaptiveMethod(bool useAdaptiveMethod);
			void setDynamicCutlineSizeOfSearchArea(int dynamicSearchSizeArea);
			void setGeneratorBorderWidth(int borderWidth);
			void setBackgroundColor(int backgroundColor);

			void matchFingerprintsWithMorphed();
		};
	}
}
