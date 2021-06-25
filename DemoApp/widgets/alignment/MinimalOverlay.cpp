#include "widgets/alignment/MinimalOverlay.h"

using namespace app::widgets::alignment;

MinimalOverlay::MinimalOverlay(QWidget *parent) : QSpinBox(parent)
{
	this->setMaximum(this->defaults["max"]);
	this->setMinimum(this->defaults["min"]);

	this->setSingleStep(this->defaults["step"]);

	this->setSuffix(this->suffix);
}

MinimalOverlay::~MinimalOverlay()
{
}

void MinimalOverlay::setDefaultValue()
{
	this->setValue(this->defaults["value"]);
}
