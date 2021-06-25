#pragma once

#include <QSpinBox>

namespace app
{
	namespace widgets
	{
		namespace cutline
		{
			namespace dynamic
			{
				class SearchAreaSize : public QSpinBox
				{
					Q_OBJECT

				private:
					std::map<QString, int> defaults = {
						{"max", 100}, {"min", 10},
						{"step", 2},
						{"value", 64}
					};
				
				public:
					SearchAreaSize(QWidget* parent);
					~SearchAreaSize();

				public slots:
					void setDefaultValue();
				};
			}
		}
	}
}
