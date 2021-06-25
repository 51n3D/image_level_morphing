#pragma once

#include <QDoubleSpinBox>

namespace app
{
	namespace widgets
	{
		namespace cutline
		{
			class WeightingFactor : public QDoubleSpinBox
			{
				Q_OBJECT

			private:
				std::map<QString, double> defaults = {
					{"max", 1}, {"min", 0},
					{"step", 0.05},
					{"Ow", 0.35}, {"Vw", 0.15}, {"Mw", 0.5}
				};

			public:
				WeightingFactor(QWidget* parent);
				~WeightingFactor();

			public slots:
				void setDefaultValue();
			};
		}
	}
}