#include "widgets/generator/BorderWidth.h"

using namespace app::widgets::generator;

BorderWidth::BorderWidth(QWidget *parent) : QSpinBox(parent)
{
	this->setMaximum(this->defaults["max"]);
	this->setMinimum(this->defaults["min"]);

	this->setSingleStep(this->defaults["step"]);
}

BorderWidth::~BorderWidth()
{
}

void BorderWidth::setDefaultValue()
{
	this->setValue(this->defaults["value"]);
}
