#include "widgets/cutline/dynamic/SearchAreaSize.h"

using namespace app::widgets::cutline::dynamic;

SearchAreaSize::SearchAreaSize(QWidget *parent) : QSpinBox(parent)
{
	this->setMaximum(this->defaults["max"]);
	this->setMinimum(this->defaults["min"]);

	this->setSingleStep(this->defaults["step"]);
}

SearchAreaSize::~SearchAreaSize()
{
}

void SearchAreaSize::setDefaultValue()
{
	this->setValue(this->defaults["value"]);
}
