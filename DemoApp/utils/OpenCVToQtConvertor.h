#pragma once

#include <opencv2/opencv.hpp>

#include <qpixmap.h>

namespace app
{
	namespace utils
	{
		class OpenCVToQtConvertor
		{
		public:
			static QPixmap matToQtPixmap(const cv::Mat& img, const QSize& size, bool fromGray = true);
			static cv::Mat adaptColorRangesToQt(const cv::Mat& img, bool colored = false);
		};
	}
}
