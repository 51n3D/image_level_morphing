#include "FingerprintProcessor.h"
#include "utils/GaborFilter.h"
#include "storage/Fingerprint.h"
#include "utils/ImageProcessor.h"

using namespace processing::utils::storage;
using namespace processing::storage;
using namespace processing::utils;
using namespace processing;
using namespace cv;

std::string FingerprintProcessor::path = "C:\\Users\\Denis\\Desktop\\Results\\processing\\synthetic\\";

int FingerprintProcessor::displayed = 0;
int FingerprintProcessor::displayedN = 0;
int FingerprintProcessor::displayedE = 0;

void FingerprintProcessor::display(const Mat& img, const std::string& trace) const
{
	std::stringstream ss; ss << ++displayed << ": Fingerprint" << " TRACE: " << trace;
	imshow(ss.str(), img);
}

void FingerprintProcessor::correctOrientations(Fingerprint& fingerprint)
{
	auto o = fingerprint.getOrientations();
	auto r = fingerprint.getRegionMask();

	for (auto i = 0; i < r.rows; i++)
	{
		for (auto j = 0; j < r.cols; j++)
		{
			o.at<Vec2f>(i, j)[1] = r.at<float>(i, j);
		}
	}

	fingerprint.setOrientations(o);
}

FingerprintProcessor::FingerprintProcessor(const OrientationsEstimator& orientations, const FrequenciesEstimator& frequencies,
                                           const GaborFilter& filter, const MinutiaeEstimator& minutiaes, const utils::FakeMinutiaeDetector& detector)
	: orientations(orientations), frequencies(frequencies), filter(filter), minutiaes(minutiaes), detector(detector) {}

Fingerprint FingerprintProcessor::getFingerprint(const Mat& fingerprintImg)
{
	return Fingerprint(fingerprintImg);
}

void FingerprintProcessor::estimateOrientations(Fingerprint& fingerprint, const bool verbose)
{
	const auto o = this->orientations.estimate(fingerprint.getNormalized());
	fingerprint.setOrientations(o);

	if (verbose)
	{
		this->displayOrientations(fingerprint, "estimateOrientations");
	}
}

void FingerprintProcessor::applyRegionMask(Fingerprint& fingerprint, bool verbose)
{
	auto regionMask = fingerprint.getRegionMask();
	const auto region = regionMask.getRegion();
	
	ImageProcessor::trim(fingerprint, region);

	auto n = fingerprint.getNormalized();
	ImageProcessor::trim(n, region);
	fingerprint.setNormalized(n);
	
	auto o = fingerprint.getOrientations();
	ImageProcessor::trim(o, region);
	fingerprint.setOrientations(o);

	this->correctOrientations(fingerprint);
	
	if (verbose)
	{
		Mat tmp;
		fingerprint.getNormalized().copyTo(tmp);
		for (auto i = 0; i < fingerprint.rows; i++)
		{
			for (auto j = 0; j < fingerprint.cols; j++)
			{
				if (regionMask.at<float>(i, j) == 0)
				{
					tmp.at<float>(i, j) = 0;
				}
			}
		}

		imshow("RegionMask", tmp);
	}
}

void FingerprintProcessor::displayNormalized(const Fingerprint& fingerprint, const std::string& trace) const
{
	Mat tmp;
	fingerprint.getNormalized().copyTo(tmp);
	for (auto i = 0; i < fingerprint.getSegmentation().rows; i++)
	{
		for (auto j = 0; j < fingerprint.getSegmentation().cols; j++)
		{
			if (fingerprint.getSegmentation().at<float>(i, j) == 0)
			{
				tmp.at<float>(i, j) = 0;
			}
		}
	}

	this->display(tmp, trace);
}

void FingerprintProcessor::displayOrientations(const Fingerprint& fingerprint, const std::string& trace, const bool blank) const
{
	this->orientations.display(fingerprint.getNormalized(), fingerprint.getOrientations(), trace, blank);
}

void FingerprintProcessor::estimateFrequencies(Fingerprint& fingerprint, const bool verbose)
{
	const auto o = fingerprint.getOrientations();
	this->frequencies.setOrientations(o);

	const auto v = this->frequencies.estimate(fingerprint.getNormalized());
	fingerprint
		.setFrequencies(v)
		.setMaxF(this->frequencies.getMaxFrequency())
		.setMinF(this->frequencies.getMinFrequency())
		.setRegionMask(this->frequencies.getRegionMask());

	if (verbose)
	{
		this->displayFrequencies(fingerprint, "estimateFrequencies");;
	}
}

