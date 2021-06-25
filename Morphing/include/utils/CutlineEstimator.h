#pragma once

#include "storage/Cutline.h"

#include <FingerprintProcessor.h>

namespace processing
{
	namespace storage
	{
		class Fingerprint;
	}

	namespace utils
	{
		namespace storage
		{
			class Minutiae;
		}
	}
}

namespace morphing
{
	namespace storage
	{
		class AlignedFingerprint;
	}
	
	namespace utils
	{
		/**
		 * \brief Trieda urcena k odhadu optimalnej reznej linie odtlacku
		 * a zarovnaneho odtlacku.
		 */
		class CutlineEstimator
		{
		private:
			// members
			/**
			 * \brief Maximalna vzdialenost od reznej linie.
			 */
			int dmax = 32;
			/**
			 * \brief Vzdialenost, v ktorej je prehladavana lepsie umiestnenie
			 * linie.
			 */
			int area = 64;
			/**
			 * \brief Krok rotacie reznej linie.
			 */
			float rotStep = CV_PI / 10;
			/**
			 * \brief Indikator kontrolneho vystupu.
			 */
			bool verboseOutput = false;
			/**
			 * \brief Indikator zobrazenia markantov.
			 */
			bool displayMinutiaes = false;
			/**
			 * \brief Indikator pouzitia vyhladania dynamickej linie.
			 */
			bool dynamicCutline = false;
			/**
			 * \brief Indikator pouzitia povodnej metody.
			 */
			bool adaptiveMethod = false;
			
			/**
			 * \brief Rezna linia.
			 */
			storage::Cutline cutline;

			/**
			 * \brief Vaha orientacii na ohodnotenie reznej linie.
			 */
			float oWeight = 1.0f / 3.0f;
			/**
			 * \brief Vaha frekvenci na ohodnotenie reznej linie.
			 */
			float vWeight = 1.0f / 3.0f;
			/**
			 * \brief Vaha markantov na ohodnotenie reznej linie.
			 */
			float mWeight = 1.0f / 3.0f;

			// static members
			/**
			 * \brief Pocet zobrazeni v ramci jedneho behu.
			 */
			static int displayed;

			// methods
			/**
			 * \brief Zahajenie vyhladavania reznej linie a prvotne potrebnej
			 * konfiguracie.
			 * \param af zarovnany odtlacok 
			 * \param f odtlacok
			 */
			void compute(morphing::storage::AlignedFingerprint& af, processing::storage::Fingerprint& f);
			/**
			 * \brief Zahajenie vyhladavania dynamickej reznej linie a prvotne
			 * potrebnej konfiguracie.
			 * \param af zarovnany odtlacok
			 * \param f odtlacok
			 */
			void computeDynamic(morphing::storage::AlignedFingerprint& af, processing::storage::Fingerprint& f);
			/**
			 * \brief Odhadne reprezentaciu reznej linie priamkou.
			 * \param rotation rotacia
			 * \param regionLength pozicia X
			 * \param regionStep pozicia Y
			 * \return 
			 */
			cv::Vec3f estimateCutline(float rotation, float regionLength, float regionStep) const;
			/**
			 * \brief Ohodnoti reznu linie, na zaklade informacii extrahovanych z odtlacku
			 * za pomoci preddefinovanych vah.
			 * \param af zarovnany odtlacok
			 * \param f odtalcok
			 * \param afPos pozicia zarovnaneho odtlacku
			 * \param fPos pozicia odtlacku
			 * \param cLine rezna linia urcena k ohodnoteniu
			 * \param bb "2D bounding box" prekrytia
			 * \param minutiae markanty odtlacku
			 * \param aMinutiae markanty zarovnaneho odtlacku
			 * \return 
			 */
			float score(const morphing::storage::AlignedFingerprint& af, const processing::storage::Fingerprint& f,
				const cv::Point& afPos, const cv::Point& fPos, storage::Cutline& cLine, const std::vector<cv::Point>& bb,
				const std::vector<processing::utils::storage::Minutiae>& minutiae, const std::vector<processing::utils::storage::Minutiae>& aMinutiae) const;
			/**
			 * \brief Separuje markanty na jednotlive strany reznej linie.
			 * \param line rezna linia
			 * \param minutiaes markanty
			 * \return rozdelenie markantov
			 */
			std::vector<std::vector<processing::utils::storage::Minutiae>> minutiaeCardinality(const cv::Vec3f& line,
				const std::vector<processing::utils::storage::Minutiae>& minutiaes) const;
			/**
			 * \brief Ohodnoti rozdelenie markantov.
			 * \param positive kardinalita markantov na pozitivnej strane
			 * \param negative kardinalita markantov na negativnej strane
			 * \return ohodnotenie rozdelenia markantov
			 */
			float minutiaeScore(int positive, int negative) const;
			/**
			 * \brief Ohodnoti rozdelenie markantov metodou dva.
			 * \param positive kardinalita markantov na pozitivnej strane
			 * \param negative kardinalita markantov na negativnej strane
			 * \return ohodnotenie rozdelenia markantov
			 */
			float minutiaeScore2(int positive, int negative) const;
			/**
			 * \brief Vyhodnoti pocetnost marknatov na jednej strane odtlacku za pomoci sigmoidnej
			 * funkcie.
			 * \param v kardinalita markantov na jednej strane reznej linie
			 * \param u parameter spolu s t obmedzuju rozsah ohodnotenia do intervalu <o, 1>
			 * \param t parameter spolu s u obmedzuju rozsah ohodnotenia do intervalu <o, 1>
			 * \return vysledne vyhodnotenie
			 */
			float zSigmoid(int v, float u, float t) const;
			/**
			 * \brief Vykresli markanty do morfovanej oblasti.
			 * \param minutiaes markanty
			 * \param offset posunutie zarovnania
			 * \param color farba
			 * \param container obrazok odtlacku
			 */
			void renderMinutiaes(const std::vector<processing::utils::storage::Minutiae>& minutiaes,
				const cv::Point& offset, const cv::Scalar& color, cv::Mat& container) const;
			/**
			 * \brief Nakofiguruje parametre odfiltrovania tak, aby markanty, ktore sa nachadzaju
			 * mimo morfovanej oblasti boli odfiltrovane.
			 * \param af zarovnany odtlacok
			 * \param f odtlacok
			 * \param adaptPosition indikator shiftnutia markantov na zaklade offsetu zarovnania 
			 * \return markanty prisposobene k morfovanej oblasti
			 */
			std::array<std::vector<processing::utils::storage::Minutiae>, 2> filterMorphedMinutiaes(
				const morphing::storage::AlignedFingerprint& af, const processing::storage::Fingerprint& f, bool adaptPosition = true);
			/**
			 * \brief Odfiltruje marknaty mimo morfovanej oblasti.
			 * \param minutiaes markanty
			 * \param offset offset zarovnania
			 * \param bb "2D bounding box" morfovanej oblasti
			 * \param afPos pozicia zarovnaneho odtlacku
			 * \param fPos pozicia odtlacku
			 * \param oa orientacie zarovnanaeho odlacku
			 * \param o orientacie odtlacku
			 * \param adaptPosition 
			 */
			void morphedAreaMinutiaes(std::vector<processing::utils::storage::Minutiae>& minutiaes,
				const cv::Point& offset, const std::vector<cv::Point>& bb, const cv::Point& afPos, const cv::Point& fPos,
				const cv::Mat& oa, const cv::Mat& o, bool adaptPosition);

