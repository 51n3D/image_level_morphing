#pragma once

#include <QGroupBox>

namespace app
{
	namespace widgets
	{
		namespace cutline
		{
			namespace dynamic
			{
				class DynamicCutlineGroup : public QGroupBox
				{
					Q_OBJECT

				private:
					std::map<QString, bool> defaults = {
						{"checkable", true},
						{"checked", false}
					};

				public:
					DynamicCutlineGroup(QWidget* parent);
					~DynamicCutlineGroup();

				public slots:
					void setDefaultValue();
				};
			}
		}
	}
}
