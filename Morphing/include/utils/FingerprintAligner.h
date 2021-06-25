#pragma once

#include <FingerprintProcessor.h>

namespace processing
{
	namespace storage
	{
		class Fingerprint;
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
		 * \brief Trieda urcena k zarovnaniu dvoch odtlackov voci sebe.
		 */
		class FingerprintAligner
		{
		private:
			// members
			/**
			 * \brief Ovladac ziskavania informacii z odtlacku.
			 */
			processing::FingerprintProcessor processor;

			/**
			 * \brief Krok posunutia zarovnavaneho odtlacku.
			 */
			int translationStep = 12;
			/**
			 * \brief Krok rotacie zarovnavaneho odtlacku.
			 */
			int rotationStep = 9;
			/**
			 * \brief Prah minimalneho prekrytia.
			 */
			float trashHold = 0.6;
			
			/**
			 * \brief Indikator kontrolneho vystupu.
			 */
			bool verboseOutput = false;
			/**
			 * \brief Zobrazenie prekrytej oblasti.
			 */
			bool showTranslatedArea = true;

			// static members
			/**
			 * \brief Pocet zobrazeni v ramci jedneho behu.
			 */
			static int displayed;

			// methods
			/**
			 * \brief Ohodnotenie podobnisti orientaci v zarovnani odtlackov.
			 * \param af zarovnany odtlacok
			 * \param f odtlaock
			 * \param afPos pozicia zarovnaneho odtlacku
			 * \param fPos pozicia odtlacku
			 * \return ohodnotenie podobnisti orientacii v zarovnani
			 */
			float similarity(morphing::storage::AlignedFingerprint& af, processing::storage::Fingerprint& f, const cv::Point& afPos, const cv::Point& fPos) const;
			/**
			 * \brief Skontroluje ci zarovnana oblast nie je zanedbatelna, na zaklade
			 * prahu minimalneho prekrytia.
			 * \param overlapped pocet blokov nachadzajucich sa v prekryti
			 * \param af zarovnany odtlacok 
			 * \param f odtlacok
			 * \return indikator zanedbatelnosti
			 */
			bool negligibleArea(int overlapped, morphing::storage::AlignedFingerprint& af, processing::storage::Fingerprint& f) const;
			
			// static methods
			/**
			 * \brief Otoci orientacne boli o x stupnov.
			 * \param orientations lokalne orientacie
			 * \param angle uhol
			 * \param bb segmentacia odtlacku.
			 * \return 
			 */
			static cv::Mat rotateOrientations(const cv::Mat& orientations, const float angle, std::vector<cv::Point>& bb);

		public:
			// static members
			static const std::string class_name;
			
			// constructors
			explicit FingerprintAligner(processing::FingerprintProcessor& processor);

			// methods
			/**
			 * \brief Zabezpeci zarovnanie dvoch odtlackov. Pri tomto odhade dochadza ku neustalemu odhadu
			 * lokalnych frekvencii pri kazdom otoceni odtlacku.
			 * \param af zarovnany odtlacok
			 * \param f odtlacok
			 */
			void accurateAlign(morphing::storage::AlignedFingerprint& af, processing::storage::Fingerprint& f);
			/**
			 * \brief Zabezpeci zarovnanie dvoch odtlackov. Orientacne bole je vytiahnute z objektov odtlackov
			 * a nasledne rotovane. 
			 * \param af zarovnany odtlacok
			 * \param f odtlacok
			 */
			void align(morphing::storage::AlignedFingerprint& af, processing::storage::Fingerprint& f) const;

			/* Metoda ziska reprezentaciu obrazku zarovnanych odtlackov v podobe cv::Mat struktury */
			cv::Mat getAlignedFingersImage(const morphing::storage::AlignedFingerprint& af, const processing::storage::Fingerprint& f) const;

			/**
			 * \brief Zobrazi zarovnanie odtlackov spolu s cestou odkial bola
			 * metoda zavolana.
			 * \param af zarovanany odtlacok
			 * \param f odtlacok
			 * \param trace cesta odkial bola metoda zavolana
			 */
			void display(const morphing::storage::AlignedFingerprint& af, const processing::storage::Fingerprint& f, const std::string& trace) const;
			/**
			 * \brief Zapise do suboru zarovnanie odtlackov na definovane miesto
			 * urcene parametrom path
			 * \param af zarovanany odtlacok
			 * \param f odtlacok
			 * \param path miesto suboru
			 */
			void write(const morphing::storage::AlignedFingerprint& af, const processing::storage::Fingerprint& f, const std::string& path) const;

			// static methods
			/**
			 * \brief Zisti poziciu zarovnaneho odtlacku z offestu zarovnania.
			 * \param xOffset offset zarovnania X
			 * \param yOffset offset zarovnania Y
			 * \return pozicia zarovnanheo odtlacku
			 */
			static cv::Point aFingPos(int xOffset, int yOffset);
			/**
			 * \brief Zisti poziciu odtlacku z offestu zarovnania.
			 * \param xOffset offset zarovnania X
			 * \param yOffset offset zarovnania Y
			 * \return pozicia odtlacku
			 */
			static cv::Point fingPos(int xOffset, int yOffset);
			/**
			 * \brief Zisti dlzku objektu, vzniknuteho prekrytim odtlackov.
			 * \param pos pozicia 
			 * \param length1 dlzka prveho prekryteho odtlacku
			 * \param length2 dlzka druheho prekryteho odtlacku
			 * \return dlzka oblasti
			 */
			static int unitLength(const cv::Vec2f& pos, int length1, int length2);
			/**
			 * \brief Zisti "2D bounding box" prekrytej oblasti.
			 * \param fPos pozicia odtlacku
			 * \param afPos pozicia zarovnaneho odtlacku
			 * \param fSize velkost odtlacku
			 * \param afSize velkost zarovnaneho odtlacku
			 * \return "2D bounding box" prekrytej oblasti
			 */
			static std::vector<cv::Point> overlay(const cv::Point& fPos, const cv::Point& afPos, const cv::Size& fSize, const cv::Size& afSize);
			
			// getters
			int getTranslationStep() const { return this->translationStep; }
			int getRotationStep() const { return this->rotationStep; }
			float getTrashHold() const { return this->trashHold; }
			bool isVerbose() const { return this->verboseOutput; }
			bool displayCommonArea() const { return this->showTranslatedArea; }

			// setters
			FingerprintAligner& setTranslationStep(const int translationStep) { this->translationStep = translationStep; return *this; }
			FingerprintAligner& setRotationStep(const int rotationStep) { this->rotationStep = rotationStep; return *this; }
			FingerprintAligner& setTrashHold(const float trashHold) { this->trashHold = trashHold; return *this; }
			FingerprintAligner& verbose(const bool verbose = true) { this->verboseOutput = verbose; return *this; }
			FingerprintAligner& showCommonArea(const bool showTranslatedArea = true) { this->showTranslatedArea = showTranslatedArea; return *this; }

		};
	}
}
