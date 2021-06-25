#include "widgets/cutline/WeightingFactor.h"

using namespace app::widgets::cutline;

WeightingFactor::WeightingFactor(QWidget *parent) : QDoubleSpinBox(parent)
{
	this->setMaximum(this->defaults["max"]);
	this->setMinimum(this->defaults["min"]);

	this->setSingleStep(this->defaults["step"]);
}

WeightingFactor::~WeightingFactor()
{
}

void WeightingFactor::setDefaultValue()
{
	this->setValue(this->defaults[this->objectName()]);
}
