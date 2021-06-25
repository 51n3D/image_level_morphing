#pragma once

#include <QSpinBox>

namespace app
{
	namespace widgets
	{
		namespace generator
		{
			class BorderWidth : public QSpinBox
			{
				Q_OBJECT

			private:
				std::map<QString, int> defaults = {
					{"max", 64}, {"min", 0},
					{"step", 2},
					{"value", 16}
				};
				
			public:
				BorderWidth(QWidget* parent);
				~BorderWidth();

			public slots:
				void setDefaultValue();
				
			};
		}
	}
}
