#include "FrequenciesEstimator.h"
#include "ImageProcessor.h"

using namespace processing::utils;
using namespace processing;
using namespace cv;

int FrequenciesEstimator::displayed = 0;
const std::string FrequenciesEstimator::class_name = "FrequenciesEstimator::";

FrequenciesEstimator::FrequenciesEstimator(const Size& size, const int type)
{
	this->frequencies = Mat::zeros(size, type);
	this->regionMask = RegionMask(this->frequencies);
}

Mat FrequenciesEstimator::estimate(const Mat& fingerprint)
{
	this->frequencies = Mat::zeros(fingerprint.size(), CV_32F);
	this->regionMask = RegionMask(fingerprint);
	
	this->compute(fingerprint);

	this->applyRegionMask();

	if (this->interpolation)
	{
		this->tmpFrequencies = Mat::zeros(this->frequencies.size(), this->frequencies.type());
		
		this->interpolateFrequencies(fingerprint);
	}

	if (this->isVerbose())
	{
		std::stringstream ss; ss << class_name << "estimate";

		this->display(this->frequencies, ss.str());
	}

	return this->frequencies;
}

void FrequenciesEstimator::display(const Mat& frequencies, const std::string& trace) const
{
	std::stringstream ss; ss << ++displayed << ": Frequencies" << " TRACE: " << trace;
	cv::imshow(ss.str(), frequencies);
}

void FrequenciesEstimator::write(const Mat& frequencies, const std::string& path) const
{
	Mat tmp;
	frequencies.copyTo(tmp);
	
	tmp.convertTo(tmp, CV_8UC3, 255);

	std::stringstream ss; ss.str(""); ss << path << ".jpg";
	imwrite(ss.str(), tmp);
}

/////////////////////////////////////// Private members ///////////////////////////////////////

void FrequenciesEstimator::compute(const Mat& fingerprint)
{
	for (auto i = this->blockSize / 2; i < fingerprint.rows; i += this->blockSize)
	{
		for (auto j = this->blockSize / 2; j < fingerprint.cols; j += this->blockSize)
		{	
			const auto signature = this->computeXSignature(i, j, fingerprint);
			
			const auto peaks = findPeaks(signature);
			
			const auto avg = averagePeakDistance(peaks[0]);

			const auto frequency = 1 / avg;

			this->addBlockInfoToRegionMask(fingerprint, i, j, frequency, avg, peaks, signature);
			
			this->placeFrequency(this->frequencies, i, j, frequency);

			this->maxMinFrequency(i, j);
		}
	}
}

std::vector<float> FrequenciesEstimator::computeXSignature(const int i, const int j, const Mat& fingerprint)
{
	std::vector<float> blockXSignature;
	
	for (auto k = -this->windowSize / 2; k < this->windowSize / 2; k++)
	{
		auto sum = .0f;
		auto count = 0;
		
		for (auto d = 0; d < this->blockSize; d++)
		{
			const auto angle = this->orientations.at<Vec2f>(i, j)[0] - CV_PI / 2;
			
			const auto u = round(i + (d - this->blockSize / 2.0) * sin(angle) + (0.5 - k) * cos(angle));
			const auto v = round(j + (d - this->blockSize / 2.0) * cos(angle) + (k - 0.5) * sin(angle));
			
			if (u >= 0 && u < fingerprint.rows && v >= 0 && v < fingerprint.cols 
				&& this->orientations.at<Vec2f>(u, v)[1] > 0)
			{
				sum += fingerprint.at<float>(u, v);
				count++;
			}
		}

		// otocim hodnoty opacne pretoze 0 reprezentuje ciernu, co znamena ze ide o liniu,
		// chcem aby linia tvorila vrchol sinusoidy takze hodnoty obratim
		blockXSignature.emplace_back(1 - sum / static_cast<float>(count));
	}

	return blockXSignature;
}

std::array<std::vector<int>, 2> FrequenciesEstimator::findPeaks(const std::vector<float>& signal)
{
	std::array<std::vector<int>, 2> indices;

	for (auto i = 0; i < signal.size(); i++)
	{
		// vyska vrcholov
		if (i == 0 
			&& signal.at(i) > 0.5
			&& signal.at(i) >= signal.at(i + 1.0))
		{
			validatePeak(i, indices[0], signal);
		}
		else if (i > 0 && i < signal.size() - 1
			&& signal.at(i) > 0.5
			&& signal.at(i - 1.0) <= signal.at(i)
			&& signal.at(i) >= signal.at(i + 1.0))
		{
			validatePeak(i, indices[0], signal);
		}
		else if (i == signal.size() - 1 
			&& signal.at(i) > 0.5
			&& signal.at(i) >= signal.at(i - 1.0))
		{
			validatePeak(i, indices[0], signal);
		}

		// hlbka priestorov medzi vrcholmi
		if (i == 0
			&& signal.at(i) < 0.5
			&& signal.at(i) <= signal.at(i + 1.0))
		{
			validatePeak(i, indices[1], signal, true);
		}
		else if (i > 0 && i < signal.size() - 1
			&& signal.at(i - 1.0) >= signal.at(i)
			&& signal.at(i) <= signal.at(i + 1.0))
		{
			validatePeak(i, indices[1], signal, true);
		}
		else if (i == signal.size() - 1
			&& signal.at(i) < 0.5
			&& signal.at(i) <= signal.at(i - 1.0))
		{
			validatePeak(i, indices[1], signal, true);
		}
	}
	
	return indices;
}

