#include "ImageProcessor.h"
#include "exceptions/NoImageFoundException.h"

#include <opencv2/imgproc.hpp>

using namespace processing::utils;
using namespace cv;

Mat ImageProcessor::read(const std::string& imageLocation, const int flags)
{
	auto image =  imread(imageLocation, flags);

	if (!image.data)
	{
		throw exception::NoImageFoundException();
	}

	return image;
}

void ImageProcessor::convertTo(Mat& img, const int type)
{
	img.convertTo(img, type, 1.0 / 255);
}

void ImageProcessor::convertTo(const Mat& grey, Mat& colored, const int type)
{
	cvtColor(grey, colored, type);
}

void ImageProcessor::copyTo(const Mat& input, Mat& output)
{
	input.copyTo(output);
}

void ImageProcessor::resize(Mat& img, const double scale)
{
	cv::resize(img, img, Size(0, 0), scale, scale);
}

void ImageProcessor::resize(Mat & img, const int height)
{
	const auto scale = static_cast<double>(height) / img.size().height;
	cv::resize(img, img, Size(0, 0), scale, scale);
}

Mat ImageProcessor::normalize(const Mat& img)
{
	Mat tmp;
	img.copyTo(tmp);

	double min, max;
	minMaxLoc(tmp, &min, &max);
	tmp.convertTo(tmp, CV_32F, 1 / (max - min), min / (min - max));

	Scalar mean, dev;
	meanStdDev(tmp, mean, dev);
	const auto m = mean.val[0];
	const auto d = dev.val[0];
	const auto m0 = 75.0 / 255.0;

	Mat normalized(tmp.size(), tmp.type());
	for (auto i = 0; i < tmp.rows; i++)
	{
		for (auto j = 0; j < tmp.cols; j++)
		{
			if (tmp.at<float>(i, j) > m)
				normalized.at<float>(i, j) = m0 + sqrt(m0 * pow(tmp.at<float>(i, j) - m, 2) / d);
			else
				normalized.at<float>(i, j) = m0 - sqrt(m0 * pow(tmp.at<float>(i, j) - m, 2) / d);

			if (normalized.at<float>(i, j) < 0)
			{
				normalized.at<float>(i, j) = 0;
			}

			if (normalized.at<float>(i, j) > 1)
			{
				normalized.at<float>(i, j) = 1;
			}
		}
	}

	return normalized;
}

Mat ImageProcessor::getBlurred(const Mat& img, const int kernelSize, const int deviation, const int border)
{
	Mat blurred(img.size(), img.type());
	
	auto gaussX = getGaussianKernel(kernelSize, deviation, CV_32F);
	auto gaussY = getGaussianKernel(kernelSize, deviation, CV_32F);
	auto kernel = gaussX * gaussY.t();

	//sepFilter2D(img, blurred, CV_32F, gaussX, gaussY);
	filter2D(img, blurred, CV_32F, kernel, Point(-1, -1), 0, border);

	return blurred;
}

void ImageProcessor::binarize(Mat& img, const Mat& segmentation)
{
	img.convertTo(img, CV_8UC1);
	img *= 255;

	threshold(img, img, 80, 255, THRESH_BINARY);
	
	img.convertTo(img, CV_32F);
	img /= 255;
}

void ImageProcessor::higherContrast(Mat& img)
{
	Scalar mean, dev;
	meanStdDev(img, mean, dev);
	const auto m = mean.val[0];

	for (auto i = 0; i < img.rows; i++)
	{
		for (auto j = 0; j < img.cols; j++)
		{
			const auto diff = abs(img.at<float>(i, j) - m);

			if (img.at<float>(i, j) < m)
			{
				img.at<float>(i, j) -= (diff / 2);
			}
			else
			{
				img.at<float>(i, j) += (diff / 3);
			}
				
			if (img.at<float>(i, j) < 0)
			{
				img.at<float>(i, j) = 0;
			}
				
			if (img.at<float>(i, j) > 1)
			{
				img.at<float>(i, j) = 1;
			}
		}
	}
}

