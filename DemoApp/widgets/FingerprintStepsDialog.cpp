#include "widgets/FingerprintStepsDialog.h"

#include "utils/OpenCVToQtConvertor.h"

#include <QHBoxLayout>

using namespace app::widgets;

FingerprintStepsDialog::FingerprintStepsDialog(QWidget *parent) : QDialog(parent)
{
}

FingerprintStepsDialog::~FingerprintStepsDialog()
{
}

void FingerprintStepsDialog::addImages(std::vector<QPixmap> images)
{
	auto layout = new QHBoxLayout(this);

	for (const auto& image: images)
	{
		auto label = new DisplayFingerprint(this);
		label->setFixedSize(QSize(300, 400));
		label->setPixmap(image);

		layout->addWidget(label);
	}

	this->setLayout(layout);

	this->setFixedSize(layout->sizeHint());
}
