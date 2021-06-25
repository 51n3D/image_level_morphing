#pragma once

#include "DisplayFingerprint.h"

#include <QDialog>

namespace app
{
	namespace widgets
	{
		class FingerprintStepsDialog : public QDialog
		{
			Q_OBJECT

		public:
			// constructors
			FingerprintStepsDialog(QWidget* parent);

			// destructors
			~FingerprintStepsDialog();

			// methods
			void addImages(std::vector<QPixmap> images);
		};
	}
}