void FingerprintProcessor::findMinutiaes(Fingerprint& fingerprint, const bool verbose)
{
	this->filterFingerprint(fingerprint);
	binarize(fingerprint);
	thinning(fingerprint);
	this->estimateMinutiaes(fingerprint);

	this->handleFakeMinutiaes(fingerprint);
	
	if (verbose)
	{
		const std::string trace = "findMinutiaes";
		
		this->displayFiltered(fingerprint, trace);
		this->displayBinarized(fingerprint, trace);
		this->displayThinned(fingerprint, trace);
		this->displayMinutiaes(fingerprint, trace);
	}
}

void FingerprintProcessor::handleFakeMinutiaes(Fingerprint& fingerprint) const
{
	auto minutiaes = fingerprint.getMinutiae();
	
	this->detector.find(fingerprint.getThinned(), minutiaes);
	this->detector.remove(minutiaes);
	
	fingerprint.setMinutiae(minutiaes);
}

void FingerprintProcessor::findMinutiaesW(Fingerprint& fingerprint, const bool verbose)
{
	thinning(fingerprint);
	this->estimateMinutiaes(fingerprint);

	this->handleFakeMinutiaes(fingerprint);

	if (verbose)
	{
		this->displayMinutiaes(fingerprint, "findMinutiaes");
	}
}

void FingerprintProcessor::displayFrequencies(const Fingerprint& fingerprint, const std::string& trace) const
{
	this->frequencies.display(fingerprint.getFrequencies(), trace);
}

void FingerprintProcessor::filterFingerprint(Fingerprint& fingerprint, const bool verbose)
{
	const auto o = fingerprint.getOrientations();
	const auto v = fingerprint.getFrequencies();
	const auto r = fingerprint.getRegionMask();
	const auto b = r.idealGaborBlock();

	this->filter
		.setOrientationMap(o)
		.setFrequencyMap(v)
		.setBlockSize(b);

	const auto filtered = this->filter.filter(fingerprint.getNormalized());
	fingerprint.setFiltered(filtered);

	binarize(fingerprint);

	if (verbose)
	{
		this->displayBinarized(fingerprint, "filterFingerprint");
	}
}

void FingerprintProcessor::displayFiltered(const Fingerprint& fingerprint, const std::string& trace) const
{
	this->filter.display(fingerprint.getFiltered(), trace);
}

void FingerprintProcessor::binarize(Fingerprint& fingerprint)
{
	Mat binarized;
	fingerprint.getFiltered().copyTo(binarized);
	const auto segmentation = fingerprint.getOrientations();

	ImageProcessor::binarize(binarized, segmentation);

	fingerprint.setBinarized(binarized);
}

void FingerprintProcessor::displayBinarized(const Fingerprint& fingerprint, const std::string& trace) const
{
	const auto binarized = fingerprint.getBinarized();
	
	this->display(binarized, trace);
}

void FingerprintProcessor::thinning(Fingerprint& fingerprint)
{
	Mat thinned;
	fingerprint.getBinarized().copyTo(thinned);
	const auto segmentation = fingerprint.getSegmentation();

	ImageProcessor::thine(thinned, segmentation);

	fingerprint.setThinned(thinned);
}

void FingerprintProcessor::estimateMinutiaes(Fingerprint& fingerprint, const bool verbose)
{
	const auto thinned = fingerprint.getThinned();

	this->minutiaes
		.setBlockSize(fingerprint.getBlockSize())
		.setSegmentation(fingerprint.getOrientations());
	
	const auto minutiaes = this->minutiaes.estimate(thinned);
	fingerprint
		.setMinutiae(minutiaes)
		.setMinutiaeTracking(this->minutiaes.geMinutiaeTracing());
}

Mat FingerprintProcessor::getOrientationsImage(const Fingerprint& fingerprint)
{
	return this->orientations.getOrientationImage(fingerprint.getNormalized(), fingerprint.getOrientations());
}

Mat FingerprintProcessor::getFrequenciesImage(const Fingerprint& fingerprint)
{
	return fingerprint.getFrequencies();
}

Mat FingerprintProcessor::getEnhancedImage(const Fingerprint& fingerprint)
{
	return fingerprint.getBinarized();
}

