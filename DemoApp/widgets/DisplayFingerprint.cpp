#include "widgets/DisplayFingerprint.h"
#include "utils/OpenCVToQtConvertor.h"
#include "DemoApp.h"

#include <utils/ImageProcessor.h>

using namespace processing::utils;
using namespace app::widgets;
using namespace app;

DisplayFingerprint::DisplayFingerprint(QWidget* parent) : QLabel(parent)
{
	this->setAutoFillBackground(true);

	this->setFrameShape(QFrame::WinPanel);

	this->setFixedWidth(this->width);
	this->setFixedHeight(this->height);

	this->setAlignment(Qt::AlignCenter);
}

DisplayFingerprint::~DisplayFingerprint()
{
}

void DisplayFingerprint::setImage(QString filename)
{
	if (filename.isEmpty())
	{
		return;
	}
	
	std::stringstream ss; ss << DemoApp::data.c_str() << DemoApp::sensors[DemoApp::currentSensor].toStdString() << "\\" << filename.toStdString() << ".bmp";
	
	const auto image = ImageProcessor::read(ss.str());
	const auto pixmap = utils::OpenCVToQtConvertor::matToQtPixmap(image, this->size());

	this->setPixmap(pixmap);

	emit reportProgress(filename.prepend("Selected fingerprint: "));
}

void DisplayFingerprint::setMorphed(const cv::Mat& morphed)
{
	const auto pixmap = utils::OpenCVToQtConvertor::matToQtPixmap(utils::OpenCVToQtConvertor::adaptColorRangesToQt(morphed), this->size());

	this->setPixmap(pixmap);
}

