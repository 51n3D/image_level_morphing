#include "widgets/cutline/ItersectionAreaWidth.h"

using namespace app::widgets::cutline;

ItersectionAreaWidth::ItersectionAreaWidth(QWidget *parent) : QSpinBox(parent)
{
	this->setMaximum(this->defaults["max"]);
	this->setMinimum(this->defaults["min"]);

	this->setSingleStep(this->defaults["step"]);
}

ItersectionAreaWidth::~ItersectionAreaWidth()
{
	
}

void ItersectionAreaWidth::setDefaultValue()
{
	this->setValue(this->defaults["value"]);
}