void FrequenciesEstimator::validatePeak(const int index, std::vector<int>& indices, const std::vector<float>& signal, const bool valley)
{
	if (!indices.empty())
	{
		const auto last = indices.back();

		// pozriem ako daleko som od posledneho
		if (index - last > 6)
		{
			// som dalej
			indices.push_back(index);
		}
		else
		{
			// nie som
			if (valley)
			{
				// ma vhodnejsiu hodnotu?
				if (signal.at(index) < signal.at(last))
				{
					indices.pop_back();
					indices.push_back(index);
				}
			}
			else
			{
				// ma vhodnejsiu hodnotu?
				if (signal.at(index) > signal.at(last))
				{
					indices.pop_back();
					indices.push_back(index);
				}
			}
		}
	}
	else
	{
		indices.push_back(index);
	}
}

float FrequenciesEstimator::averagePeakDistance(const std::vector<int>& peaks)
{
	float avg = -1;
	if (peaks.size() > 1)
	{
		auto sum = .0f;
		for (auto l = 0; l < peaks.size() - 1; l++)
		{
			sum += peaks.at(l + 1) - peaks.at(l);
		}

		avg = sum / static_cast<float>(peaks.size() - 1);

		// toto nemoze prekrocit 500 DPI obrazok
		if (avg < 3 || avg > 25)
		{
			avg = -1;
		}
	}

	return avg;
}

void FrequenciesEstimator::placeFrequency(Mat& map, const int i, const int j, const float freq) const
{
	for (auto x = -this->blockSize / 2; x < this->blockSize / 2 && i + x < map.rows; x++)
	{
		for (auto y = -this->blockSize / 2; y < this->blockSize / 2 && j + y < map.cols; y++)
		{
			map.at<float>(i + x, j + y) = freq;
		}
	}
}

void FrequenciesEstimator::maxMinFrequency(const int i, const int j)
{
	if (this->maxF < this->frequencies.at<float>(i, j))
	{
		this->maxF = this->frequencies.at<float>(i, j);
	}

	if (this->frequencies.at<float>(i, j) != -1
		&& this->minF > this->frequencies.at<float>(i, j))
	{
		this->minF = this->frequencies.at<float>(i, j);
	}
}

void FrequenciesEstimator::interpolateFrequencies(const Mat& fingerprint)
{
	const auto filtered = ImageProcessor::getBlurred(this->frequencies, 7 * blockSize, 9 * blockSize);

	for (auto i = this->blockSize / 2; i < this->frequencies.rows; i += this->blockSize)
	{
		for (auto j = this->blockSize / 2; j < this->frequencies.cols; j += this->blockSize)
		{
			auto freq = this->frequencies.at<float>(i, j);

			// nepodareny odhad frekvencie, interpolujem
			if (freq == -1)
			{
				auto top = .0f;
				auto bottom = .0f;

				for (auto x = -this->blockSize; x <= this->blockSize && i + x < this->frequencies.rows && i + x > 0; x += blockSize)
				{
					for (auto y = -this->blockSize; y <= this->blockSize && j + y < this->frequencies.cols && j + y > 0; y += blockSize)
					{
						const auto filteredFreq = filtered.at<float>(i + x, j + y);

						top += filteredFreq <= 0 ? 0 : filteredFreq;
						bottom += filteredFreq + 1 <= 0 ? 0 : 1;
					}
				}

				// delenie nulouuu
				if (bottom > 0)
				{
					freq = top / bottom;
				}
				else
				{
					// ulozim to co som nasiel, aspon nieco (priemerovanim sa to upravy)
					freq = top;
				}
			}

			this->placeFrequency(this->tmpFrequencies, i, j, freq);
		}
	}

	// vyhladim frekvencie
	this->frequencies = ImageProcessor::getBlurred(this->frequencies, 7 * blockSize, 9 * blockSize);
}

void FrequenciesEstimator::addBlockInfoToRegionMask(const Mat& img, const int i, const int j, const float frequency, const float peakDistance,
	const std::array<std::vector<int>, 2>& peaks, const std::vector<float>& signal)
{
	// check bounds of img
	const auto width = img.size().width;
	const auto height = img.size().height;
	const auto blockW = ((width - j) < blockSize * 4) ? (width - j) : blockSize * 4;
	const auto blockH = ((height - i) < blockSize * 4) ? (height - i) : blockSize * 4;

	const auto block = img(Rect(j, i, blockW, blockH));

	Scalar mean, dev;
	meanStdDev(block, mean, dev);
	const auto var = pow(dev.val[0], 2);

	const auto amplitude = RegionMask::amplitude(peaks, signal);

	// jedna sa o popredie odtlacku?
	auto recoverable = false;
	if (amplitude > 0.2 && var > 0.01 && peakDistance != -1)
	{
		// pridam vzdialenost vrcholov sinusoidy do regionalnej masky (vyuzijem to
		// pri vylepseni kvality)
		recoverable = true;
		this->regionMask.addPeaksDistance(peakDistance);
	}

	// ulozim informaciu o regione
	this->regionMask.at<float>(i, j) = recoverable ? 1 : 0;
}

void FrequenciesEstimator::applyRegionMask()
{
	// vyplnim priestory v odtlacku
	const auto region = ImageProcessor::correctSegmentation(this->regionMask, this->blockSize);
	regionMask.setRegion(region);

	// orezem vlastnosti a aj masku aby som neukladal zbytocne prazdne priestory
	ImageProcessor::trim(this->regionMask, region);
	ImageProcessor::trim(this->frequencies, region);

	for (auto i = blockSize / 2; i < this->frequencies.rows; i += blockSize)
	{
		for (auto j = blockSize / 2; j < this->frequencies.cols; j += blockSize)
		{
			if (this->regionMask.at<float>(i, j) == 0)
			{
				this->placeFrequency(this->frequencies, i, j, 0);
			}
		}
	}
}
