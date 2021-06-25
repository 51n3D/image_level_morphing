#include "widgets/generator/BackgroundColor.h"

#include "DemoApp.h"

using namespace app::widgets::generator;

BackgroundColor::BackgroundColor(QWidget *parent) : QSpinBox(parent)
{
	this->setMaximum(this->defaults["max"]);
	this->setMinimum(this->defaults["min"]);

	this->setSingleStep(this->defaults["step"]);
}

BackgroundColor::~BackgroundColor()
{
}

void BackgroundColor::setDefaultValue()
{
	this->setValue(this->defaults[DemoApp::sensors[DemoApp::currentSensor]]);
}
