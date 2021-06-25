#include "GaborFilter.h"
#include "exceptions/KernelSizeIsNotOdd.h"
#include "ImageProcessor.h"

using namespace processing::utils;
using namespace cv;

int GaborFilter::displayed = 0;
const std::string GaborFilter::class_name = "GaborFilter::";

Mat GaborFilter::filter(const Mat& fingerprint)
{
	if (this->blockSize % 2 == 0)
	{
		throw exception::KernelSizeIsNotOdd();
	}

	this->filtered = Mat::ones(fingerprint.size(), fingerprint.type());
	
	fingerprint.copyTo(this->filtered);

	this->filterImage();

	if (this->isVerbose())
	{
		std::stringstream ss; ss << class_name << "filter";

		this->display(this->filtered, ss.str());
	}
	
	return this->filtered;
}

void GaborFilter::filterImage()
{
	for (auto offset = -this->blockSize / 2; offset <= this->blockSize / 2; offset++)
	{
		for (auto i = offset; i < this->filtered.rows; i += this->blockSize)
		{
			for (auto j = offset; j < this->filtered.cols; j += this->blockSize)
			{
				if (i < 0 || j < 0) continue;

				const auto orientation = this->orientations.at<Vec2f>(i, j)[0];
				const auto frequency = this->frequencies.at<float>(i, j);

				// pozriem co filtrujem, aby gaborov filter nezanasal chybu, ak som mimo obrazka,
				// vyplnim ho ciernym blokom, kvoli krajsiemu vyzualnemu vystupu a pre zjednodusenie
				// stencenia priestorov medzi liniami
				if (!this->isFilterable(i, j))
				{
					auto b = this->blockAt(this->frequencies, i, j);
					b = Mat::zeros(b.rows, b.cols, CV_32F);
					this->blockCopyTo(b, this->filtered, i, j);

					continue;
				}

				auto block = this->blockAt(this->filtered, i, j);

				this->filterBlock(block, orientation, frequency);

				this->blockCopyTo(block, this->filtered, i, j);
			}
		}
	}
}

cv::Mat GaborFilter::blockAt(const Mat& fingerprint, const int i, const int j) const
{
	const auto h = ((this->filtered.rows - i) < this->blockSize / 2 + 1) ? this->blockSize / 2 + (this->filtered.rows - i) : this->blockSize;
	const auto w = ((this->filtered.cols - j) < this->blockSize / 2 + 1) ? this->blockSize / 2 + (this->filtered.cols - j) : this->blockSize;

	const auto x = j - this->blockSize / 2 < 0 ? 0 : j - this->blockSize / 2;
	const auto y = i - this->blockSize / 2 < 0 ? 0 : i - this->blockSize / 2;

	auto block = this->filtered(Rect(x, y, w, h));

	return block;
}

void GaborFilter::blockCopyTo(const Mat& block, Mat& fingerprint, const int i, const int j) const
{
	const auto x = j - this->blockSize / 2 < 0 ? 0 : j - this->blockSize / 2;
	const auto y = i - this->blockSize / 2 < 0 ? 0 : i - this->blockSize / 2;
	
	block.copyTo(this->filtered(Rect(x, y, block.cols, block.rows)));
}

void GaborFilter::filterBlock(cv::Mat& block, const float orientation, const float frequency) const
{
	const auto kernel = getGaborKernel(
		Size(this->blockSize, this->blockSize), this->deviation,
		orientation, 1 / frequency, 1, 0, CV_32F
	);

	filter2D(block, block, CV_32F, kernel);

	double min, max;
	minMaxLoc(block, &min, &max);
	block.convertTo(block, CV_32F, 1 / (max - min), min / (min - max));
}

bool GaborFilter::isFilterable(const int i, const int j)
{
	const auto halfBlock = this->blockSize / 2 + 1;
	const auto around = this->blockSize % 2 == 0 ? halfBlock : halfBlock + 1;

	// hlavou myslienkou je, ci gaborov filter sa nesnazi zffiltrovat nieco,
	// kde nema data, nechcem aby zavadzal chybu do vylepsenia
	return (
		i > around && j > around
		&& i < this->filtered.rows - around
		&& j < this->filtered.cols - around
		&& this->orientations.at<Vec2f>(i, j)[1] > .0f
		&& this->orientations.at<Vec2f>(i - around, j - around)[1] > .0f
		&& this->orientations.at<Vec2f>(i + around, j + around)[1] > .0f
		&& this->orientations.at<Vec2f>(i - around, j + around)[1] > .0f
		&& this->orientations.at<Vec2f>(i + around, j - around)[1] > .0f
		&& this->orientations.at<Vec2f>(i - around, j)[1] > .0f
		&& this->orientations.at<Vec2f>(i + around, j)[1] > .0f
		&& this->orientations.at<Vec2f>(i, j + around)[1] > .0f
		&& this->orientations.at<Vec2f>(i, j - around)[1] > .0f
	);
}

void GaborFilter::display(const Mat& fingerprint, const std::string& trace) const
{	
	std::stringstream ss; ss << ++displayed << ": GaborFilter - " << trace;
	imshow(ss.str(), fingerprint);
}

void GaborFilter::write(const Mat& fingerprint, const std::string& path) const
{
	Mat tmp;
	fingerprint.copyTo(tmp);

	tmp.convertTo(tmp, CV_8UC3, 255);
	std::stringstream ss; ss << path << ++displayed << ".jpg";
	imwrite(ss.str(), tmp);
}
