#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_DemoApp.h"
#include "MorphingProcessor.h"
#include "action/ActionManager.h"
#include "action/TestCase.h"

namespace app
{
	class DemoApp : public QMainWindow
	{
		Q_OBJECT

	private:
		// members
		Ui::DemoAppClass ui;
		/**
		 * \brief Handler jednotlivých akcíí aplikácie.
		 */
		action::ActionManager* manager;
		/**
		 * \brief Handlar testov.
		 */
		action::TestCase* testing;

		// methods
		/**
		 * \brief Prepojí signály a sloty.
		 */
		void connect();
		/**
		 * \brief Prepojí signály a sloty na defaultné nastavenia.
		 */
		void connectPresets();
		/**
		 * \brief Prepojí signály a sloty menu.
		 */
		void connectMenu();
		/**
		 * \brief Prepojí nástroje na morfovanie.
		 */
		void connectMorphingTools();
		
	public:
		// enums
		enum SensorTypes { NONE = -1, S1 = 1, S2, S3, SYNTHETIC };
		
		// static members
		/**
		 * \brief Cesta k dátam.
		 */
		static const std::string data;
		/**
		 * \brief Senzory.
		 */
		static std::map<int, QString> sensors;
		/**
		 * \brief Aktuálny senzor.
		 */
		static int currentSensor;
		
		// constructors
		DemoApp(QWidget* parent = Q_NULLPTR);

		// methods
		void setUp();

	signals:
		void preset();
		void init();
		void sensorChanged(int);
		void reportProgress(QString);

		void sentFingerprint(QString);
		void sentAlignedFingerprint(QString);
		void sentAlignmentThreshold(int);
		void sentCutlineWeights(double, double, double);
		void sentCutlineIntersectionAreaWidth(int);
		void sentCutlineRotationStep(int);
		void sentUseDynamicCutline(bool);
		void sentUseAdaptiveMethod(bool);
		void sentDynamicCutlineSizeOfSearchArea(int);
		void sentGeneratorBorderWidth(int);
		void sentBackgroundColor(int);

		void parametersWereSet();

		void enableSynthetic(bool);
		void enableReal(bool);
		
	public slots:
		void setDefaultSensor();
		void setSensor();
		void disableButtons();
		void disableWidgets();
		void enableWidgets();
		void setupParameters();

		void enableTestingResultsTab(bool enabled);
	};
}
