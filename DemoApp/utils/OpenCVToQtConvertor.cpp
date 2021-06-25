#include "utils/OpenCVToQtConvertor.h"

using namespace app::utils;
using namespace cv;

QPixmap OpenCVToQtConvertor::matToQtPixmap(const Mat& img, const QSize& size, const bool fromGray)
{
	Mat tmp;
	img.copyTo(tmp);

	// prisposobi velkost obrazku
	if (img.size().height > size.height() || img.size().width > size.width())
	{
		auto scale = static_cast<double>(size.height()) / img.size().height;
		if (scale * img.size().width > size.width())
		{
			scale = static_cast<double>(size.width()) / img.size().width;
		}

		resize(tmp, tmp, Size(0, 0), scale, scale);
	}

	if (fromGray)
	{
		cvtColor(tmp, tmp, COLOR_GRAY2RGB);
	}
	else
	{
		cvtColor(tmp, tmp, COLOR_BGR2RGB);
	}

	const auto pixmap = QPixmap::fromImage(
		QImage(
			static_cast<unsigned char*>(tmp.data),
			tmp.cols, tmp.rows,
			tmp.step,
			QImage::Format_RGB888
		)
	);

	return pixmap;
}

Mat OpenCVToQtConvertor::adaptColorRangesToQt(const Mat& img, const bool colored)
{
	if (!colored)
	{
		Mat tmp(img.size(), CV_8U);
		for (auto i = 0; i < img.rows; i++)
		{
			for (auto j = 0; j < img.cols; j++)
			{
				tmp.at<char>(i, j) = static_cast<int>(img.at<float>(i, j) * 255);
			}
		}

		return tmp;
	}
	
	Mat tmp(img.size(), CV_8UC3);
	for (auto i = 0; i < img.rows; i++)
	{
		for (auto j = 0; j < img.cols; j++)
		{
			const int red = img.at<Vec4f>(i, j)[0] * 255;
			const int green = img.at<Vec4f>(i, j)[1] * 255;
			const int blue = img.at<Vec4f>(i, j)[2] * 255;
			
			
			tmp.at<Vec3b>(i, j)[0] = red;
			tmp.at<Vec3b>(i, j)[1] = green;
			tmp.at<Vec3b>(i, j)[2] = blue;
		}
	}

	return tmp;
}
