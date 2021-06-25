#include "widgets/SelectFingerprint.h"
#include "DemoApp.h"

#include <filesystem>

using namespace app::widgets;

namespace fs = std::filesystem;

SelectFingerprint::SelectFingerprint(QWidget* parent) : QComboBox(parent)
{
}

SelectFingerprint::~SelectFingerprint()
{
}

void SelectFingerprint::reset()
{
	this->setCurrentIndex(0);
}

void SelectFingerprint::loadData(const int type)
{
	std::stringstream ss; ss << DemoApp::data << DemoApp::sensors[DemoApp::currentSensor].toStdString();
	for (const auto& entry : fs::directory_iterator(ss.str()))
	{
		const auto filename = QString(entry.path().stem().string().c_str());

		this->addItem(filename);
	}
}
