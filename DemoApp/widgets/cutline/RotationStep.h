#pragma once

#include <QSpinBox>

namespace app
{
	namespace widgets
	{
		namespace cutline
		{
			class RotationStep : public QSpinBox
			{
				Q_OBJECT

			private:
				std::map<QString, int> defaults = {
					{"max", 100}, {"min", 10},
					{"step", 5},
					{"value", 60}
				};
				
			public:
				RotationStep(QWidget* parent);
				~RotationStep();

			public slots:
				void setDefaultValue();
			};
		}
	}
}
