#pragma once

#include <QRadioButton>

namespace app
{
	namespace widgets
	{
		namespace cutline
		{
			class MorphingMethod : public QRadioButton
			{
				Q_OBJECT

			private:
				std::map<QString, double> defaults = {
					{"max", 1}, {"min", 0},
					{"step", 0.05},
					{"Ow", 0.35}, {"Vw", 0.15}, {"Mw", 0.5}
				};

			public:
				MorphingMethod(QWidget* parent);
				~MorphingMethod();

			public slots:
				void setDefaultValue();
			};
		}
	}
}