Mat FingerprintProcessor::getThinnedImage(const Fingerprint& fingerprint)
{
	return fingerprint.getThinned();
}

Mat FingerprintProcessor::getMinutiaeTracing(const Fingerprint& fingerprint)
{
	return fingerprint.getMinutiaeTracing();
}

Mat FingerprintProcessor::getMinutiaeImage(const Fingerprint& fingerprint)
{
	return this->minutiaes.getMinutiaeImage(fingerprint, fingerprint.getMinutiae());
}

Mat FingerprintProcessor::getMinutiaeImage(const Mat& fingerprint, const std::vector<Minutiae>& minutiae)
{
	return this->minutiaes.getMinutiaeImage(fingerprint, minutiae);
}

void FingerprintProcessor::displayThinned(const Fingerprint& fingerprint, const std::string& trace) const
{
	const auto thinned = fingerprint.getThinned();
	
	this->display(thinned, trace);
}

void FingerprintProcessor::displayMinutiaes(const Fingerprint& fingerprint, const std::string& trace) const
{
	const auto minutiaes = fingerprint.getMinutiae();
	
	this->minutiaes.display(fingerprint, minutiaes, trace);
}

void FingerprintProcessor::writeNormalized(const Fingerprint& fingerprint, const std::string& filename) const
{
	Mat tmp;
	auto segmentation = fingerprint.getSegmentation();
	fingerprint.getNormalized().copyTo(tmp);
	for (auto i = 0; i < segmentation.rows; i++)
	{
		for (auto j = 0; j < segmentation.cols; j++)
		{
			if (segmentation.at<float>(i, j) == 0)
			{
				tmp.at<float>(i, j) = 0;
			}
		}
	}

	std::stringstream ss; ss.str(""); ss << path << "1_normalized\\" << filename;
	write(tmp, ss.str());
}

void FingerprintProcessor::writeOrientations(const Fingerprint& fingerprint, const std::string& filename) const
{
	std::stringstream ss; ss.str(""); ss << path << "2_orientations\\" << filename;
	this->orientations.write(fingerprint.getNormalized(), fingerprint.getOrientations(), ss.str());
}

void FingerprintProcessor::writeFrequencies(const Fingerprint& fingerprint, const std::string& filename) const
{
	std::stringstream ss; ss.str(""); ss << path << "frequencies\\" << filename;
	this->frequencies.write(fingerprint.getFrequencies(), ss.str());
}

void FingerprintProcessor::writeEnhanced(const Fingerprint& fingerprint, const std::string& filename) const
{
	std::stringstream ss; ss.str(""); ss << path << "3_enhanced\\" << filename;
	write(fingerprint.getBinarized(), ss.str());
}

void FingerprintProcessor::writeThinning(const Fingerprint& fingerprint, const std::string& filename) const
{
	std::stringstream ss; ss.str(""); ss << path << "5_thinning\\" << filename;
	write(fingerprint.getThinned(), ss.str());
}

void FingerprintProcessor::writeMinutiaes(const Fingerprint& fingerprint, const std::string& filename) const
{
	this->minutiaes.write(fingerprint, fingerprint.getMinutiae(), path, filename);
}

void FingerprintProcessor::write(const Mat& img, const std::string& path)
{
	Mat tmp;
	img.copyTo(tmp);
	
	tmp.convertTo(tmp, CV_8UC3, 255);

	if (tmp.size().height == 0 || tmp.size().width == 0)
	{
		tmp = Mat::zeros(50, 50, CV_8UC3);
	}
	
	std::stringstream ss; ss.str(""); ss << path << ".jpg";
	imwrite(ss.str(), tmp);
}


void FingerprintProcessor::normalize(Fingerprint& fingerprint, const bool verbose) const
{
	ImageProcessor::resize(fingerprint, 1.1);
	
	const auto blurred = ImageProcessor::getBlurred(fingerprint, 5);
	const auto normalized = ImageProcessor::normalize(blurred);
	
	fingerprint.setNormalized(normalized);

	if (verbose)
	{
		this->displayNormalized(fingerprint, "normalize");
	}
}

void FingerprintProcessor::adapt(Fingerprint& fingerprint, const int blockSize, const int windowSize, const float trashHold)
{
	fingerprint
		.setWindowSize(windowSize)
		.setBlockSize(blockSize)
		.setTrashHold(trashHold);
	
	ImageProcessor::convertTo(fingerprint, CV_32F);
}