		public:
			// static members
			static const std::string class_name;
			
			// constructors 
			explicit CutlineEstimator(processing::FingerprintProcessor& processor);

			// methods
			/**
			 * \brief Zahanie odhadu reznej linie a prvotnej konfiguracie potrebnej k odhadu
			 * \param af zarovanany odtlacok
			 * \param f odtlacok
			 * \return 
			 */
			storage::Cutline estimate(morphing::storage::AlignedFingerprint& af, processing::storage::Fingerprint& f);

			/**
			 * \brief Zobrazi reznu liniu zarovnanych odtlackov spolu s cestou odkial bola
			 * metoda zavolana.
			 * \param af zarovanany odtlacok
			 * \param f odtlacok
			 * \param trace cesta odkial bola metoda zavolana
			 */
			void display(morphing::storage::AlignedFingerprint& af, processing::storage::Fingerprint& f, const std::string& trace);
			/**
			 * \brief Zapise do suboru reznu liniu zarovnanych odtlackov na definovane miesto
			 * urcene parametrom path
			 * \param af zarovanany odtlacok
			 * \param f odtlacok
			 * \param path miesto suboru
			 */
			void write(const morphing::storage::AlignedFingerprint& af, const processing::storage::Fingerprint& f, const std::string& path);

			/* Metoda ziska reprezentaciu obrazku reznej linie v podobe cv::Mat struktury */
			cv::Mat getCutlineImage(const morphing::storage::AlignedFingerprint& af, const processing::storage::Fingerprint& f);
			
			// static methods
			/**
			 * \brief zisti vzdialenost bodu od linie.
			 * \param line linie
			 * \param p bod
			 * \return vzdialenost bodu od linie
			 */
			static float distance(const cv::Vec3f& line, const cv::Point& p);
			/**
			 * \brief Zisti ci sa bod nachadza na strane reznej linie, pre ktoru ma vyhodnotenie
			 * kladny vysledok.
			 * \param line linia
			 * \param pos bod
			 * \return indikator lokacie body na pozitivnej strane reznej linie
			 */
			static bool isPositive(const cv::Vec3f& line, const cv::Point& pos);
			
			// getters
			bool isVerbose() const { return this->verboseOutput; }
			int getMaxDistance() const { return this->dmax; }
			float getRotationStep() const { return this->rotStep; }
			float getOWeight() const { return this->oWeight; }
			float getVWeight() const { return this->vWeight; }
			float getMWeight() const { return this->mWeight; }

			// setters
			CutlineEstimator& setMaxDistance(const int dmax) { this->dmax = dmax; return *this; }
			CutlineEstimator& setRotationStep(const float rotStep) { this->rotStep = rotStep; return *this; }
			CutlineEstimator& setCutlineCount(const int lines) { this->rotStep = CV_PI / lines; return *this; }
			CutlineEstimator& verbose(const bool verboseOutput = true) { this->verboseOutput = verboseOutput; return *this; }
			CutlineEstimator& showMinutiaes(const bool displayMinutiaes = true) { this->displayMinutiaes = displayMinutiaes; return *this; }
			CutlineEstimator& setWeights(const float oW, const float vW, const float mW) { this->oWeight = oW, this->vWeight = vW; this->mWeight = mW; return *this; }
			CutlineEstimator& setArea(const int area) { this->area = area; return *this; }
			CutlineEstimator& useDynamicCutline(const bool dynamicCutline = true) { this->dynamicCutline = dynamicCutline; return *this; }
			CutlineEstimator& useAdaptiveMethod(const bool adaptiveMethod = true) { this->adaptiveMethod = adaptiveMethod; return *this; }
			
		};
	}
}
