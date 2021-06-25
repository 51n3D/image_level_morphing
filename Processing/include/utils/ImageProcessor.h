#pragma once

#include <opencv2/opencv.hpp>

#include <string>

namespace processing
{
	namespace utils
	{
		/**
		 * \brief Kontainer metod zakladnych pixelovych operacii.
		 */
		class ImageProcessor
		{
		private:
			/**
			 * \brief Iteracia stencovania.
			 * \param img obrazok binarizovaneho odtlacku
			 * \param iter poradie iteracie
			 */
			static void thinningIteration(cv::Mat& img, int iter);

		public:
			/**
			 * \brief Pristup k pixelu obrazku, pripade ze sa pozicia nenachadza na obrazku
			 * vrati def.
			 * \param obj obrazok
			 * \param i pozicia i
			 * \param j pozicia j
			 * \param def defaultna hodnota
			 * \return hodnota pixelu obrazku
			 */
			static float at(const cv::Mat& obj, int i, int j, float def);
			/**
			 * \brief Nacita obrazok zo suboru.
			 * \param imageLocation cesta k obrazku
			 * \param flags flags
			 * \return obrazok nacitany zo suboru
			 */
			static cv::Mat read(const std::string& imageLocation, int flags = 0);
			/**
			 * \brief Prevedie obrazok dotavej struktury lubovolneho typu.
			 * \param img obrazok
			 * \param type typ do ktoreho bude obrazok prevedeny
			 */
			static void convertTo(cv::Mat& img, int type);
			/**
			 * \brief Prevedie sedotonovy obrazok do farebneho s lubovolnym typom.
			 * \param grey sedotonovy obrazok
			 * \param colored ulozisko pre farebny obrazok
			 * \param type typ do ktoreho sa bude prevadzat
			 */
			static void convertTo(const cv::Mat& grey, cv::Mat& colored, int type);
			/**
			 * \brief Prekopiruje cv::Mat do inej cv::Mat 
			 * \param input vstup
			 * \param output vystup
			 */
			static void copyTo(const cv::Mat& input, cv::Mat& output);
			/**
			 * \brief Zmeni velkost obrazku na zaklade percent.
			 * \param img obrazok
			 * \param scale percentualne zvatsenie
			 */
			static void resize(cv::Mat& img, double scale);
			/**
			 * \brief Zmeni velkost obrazku na urcitu velkost.
			 * \param img obrazok
			 * \param height vyska
			 */
			static void resize(cv::Mat& img, int height);
			/**
			 * \brief Normalizuje obrazok.
			 * \param img obrazok
			 * \return normalizovany obrazok
			 */
			static cv::Mat normalize(const cv::Mat& img);
			/**
			 * \brief Rozmaze hodnoty vstupnej matice img.
			 * \param img matica
			 * \param kernelSize velkost jadra 
			 * \param deviation odchylka
			 * \param border okrajove hodnoty
			 * \return rozmazana matica
			 */
			static cv::Mat getBlurred(const cv::Mat& img, int kernelSize, int deviation = 0, int border = cv::BORDER_DEFAULT);
			/**
			 * \brief Binarizuje obrazok odtlacku.
			 * \param img obrazok
			 * \param segmentation segmentacia 
			 */
			static void binarize(cv::Mat& img, const cv::Mat& segmentation);
			/**
			 * \brief Zvysi kontrast linii.
			 * \param img obrazok
			 */
			static void higherContrast(cv::Mat& img);
			/**
			 * \brief Oreze obrazok na zaklade velkost regionu popredia odtlacku.
			 * \param img obrazok
			 * \param region region popredia
			 */
			static void trim(cv::Mat& img, const std::vector<cv::Point>& region);
			/**
			 * \brief Otoci obrazok o x stupnov.
			 * \param img obrazok
			 * \param angle uhol v stupnoch
			 * \param identity segmentacia odtlacku
			 * \param fillBackground idikator vyplne pozadia
			 * \return otoceny obrazok.
			 */
			static cv::Mat rotate(const cv::Mat& img, float angle, const std::vector<cv::Point>& identity = std::vector<cv::Point>(), bool fillBackground = false);
			/**
			 * \brief Ztensi obrazok binarizovaneho odtlacku prsta.
			 * \param img obrazok
			 * \param segmentation segmentacia 
			 */
			static void thine(cv::Mat& img, const cv::Mat& segmentation);
			/**
			 * \brief Opravi zle ohodnotene oblasti, kde doslo k chybnej segemntacii.
			 * \param segmentation segmentacia odtlacku
			 * \param blockSize velkost bloku
			 * \return 
			 */
			static std::vector<cv::Point> correctSegmentation(cv::Mat& segmentation, int blockSize);
			/**
			 * \brief Doda obrazku nadych urcitej farby.
			 * \param img obrazok
			 * \param color farba
			 */
			static void addColorBreathToImage(cv::Mat& img, const cv::Scalar& color);
		};
	}
}
