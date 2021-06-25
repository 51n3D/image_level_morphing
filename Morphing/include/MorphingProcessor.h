#pragma once

#include "utils/FingerprintAligner.h"
#include "utils/CutlineEstimator.h"
#include "utils/TemplateGenerator.h"

#include <FingerprintProcessor.h>
#include <storage/Fingerprint.h>

#include <opencv2/opencv.hpp>

namespace morphing
{
	namespace storage
	{
		class AlignedFingerprint;
	}
	
	/**
	 * \brief Top-level ovladac. Trieda je urcenia na zabuzdrenie cinnosti
	 * jednotlivych nastrajov a pritup k datam objektu AlignedFingerprint. 
	 */
	class MorphingProcessor
	{
	private:
		// members
		/**
		 * \brief Ovladac ziskavania informacii z odtlacku.
		 */
		processing::FingerprintProcessor processor;
		/**
		 * \brief Nastroj na zarovnanie dvoch odtlackov.
		 */
		utils::FingerprintAligner aligner;
		/**
		 * \brief Nastroj na odhad optimalnej reznej linie zarovnanych odtlackov.
		 */
		utils::CutlineEstimator cutline;
		/**
		 * \brief Nastroj na vygenerovanie morfovaneho odtlacku.
		 */
		utils::TemplateGenerator generator;

	public:
		// members
		static std::string path;
		
		// constructors
		MorphingProcessor(processing::FingerprintProcessor& processor, utils::FingerprintAligner& aligner, utils::CutlineEstimator& cutline, utils::TemplateGenerator& generator);

		// methods
		processing::storage::Fingerprint morph(storage::AlignedFingerprint& af, processing::storage::Fingerprint& f);

		/* Metody pre zapis demo obrazkov do suboru. */
		void writeAligned(const storage::AlignedFingerprint& af, const processing::storage::Fingerprint& f, const std::string& filename) const;
		void writeCutline(const storage::AlignedFingerprint& af, const processing::storage::Fingerprint& f, const std::string& filename);
		void writeMorphed(const processing::storage::Fingerprint& f, const std::string& filename) const;

		// static methods
		static storage::AlignedFingerprint getFingerprint(const cv::Mat& fingerprintImg);
		
		// getters
		cv::Mat getAlignedImage(const storage::AlignedFingerprint& af, const processing::storage::Fingerprint& f) const
			{ return this->aligner.getAlignedFingersImage(af, f); }
		cv::Mat getCutlineImage(const storage::AlignedFingerprint& af, const processing::storage::Fingerprint& f)
			{ return this->cutline.getCutlineImage(af, f); }
		processing::storage::Fingerprint getMorphedImage(const storage::AlignedFingerprint& af, const processing::storage::Fingerprint& f)
			{ return this->generator.generate(af, f, utils::TemplateGenerator::Type::IMAGE_LEVEL_COLORED); }
		
		// setters
		MorphingProcessor& setMinimalAlignment(const float threshold) { this->aligner.setTrashHold(threshold); return *this; }
		MorphingProcessor& setCutlineWeights(const float oW, const float vW, const float mW) { this->cutline.setWeights(oW, vW, mW); return *this; }
		MorphingProcessor& useDynamicCutline(const bool dynamicCutline = true) { this->cutline.useDynamicCutline(dynamicCutline); return *this; }
		MorphingProcessor& useAdaptiveMethod(const bool adaptiveMethod = true) { this->cutline.useAdaptiveMethod(adaptiveMethod); return *this; }
		MorphingProcessor& setDynamicCutlineSearchArea(const int dynamicSearchSizeArea) { this->cutline.setArea(dynamicSearchSizeArea); return *this; }
		MorphingProcessor& setCutlineRotations(const float rotationStep) { this->cutline.setRotationStep(rotationStep); return *this; }
		MorphingProcessor& setCutlineDMax(const int dmax) { this->cutline.setMaxDistance(dmax); return *this; }
		MorphingProcessor& setTemplateBackground(const int color) { this->generator.setBackgroundColor(static_cast<float>(color)); return *this; }
		MorphingProcessor& setBorderWidth(const int border) { this->generator.setBorder(border); return *this; }
	};
}
