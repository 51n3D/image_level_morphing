#include "MorphingProcessor.h"
#include "storage/AlignedFingerprint.h"

#include <utils/ImageProcessor.h>

using namespace processing::utils::storage;
using namespace processing::storage;
using namespace processing;
using namespace morphing::storage;
using namespace morphing::utils;
using namespace morphing;
using namespace cv;

std::string MorphingProcessor::path = "C:\\Users\\Denis\\Desktop\\Results\\morphing\\synthetic\\";

MorphingProcessor::MorphingProcessor(FingerprintProcessor& processor, FingerprintAligner& aligner, CutlineEstimator& cutline, TemplateGenerator& generator)
	: processor(processor), aligner(aligner), cutline(cutline), generator(generator) {}

Fingerprint MorphingProcessor::morph(AlignedFingerprint& af, Fingerprint& f)
{
	this->processor.normalize(f);
	this->processor.normalize(af);
	
	this->processor.estimateOrientations(f);
	this->processor.estimateOrientations(af);

	this->processor.estimateFrequencies(f);
	this->processor.estimateFrequencies(af);
	
	this->processor.applyRegionMask(f);
	this->processor.applyRegionMask(af);
	
	this->aligner.align(af, f);

	this->processor.findMinutiaes(af);
	this->processor.findMinutiaes(f);
	
	const auto line = this->cutline.estimate(af, f);
	af.setCutline(line);

	const auto morphed = this->generator.generate(af, f, TemplateGenerator::Type::IMAGE_LEVEL);
	
	return morphed;
}

void MorphingProcessor::writeAligned(const AlignedFingerprint& af, const Fingerprint& f, const std::string& filename) const
{
	std::stringstream ss; ss.str(""); ss << path << "1_aligned\\" << filename;
	this->aligner.write(af, f, ss.str());
}

void MorphingProcessor::writeCutline(const AlignedFingerprint& af, const Fingerprint& f, const std::string& filename)
{
	std::stringstream ss; ss.str(""); ss << path << "2_cutline\\" << filename;
	this->cutline.write(af, f, ss.str());
}

void MorphingProcessor::writeMorphed(const Fingerprint& f, const std::string& filename) const
{
	std::stringstream ss; ss.str(""); ss << path << "3_morphed\\" << filename;
	this->generator.write(f, ss.str());
}

AlignedFingerprint MorphingProcessor::getFingerprint(const Mat& fingerprintImg)
{
	return AlignedFingerprint(fingerprintImg);
}
