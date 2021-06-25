#pragma once

#include <QSpinBox>

namespace app
{
	namespace widgets
	{
		namespace cutline
		{
			class ItersectionAreaWidth : public QSpinBox
			{
				Q_OBJECT

			private:
				std::map<QString, int> defaults = {
					{"max", 60}, {"min", 10},
					{"step", 5},
					{"value", 30}
				};

			public:
				ItersectionAreaWidth(QWidget* parent);
				~ItersectionAreaWidth();

			public slots:
				void setDefaultValue();
			};
		}
	}
}
