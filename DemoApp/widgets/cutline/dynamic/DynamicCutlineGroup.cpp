#include "widgets/cutline/dynamic/DynamicCutlineGroup.h"

using namespace app::widgets::cutline::dynamic;

DynamicCutlineGroup::DynamicCutlineGroup(QWidget *parent) : QGroupBox(parent)
{
}

DynamicCutlineGroup::~DynamicCutlineGroup()
{
}

void DynamicCutlineGroup::setDefaultValue()
{
	this->setCheckable(this->defaults["checkable"]);
	this->setChecked(this->defaults["checked"]);
}
