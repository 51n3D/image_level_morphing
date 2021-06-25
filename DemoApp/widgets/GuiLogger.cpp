#include "widgets/GuiLogger.h"

#include <QScrollBar>
#include <sstream>

using namespace app::widgets;

GuiLogger::GuiLogger(QWidget *parent) : QPlainTextEdit(parent)
{
	this->setReadOnly(true);
	this->currentLine = 1;
}

GuiLogger::~GuiLogger()
{
}

void GuiLogger::log(QString message)
{
	std::stringstream ss; ss << "<strong>" << this->currentLine << ": " << "</strong>";
	this->currentLine++;
	
	message.prepend(ss.str().c_str());
	this->appendHtml(message);
	this->verticalScrollBar()->setValue(this->verticalScrollBar()->maximum());
}

void GuiLogger::clear()
{
	this->setPlainText("");
	this->currentLine = 1;
}