void ImageProcessor::trim(Mat& img, const std::vector<Point>& region)
{
	const auto& min = region.at(0);
	const auto& max = region.at(1);

	if (max.y > img.rows)
	{
		const auto rows = max.y - img.rows;
		
		const auto rowsOnes = Mat(rows, img.cols, img.type(), Scalar(75.0 / 255.0));
		vconcat(img, rowsOnes, img);
	}

	if (max.x > img.cols)
	{
		const auto cols= max.x - img.cols;
		
		const auto colsOnes = Mat(img.rows, cols, img.type(), Scalar(75.0 / 255.0));
		hconcat(img, colsOnes, img);
	}
	
	img(Rect(min.x, min.y, max.x - min.x, max.y - min.y)).copyTo(img);
}

Mat ImageProcessor::rotate(const Mat& img, const float angle, const std::vector<Point>& identity, const bool fillBackground)
{
	// get rotation matrix for rotating the image around its center in pixel coordinates
	const Point2f center((img.cols - 1) / 2.0, (img.rows - 1) / 2.0);
	auto rot = getRotationMatrix2D(center, -angle, 1.0);
	// determine bounding rectangle, center not relevant
	const auto bbox = RotatedRect(Point2f(), img.size(), -angle).boundingRect2f();
	// adjust transformation matrix
	rot.at<double>(0, 2) += bbox.width / 2.0 - img.cols / 2.0;
	rot.at<double>(1, 2) += bbox.height / 2.0 - img.rows / 2.0;

	Mat dst = Mat::ones(bbox.size(), img.type());
	if (fillBackground)
	{
		warpAffine(img, dst, rot, bbox.size(), INTER_LINEAR, BORDER_CONSTANT, Scalar(1, 1, 1, 1));
	}
	else
	{
		warpAffine(img, dst, rot, bbox.size());
	}

	if (!identity.empty())
	{
		dst(Rect(
			identity[0].x, identity[0].y,
			identity[1].x - identity[0].x,
			identity[1].y - identity[0].y)
		).copyTo(dst);
	}
	
	return dst;
}

void ImageProcessor::thine(Mat& img, const Mat& segmentation)
{
	Mat prev = Mat::zeros(img.size(), CV_32F);
	Mat diff;
	
	do
	{
		thinningIteration(img, 0);
		thinningIteration(img, 1);
		absdiff(img, prev, diff);
		img.copyTo(prev);
	}
	while (countNonZero(diff) > 0);
}

void ImageProcessor::thinningIteration(Mat& img, const int iter)
{
	Mat marker = Mat::zeros(img.size(), CV_32F);

	for (auto i = 1; i < img.rows - 1; i++)
	{
		for (auto j = 1; j < img.cols - 1; j++)
		{
			const auto p2 = img.at<float>(i - 1, j);
			const auto p3 = img.at<float>(i - 1, j + 1);
			const auto p4 = img.at<float>(i, j + 1);
			const auto p5 = img.at<float>(i + 1, j + 1);
			const auto p6 = img.at<float>(i + 1, j);
			const auto p7 = img.at<float>(i + 1, j - 1);
			const auto p8 = img.at<float>(i, j - 1);
			const auto p9 = img.at<float>(i - 1, j - 1);

			const auto a = (p2 == 0 && p3 == 1) + (p3 == 0 && p4 == 1) +
				(p4 == 0 && p5 == 1) + (p5 == 0 && p6 == 1) +
				(p6 == 0 && p7 == 1) + (p7 == 0 && p8 == 1) +
				(p8 == 0 && p9 == 1) + (p9 == 0 && p2 == 1);
			
			const int b = p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9;
			
			const int m1 = iter == 0 ? (p2 * p4 * p6) : (p2 * p4 * p8);
			const int m2 = iter == 0 ? (p4 * p6 * p8) : (p2 * p6 * p8);

			const int n1 = (p2 * p4 == 1) && (p6 + p7 + p8 == 0);
			const int n2 = (p4 * p6 == 1) && (p2 + p8 + p9 == 0);

			if (b >= 2 && b <= 7)
			{
				if (a == 1 && m1 == 0 && m2 == 0)
				{
					marker.at<float>(i, j) = 1;
				}
				else if (a == 2 && (n1 == 1 || n2 == 1))
				{
					marker.at<float>(i, j) = 1;
				}
			}
		}
	}

	img &= ~marker;
}

