#pragma once

#include <QComboBox>

namespace app
{
	namespace widgets
	{
		class SelectFingerprint : public QComboBox
		{
			Q_OBJECT

		public:
			SelectFingerprint(QWidget* parent);
			~SelectFingerprint();

		public slots:
			void reset();
			void loadData(int type);
		};
	}
}
