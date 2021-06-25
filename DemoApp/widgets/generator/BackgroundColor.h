#pragma once

#include <QSpinBox>

namespace app
{
	namespace widgets
	{
		namespace generator
		{
			class BackgroundColor : public QSpinBox
			{
				Q_OBJECT

			private:
				std::map<QString, int> defaults = {
					{"max", 255}, {"min", 0},
					{"step", 5},
					{"value", 30},
					{"SAGEMMSO", 220}, {"SECUGEN", 240}, {"BERGDATA", 210}, {"SYNTHETIC", 255}
				};

			public:
				BackgroundColor(QWidget* parent);
				~BackgroundColor();

			public slots:
				void setDefaultValue();
			};
		}
	}
}