std::vector<Point> ImageProcessor::correctSegmentation(Mat& segmentation, const int blockSize)
{
	const auto height = segmentation.rows;
	const auto width = segmentation.cols;

	auto minX = segmentation.cols, minY = segmentation.rows, maxX = 0, maxY = 0;

	for (auto i = blockSize / 2; i < height; i += blockSize)
	{
		auto max = 0; auto min = width;
		for (auto j = blockSize / 2; j < width; j += blockSize)
		{
			if (segmentation.at<float>(i, j) == 1)
			{
				if (j > max) max = j;
				if (j < min) min = j;
			}
		}

		for (auto j = blockSize / 2; j < width; j += blockSize)
		{
			if (j >= min && j <= max)
			{
				segmentation.at<float>(i, j) = 1;
			}
			
			//for check bounds of img
			const auto blockH = ((height - i) < blockSize) ? (height - i) : blockSize;
			const auto blockW = ((width - j) < blockSize) ? (width - j) : blockSize;

			for (auto x = i; x < i + blockH; x++)
			{
				for (auto y = j; y < j + blockW; y++)
				{
					segmentation.at<float>(x, y) = segmentation.at<float>(i, j);
				}
			}

			if (segmentation.at<float>(i, j) == 1)
			{
				if (minX > j) { minX = j; }
				if (minY > i) { minY = i; }
				if (maxX < j) { maxX = j; }
				if (maxY < i) { maxY = i; }
			}
		}
	}

	for (auto j = blockSize / 2; j < width; j += blockSize)
	{
		auto max = 0; auto min = height;
		for (auto i = blockSize / 2; i < height; i += blockSize)
		{
			if (segmentation.at<float>(i, j) == 1)
			{
				if (i > max) max = i;
				if (i < min) min = i;
			}
		}

		for (auto i = blockSize / 2; i < height; i += blockSize)
		{
			if (i >= min && i <= max)
			{
				segmentation.at<float>(i, j) = 1;
			}

			//for check bounds of img
			const auto blockH = ((height - i) < blockSize) ? (height - i) : blockSize;
			const auto blockW = ((width - j) < blockSize) ? (width - j) : blockSize;

			for (auto x = i; x < i + blockH; x++)
			{
				for (auto y = j; y < j + blockW; y++)
				{
					segmentation.at<float>(x, y) = segmentation.at<float>(i, j);
				}
			}

			if (segmentation.at<float>(i, j) == 1)
			{
				if (minX > j) { minX = j; }
				if (minY > i) { minY = i; }
				if (maxX < j) { maxX = j; }
				if (maxY < i) { maxY = i; }
			}
		}
	}

	return { {minX, minY} , {maxX, maxY} };
}

void ImageProcessor::addColorBreathToImage(Mat& img, const Scalar& color)
{
	for (auto i = 0; i < img.rows; i++)
	{
		for (auto j = 0; j < img.cols; j++)
		{
			const auto r = img.at<Vec4f>(i, j)[0] * color.val[0];
			const auto g = img.at<Vec4f>(i, j)[1] * color.val[1];
			const auto b = img.at<Vec4f>(i, j)[2] * color.val[2];
			const auto a = img.at<Vec4f>(i, j)[3] * color.val[3];
			
			img.at<Vec4f>(i, j)[0] = r > 1 ? 1 : r;
			img.at<Vec4f>(i, j)[1] = g > 1 ? 1 : g;
			img.at<Vec4f>(i, j)[2] = b > 1 ? 1 : b;
			img.at<Vec4f>(i, j)[3] = a > 1 ? 1 : a;
		}
	}	
}

float ImageProcessor::at(const Mat& obj, const int i, const int j, const float def)
{
	if (i >= 0 && i < obj.rows && j >= 0 && j < obj.cols)
	{
		return obj.at<float>(i, j);
	}

	return def;
}
