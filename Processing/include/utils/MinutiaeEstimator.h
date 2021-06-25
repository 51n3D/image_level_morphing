#pragma once

#include "storage/Minutiae.h"

namespace processing
{
	namespace utils
	{
		/**
		 * \brief Odhadne markanty z odlacku.
		 */
		class MinutiaeEstimator
		{
		private:
			// members
			/**
			 * \brief Ulozisko pre krokovanie tracovania linii, a vyhladavania susednych.
			 */
			cv::Mat tmp;
			/**
			 * \brief Ulozisko defaultny tracing linii.
			 */
			cv::Mat lineTracking;
			/**
			 * \brief Ulozisko pre markanty.
			 */
			std::vector<storage::Minutiae> minutiaes;

			/**
			 * \brief Velkost spracovavaneho bloku.
			 */
			int blockSize = 11;
			/**
			 * \brief Segmentacia odtlacku.
			 */
			cv::Mat segmentation;

			// static members
			/**
			 * \brief Pocet zobrazeni v ramci jedneho behu.
			 */
			static int displayed;

			// methods
			/**
			 * \brief Spustenie procesu vyhladavania markantov.
			 * \param img stenceny obrazok odtlacku.
			 */
			void compute(const cv::Mat& img);
			/**
			 * \brief Kontrola ci nejde o oznacanie markantu na okraju odtlacku.
			 * \param pos pozicia markantu
			 * \return idikator validnosti markantu
			 */
			bool isValid(const cv::Point& pos);
			/**
			 * \brief Spocita smer markantu.
			 * \param img stenceny odtlacok
			 * \param pos pozicia markantu
			 * \param type typ markantu
			 * \param threshold dlzka tracovania
			 * \param verbose kontrolny vystup
			 * \return smer markantu
			 */
			float calculateDirection(const cv::Mat& img, const cv::Point& pos, int type, float threshold = 20, bool verbose = false);
			/**
			 * \brief Najdze najmensi rozdiel medzi uhlami (urcenie uhlu pre zdvojenie linii).
			 * \param directions smery linii
			 * \return indexy najmensich rozdielov
			 */
			std::vector<int> findSmallestDifferenceOfAngles(const std::vector<float>& directions) const;
			/**
			 * \brief Spocita smer pre 2 body.
			 * \param offset bod ku ktoremu sa pocita
			 * \param center bod z ktoreho sa pocita
			 * \return 
			 */
			float direction(const cv::Point& offset, const cv::Point& center) const;
			/**
			 * \brief Urci startovacie tracovacie pozicie. 
			 * \param base pociatocny bod
			 * \param map obrazok
			 * \param except bod na vynechanie
			 * \param verbose kontrolny vystup
			 * \return startovacie body tracovania
			 */
			std::vector<cv::Point> getStartingTracingPositions(const cv::Point& base, const cv::Mat& map, const cv::Point& except = cv::Point(-1, -1), bool verbose = false);
			/**
			 * \brief Tracovanie linie stenceneho odtlacku
			 * \param base pociatocny bod
			 * \param positions startovacie pozicie
			 * \param length dlzka tracovania
			 * \param map stenceny obrazok
			 * \param processed sem sa ulozi vektorova reprezentacia tracovanej linie 
			 * \param verbose kontrolny vystup
			 * \return okrajove body tracovania linie
			 */
			std::vector<cv::Point> trace(const cv::Point& base, std::vector<cv::Point> positions, int length, const cv::Mat& map, std::vector<cv::Point>& processed, bool verbose = false);
			/**
			 * \brief Spocita adaptivnu hodnotu dlzky tracovania (trashold) na zaklade vzdialenosti
			 * subeznych linii.
			 * \param map stenceny odtlacok 
			 * \param pos pozicia markantu
			 * \param direction prvotny smer marakntu na zaklade defaultnejdlzky tracovania
			 * \param verbose kontrolny vystup
			 * \return adaptivna dlzka tracovania urcena pre markant
			 */
			float calculateAdaptiveThreshold(const cv::Mat& map, const cv::Point& pos, float direction, bool verbose = false);
			/**
			 * \brief Skontroluje ci sa v susednych bodoch pos nachadza linia.
			 * \param map stenceny obrazok
			 * \param pos pozicia markantu
			 * \param bound vzdialenost od markantu
			 * \param direction smer v ktorom sa prehladava 
			 * \param ridge ulozisko pozicie susednej linie
			 * \param line vektorova reprezenctacia tracovanej linie
			 * \return indikator uspesnosti najdenia linie
			 */
			bool locationContainsRidge(const cv::Mat& map, const cv::Point& pos, int bound, float direction, cv::Point& ridge, const std::vector<cv::Point>& line) const;
			/**
			 * \brief Najde najblizsiu vzdialenost k susednej linie.
			 * \param map stenceny obrazok
			 * \param pos pozicia markantu
			 * \param ridge pozicia na susednej linii
			 * \return 
			 */
			float lookForClosest(const cv::Mat& map, const cv::Point& pos, cv::Point& ridge) const;
			/**
			 * \brief Zisti vzdialenost dvoch bodov.
			 * \param p1 bod 1
			 * \param p2 bod 2
			 * \return vzdielenost bodov
			 */
			float distance(const cv::Point& p1, const cv::Point& p2) const;

		public:
			// static members
			static const std::string class_name;
			/**
			 * \brief Indexy susednych bodov.
			 */
			static const std::array<std::array<int, 2>, 8> indices;
			
			// constructors
			MinutiaeEstimator() = default;
			
			// methods
			/**
			 * \brief Spusti vyhladavanie markantov a nakonfiguruje potrebne parametre.
			 * \param img stenceny obrazok
			 * \return najdene markanty
			 */
			std::vector<storage::Minutiae> estimate(const cv::Mat& img);

			cv::Mat getMinutiaeImage(const cv::Mat& img, const std::vector<storage::Minutiae>& minutiaes) const;

			/**
			 * \brief Zobrazi markanty spolu s cestou odkial bola metoda zavolana.
			 * \param img odtlacok
			 * \param minutiaes markanty
			 * \param trace cesta odkial bola metoda zavolana
			 */
			void display(const cv::Mat& img, const std::vector<storage::Minutiae>& minutiaes, const std::string& trace) const;
			/**
			 * \brief Zapise markanty do suboru na definovaej ceste path.
			 * \param img obrazok
			 * \param minutiaes markanty
			 * \param path cesta
			 * \param filename nazov suboru
			 */
			void write(const cv::Mat& img, const std::vector<storage::Minutiae>& minutiaes, const std::string& path, const std::string& filename) const;

			// getters
			cv::Mat getSegmentation() const { return this->segmentation; }
			int getBlockSize() const { return this->blockSize; }
			cv::Mat geMinutiaeTracing() const { return this->tmp; }

			// setters
			MinutiaeEstimator& setSegmentation(const cv::Mat& segmentation) { this->segmentation = segmentation; return *this; }
			MinutiaeEstimator& setBlockSize(const int blockSize) { this->blockSize = blockSize; return *this; }
			
		};
	}
}
