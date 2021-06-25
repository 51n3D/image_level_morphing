#pragma once

#include <QPlainTextEdit>

namespace app
{
	namespace widgets
	{
		class GuiLogger : public QPlainTextEdit
		{
			Q_OBJECT

		private:
			// members
			int currentLine;
			
		public:
			GuiLogger(QWidget* parent);
			~GuiLogger();

		public slots:
			void log(QString message);
			void clear();
		};
	}
}
