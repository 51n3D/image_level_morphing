#include "widgets/cutline/RotationStep.h"

using namespace app::widgets::cutline;

RotationStep::RotationStep(QWidget *parent) : QSpinBox(parent)
{
	this->setMaximum(this->defaults["max"]);
	this->setMinimum(this->defaults["min"]);

	this->setSingleStep(this->defaults["step"]);
}

RotationStep::~RotationStep()
{
}

void RotationStep::setDefaultValue()
{
	this->setValue(this->defaults["value"]);
}
