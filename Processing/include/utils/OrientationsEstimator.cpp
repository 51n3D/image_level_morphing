#include "OrientationsEstimator.h"
#include "ImageProcessor.h"

using namespace processing::utils;
using namespace cv;

int OrientationsEstimator::displayed = 0;
const std::string OrientationsEstimator::class_name = "OrientationsEstimator::";

Mat OrientationsEstimator::estimate(const Mat& fingerprint)
{
	this->orientations = Mat(fingerprint.size(), CV_32FC2, Scalar(0, 1));
	
	this->computeGradients(fingerprint);

	this->compute(fingerprint);

	if (this->isVerbose())
	{
		std::stringstream ss; ss << class_name << "estimate";
		
		this->display(fingerprint, this->orientations, ss.str());
	}
	
	return this->orientations;
}

void OrientationsEstimator::display(const Mat& fingerprint, const Mat& orientations, const std::string& trace, const bool blank) const
{
	auto tmp = this->getOrientationImage(fingerprint, orientations, blank);
	
	std::stringstream ss; ss << ++displayed << ": Orientations" << " TRACE: " << trace;
	imshow(ss.str(), tmp);
}

void OrientationsEstimator::write(const cv::Mat& fingerprint, const cv::Mat& orientations, const std::string& path, const bool blank) const
{
	auto tmp = this->getOrientationImage(fingerprint, orientations, blank);
	
	tmp.convertTo(tmp, CV_8UC3, 255);

	std::stringstream ss; ss << path << ".jpg";
	imwrite(ss.str(), tmp);
}

/////////////////////////////////////// Private members ///////////////////////////////////////

void OrientationsEstimator::computeGradients(const Mat& fingerprint)
{
	Sobel(fingerprint, this->gradX, fingerprint.type(), 1, 0, 3);
	Sobel(fingerprint, this->gradY, fingerprint.type(), 0, 1, 3);
}

void OrientationsEstimator::compute(const Mat& fingerprint)
{	
	const auto rows = fingerprint.rows;
	const auto cols = fingerprint.cols;

	Mat phiX = Mat::zeros(this->orientations.size(), CV_32F);
	Mat phiY = Mat::zeros(this->orientations.size(), CV_32F);
	
	for (auto i = this->blockSize / 2.0f; i < rows; i += this->blockSize)
	{
		for (auto j = this->blockSize / 2.0f; j < cols; j += this->blockSize)
		{
			auto vsx = .0f;
			auto vsy = .0f;

			// pozriem ci neskocim mimo, ak hej zmensim velkost bloku
			const auto blockH = ((rows - i) < this->blockSize / 2.0f) ? (rows - i) : this->blockSize / 2.0f;
			const auto blockW = ((cols - j) < this->blockSize / 2.0f) ? (cols - j) : this->blockSize / 2.0f;

			for (auto u = i - this->blockSize / 2; u < i + blockH; u++)
			{
				for (auto v = j - this->blockSize / 2; v < j + blockW; v++)
				{
					vsy += 2 * this->gradX.at<float>(u, v) * this->gradY.at<float>(u, v);
					vsx += pow(this->gradX.at<float>(u, v), 2) - pow(this->gradY.at<float>(u, v), 2);
				}
			}

			if (vsx != .0f)
			{
				const auto orientation = .5 * fastAtan2(vsy, vsx) * CV_PI / 180;
				
				for (auto u = i - this->blockSize / 2; u < i + blockH && u < rows; u++)
				{
					for (auto v = j - this->blockSize / 2; v < j + blockW && v < cols; v++)
					{
						this->orientations.at<Vec2f>(u, v)[0] = orientation;
						this->orientations.at<Vec2f>(u, v)[1] = 1;

						phiX.at<float>(u, v) = cos(2.0f * orientation);
						phiY.at<float>(u, v) = sin(2.0f * orientation);
					}
				}
			}
		}
	}

	if (this->lowPassFilter)
	{
		this->filterOrientations(phiX, phiY);
	}
}

void OrientationsEstimator::filterOrientations(Mat& phiX, Mat& phiY)
{
	auto filteredX = ImageProcessor::getBlurred(phiX, 7 * blockSize);
	auto filteredY = ImageProcessor::getBlurred(phiY, 7 * blockSize);

	for (auto i = this->blockSize / 2; i < this->orientations.rows; i += this->blockSize)
	{
		for (auto j = this->blockSize / 2; j < this->orientations.cols; j += this->blockSize)
		{
			// pozriem ci neskocim mimo, ak hej zmensim velkost bloku
			const auto blockH = ((orientations.rows - i) < this->blockSize / 2.0f) ? (orientations.rows - i) : this->blockSize / 2.0f;
			const auto blockW = ((orientations.cols - j) < this->blockSize / 2.0f) ? (orientations.cols - j) : this->blockSize / 2.0f;

			auto orientation = .0f;
			if (filteredX.at<float>(i, j) != .0f)
			{
				orientation = .5 * fastAtan2(filteredY.at<float>(i, j), filteredX.at<float>(i, j)) * CV_PI / 180;
			}

			for (auto u = i - this->blockSize / 2; u < i + blockH; u++)
			{
				for (auto v = j - this->blockSize / 2; v < j + blockW; v++)
				{
					this->orientations.at<Vec2f>(u, v)[0] = orientation;
					
				}
			}
		}
	}
}


Mat OrientationsEstimator::getOrientationImage(const Mat& fingerprint, const Mat& orientations, const bool blank) const
{
	Mat tmp;
	if (blank)
	{
		tmp = Mat::zeros(orientations.size(), CV_32F);
	}
	else
	{
		fingerprint.copyTo(tmp);
	}

	const auto lineColor = Scalar(1, 1, 1, 0);
	for (auto row = this->blockSize / 2; row < orientations.rows; row += this->blockSize)
	{
		for (auto col = this->blockSize / 2; col < orientations.cols; col += this->blockSize)
		{
			if (orientations.at<Vec2f>(row, col)[1] == 0)
			{
				rectangle(tmp,
					Point(col - blockSize / 2, row - blockSize / 2),
					Point(col + blockSize / 2, row + blockSize / 2),
					Scalar(0, 0, 0),
					FILLED,
					LINE_8);

				continue;
			}

			const auto x1 = col + this->blockSize / 2 * cos(orientations.at<Vec2f>(row, col)[0] - CV_PI / 2);
			const auto y1 = row + this->blockSize / 2 * sin(orientations.at<Vec2f>(row, col)[0] - CV_PI / 2);
			const auto x2 = col - this->blockSize / 2 * cos(orientations.at<Vec2f>(row, col)[0] - CV_PI / 2);
			const auto y2 = row - this->blockSize / 2 * sin(orientations.at<Vec2f>(row, col)[0] - CV_PI / 2);

			const auto a = Point(x1, y1);
			const auto b = Point(x2, y2);

			line(tmp, a, b, lineColor, 2);
		}
	}

	return tmp;
}
