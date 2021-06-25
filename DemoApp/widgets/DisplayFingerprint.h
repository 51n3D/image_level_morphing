#pragma once

#include <opencv2/opencv.hpp>

#include <QLabel>

namespace app
{
	namespace widgets
	{
		class DisplayFingerprint : public QLabel
		{
			Q_OBJECT

		public:
			// members
			int width = 241;
			int height = 331;
			
			// constructors
			DisplayFingerprint(QWidget* parent);

			// destructors
			~DisplayFingerprint();

		signals:
			void reportProgress(QString);
			
		public slots:
			void setImage(QString filename);
			void setMorphed(const cv::Mat& morphed);

		};
	}
}
