#pragma once

#include <QSpinBox>

namespace app
{
	namespace widgets
	{
		namespace alignment
		{
			class MinimalOverlay : public QSpinBox
			{
				Q_OBJECT
				
			private:
				std::map<QString, int> defaults = {
					{"max", 80}, {"min", 20},
					{"step", 5},
					{"value", 70}
				};

				QString suffix = "%";

			public:
				MinimalOverlay(QWidget* parent);
				~MinimalOverlay();

			public slots:
				void setDefaultValue();
			};
		}
	}
}
