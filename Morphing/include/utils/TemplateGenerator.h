#pragma once

#include <opencv2/core/mat.hpp>

#include <string>

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
		namespace storage {
			class Cutline;
		}

		/**
		 * \brief Trieda urcena k vygenerovaniu morfovaneho odtlakcu,
		 * zo zarovnanych, s vyucitim informacii ulozenych v reznej linii
		 * storage::Cutline.
		 */
		class TemplateGenerator
		{
		private:
			// members
			/**
			 * \brief Ulozisko morfovaneho odtlacku.
			 */
			cv::Mat morphedTemplate;
			/**
			 * \brief Farba pozadia morfovaneho odtlacku.
			 */
			float backgroundColor;
			/**
			 * \brief Sirka rozmazaneho okraju morfovaneho odtlacku.
			 */
			int border = 16;
			
			// static members
			/**
			 * \brief Pocet zobrazeni v ramci jedneho behu.
			 */
			static int displayed;

			// methods
			/**
			 * \brief Vygeneruje morfovany odtlacok na urvoni obrazku.
			 * \param af zarovnany odtlacok
			 * \param f odtlacok
			 * \param colored indikator farebnosti
			 * \return morfovany odtlacok
			 */
			processing::storage::Fingerprint imageLevelTemplate(const morphing::storage::AlignedFingerprint& af, const processing::storage::Fingerprint& f, bool colored = false);
			/**
			 * \brief TODO Vygeneruje synteticky morfovany odtlacok.
			 * \param af zarovnany odtlacok
			 * \param f odtlacok
			 * \return morfovany odtlacok
			 */
			processing::storage::Fingerprint featureLevelTemplate(const morphing::storage::AlignedFingerprint& af, const processing::storage::Fingerprint& f);

			/**
			 * \brief Vygenerovanie sedotonoveho odtlacku s dvojitou identitou.
			 * \param positive pozitivna strana.
			 * \param negative negativna strana.
			 * \param pR segmentacia pozitivnej strany
			 * \param nR segmentacia negativnej strany
			 * \param bb "2D bounding box" morfovanej oblasti
			 * \param pPos offset pozitivnej strany
			 * \param nPos offset negativnej strany
			 * \param line rezna linia
			 * \return morfovany odtlacok
			 */
			cv::Mat morphedFingerprintIL(const cv::Mat& positive, const cv::Mat& negative, const cv::Mat& pR, const cv::Mat& nR, 
				const std::vector<cv::Point>& bb, const cv::Point& pPos, const cv::Point& nPos, const storage::Cutline& line);
			/**
			 * \brief Vygenerovanie farebneho odtlacku s dvojitou identitou.
			 * \param positive pozitivna strana.
			 * \param negative negativna strana.
			 * \param pR segmentacia pozitivnej strany
			 * \param nR segmentacia negativnej strany
			 * \param bb "2D bounding box" morfovanej oblasti
			 * \param pPos offset pozitivnej strany
			 * \param nPos offset negativnej strany
			 * \param line rezna linia
			 * \return morfovany odtlacok
			 */
			cv::Mat morphedFingerprintILColored(const cv::Mat& positive, const cv::Mat& negative, const cv::Mat& pR, const cv::Mat& nR,
				const std::vector<cv::Point>& bb, const cv::Point& pPos, const cv::Point& nPos, const storage::Cutline& line);

			/**
			 * \brief Spocita vahovy faktor prechodu medzi odlackami.
			 * \param line linia
			 * \param dmax maximalna vzdialenost od linie
			 * \param distance aktualna vzdialenost spracovavaneho bodu od linie
			 * \param current aktualny bod
			 * \return 
			 */
			float calculateWeightingFactor(const cv::Vec3f& line, int dmax, float distance, const cv::Point& current) const;
			/**
			 * \brief Aplikuje vahu na body odtlackov v prechode medzi pozitivnou
			 * a negativnou stranou morfovaneho odtlacku, v sedotonovom obrazku.
			 * \param p body pozitivnej strany
			 * \param n body negativnej strany
			 * \param weight vaha
			 * \return vyvazena pixelova hodnota bodu
			 */
			float calculateWeightedPixelValue(float p, float n, float weight);
			/**
			 * \brief Aplikuje vahu na body odtlackov v prechode medzi pozitivnou
			 * a negativnou stranou morfovaneho odtlacku, vo farebnom obrazku.
			 * \param p body pozitivnej strany
			 * \param n body negativnej strany
			 * \param weight vaha
			 * \return vyvazena pixelova hodnota bodu
			 */
			cv::Vec4f calculateWeightedColoredPixelValue(const cv::Vec4f& p, const cv::Vec4f& n, float weight);
			/**
			 * \brief Vyvazi okrajove body odtlacku tak aby vytvarali dojem rozmazania.
			 * \param map obrazok
			 * \param pos1 pozicia na odtlacku, z ktoreho bude jedna strana prevzata
			 * \param pos2 pozicia na odtlacku, z ktoreho bude druha strana prevzata
			 * \param r1 segmentacia odtlacku, z ktoreho bude jedna strana prevzata
			 * \param r2 segmentacia odtlacku, z ktoreho bude druha strana prevzata
			 * \return bod s vyvazenou pixelovou hodnotou
			 */
			float applyBorderWeightToPixel(const cv::Mat& map, const cv::Point& pos1, const cv::Point& pos2, const cv::Mat& r1, const cv::Mat& r2);
			/**
			 * \brief Zisti vzdialenost od okraja odtlacku.
			 * \param pos1 pozicia na odtlacku, z ktoreho bude jedna strana prevzata
			 * \param pos2 pozicia na odtlacku, z ktoreho bude druha strana prevzata
			 * \param r1 segmentacia odtlacku, z ktoreho bude jedna strana prevzata
			 * \param r2 segmentacia odtlacku, z ktoreho bude druha strana prevzata
			 * \param shifts posunutia
			 * \param cosRadius zakryvenie v ose y
			 * \param sinRadius zakryvenie v ose x
			 * \param distance vzdialenost
			 */
			void distanceInBorderRegion(const cv::Point& pos1, const cv::Point& pos2, const cv::Mat& r1, const cv::Mat& r2,
				const std::array<std::array<float, 2>, 8>& shifts, float cosRadius, float sinRadius, int& distance);
			/**
			 * \brief Posunie aktualnu poziciu na urcite miesto.
			 * \param current aktualna pozicia
			 * \param where miesto posunutia
			 * \param r segmentacia
			 */
			void shiftCurrentPosition(cv::Point& current, const cv::Point& where, const cv::Mat& r);
			
		public:
			enum Type { IMAGE_LEVEL, FEATURE_LEVEL, IMAGE_LEVEL_COLORED};
			
			// static members
			static const std::string class_name;

			// constructors
			TemplateGenerator() = default;

			// methods
			/**
			 * \brief Spusti proces pozadovaneho typu generovania.
			 * \param af zarovnany odtlacok
			 * \param f odtlacok
			 * \param type typ generovania
			 * \return morfovany odtlacok
			 */
			processing::storage::Fingerprint generate(const morphing::storage::AlignedFingerprint& af, const processing::storage::Fingerprint& f, int type);

			/**
			 * \brief Zobrazi morfovany odtlack spolu s cestou odkial bola metoda
			 * zavolana.
			 * \param f odtlacok
			 * \param trace cesta odkial bola metoda zavolana
			 */
			void display(const processing::storage::Fingerprint& fingerprint, const std::string& trace) const;
			/**
			 * \brief Zapise do suboru morfovany odtlack na definovane miesto urcene
			 * parametrom path
			 * \param af zarovanany odtlacok
			 * \param f odtlacok
			 * \param path miesto suboru
			 */
			void write(const processing::storage::Fingerprint& fingerprint, const std::string& path) const;
			
			// static methods
			/**
			 * \brief Vytvori syfdas sablonu markantov do textoveho suboru.
			 * \param fingerprint 
			 * \return indikator vyvorenia sablony
			 */
			static bool createSyFDaSTemplate(const processing::storage::Fingerprint& fingerprint);

			// getters
			float getBackgroundColor() const { return this->backgroundColor; }

			// setters
			TemplateGenerator& setBackgroundColor(const float color) { this->backgroundColor = color / 255.0f; return *this; }
			TemplateGenerator& setBorder(const int border) { this->border = border; return *this; }
		};
	}
}
