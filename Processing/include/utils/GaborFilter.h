#pragma once

#include <opencv2/opencv.hpp>

namespace processing
{
	namespace utils
	{
		class Orientations;
		class Frequencies;
		
		/**
		 * \brief Trieda vylepsi kvaltu odtlacku pomocou Gaborovho
		 * filtra.
		 */
		class GaborFilter
		{
		private:
			// members
			/**
			 * \brief Velkost spracovavaneho bloku.
			 */
			int blockSize = 16;
			/**
			 * \brief Odchylka.
			 */
			float deviation = 4;

			/**
			 * \brief Indikator vizualnych vystupov.
			 */
			bool verboseOutput = false;
			
			/**
			 * \brief Lokalne frekvencie odtlacku.
			 */
			cv::Mat frequencies;
			/**
			 * \brief Lokalne orientacie odtlacku.
			 */
			cv::Mat orientations;
			/**
			 * \brief Vylepseny odtlacok.
			 */
			cv::Mat filtered;

			// static members
			/**
			 * \brief Pocet zobrazeni vylepseneho odtlacku.
			 */
			static int displayed;

			//methods
			/**
			 * \brief Vylepsi kvalitu odtlacku.
			 */
			void filterImage();
			/**
			 * \brief Vytiahne blok z obrazku na pozicii [i, j].
			 * \param fingerprint odtlacok
			 * \param i pozicia i
			 * \param j pozicia j
			 * \return blok
			 */
			cv::Mat blockAt(const cv::Mat& fingerprint, int i, int j) const;
			/**
			 * \brief Nakopiruje blok do obrazku na pozicii [i, j]
			 * \param block blok
			 * \param fingerprint obrazok
			 * \param i pozicia i
			 * \param j pozicia j
			 */
			void blockCopyTo(const cv::Mat& block, cv::Mat& fingerprint, int i, int j) const;
			/**
			 * \brief Zfiltruje blok povodneho obrazku gaborovym filtrom.
			 * \param block blok
			 * \param orientation lokalna orientacia 
			 * \param frequency lokalna frekvencia
			 */
			void filterBlock(cv::Mat& block, float orientation, float frequency) const;
			/**
			 * \brief Zisti ci je blok na pozicii [i, j] filtrovatelny.
			 * \param i pozicia i
			 * \param j pozicia j
			 * \return indikator filtrovatelnosti
			 */
			bool isFilterable(int i, int j);
			
		public:
			// static members
			static const std::string class_name;
			
			// constructors
			GaborFilter() = default;

			// methods
			/**
			 * \brief Vylepsi kvalitu odtlacku.
			 * \param fingerprint obrazok
			 * \return 
			 */
			cv::Mat filter(const cv::Mat& fingerprint);

			/**
			 * \brief Zobrazi frekvencie spolu s cestou odkial bola metoda zavolana.
			 * \param frequencies frekvencie
			 * \param trace cesta odkial bola metoda volana
			 */
			void display(const cv::Mat& fingerprint, const std::string& trace) const;
			/**
			 * \brief Zapise frekvencie do suboru v danej ceste.
			 * \param frequencies frekvencie
			 * \param path cesta
			 */
			void write(const cv::Mat& fingerprint, const std::string& path) const;
				
			// getters
			float getDeviation() const { return this->deviation; }
			cv::Mat getFrequencyMap() const { return this->frequencies; }
			cv::Mat getOrientationMap() const { return this->orientations; }
			int getBlockSize() const { return this->blockSize; }
			cv::Mat getFiltered() const { return this->filtered; }
			bool isVerbose() const { return this->verboseOutput; }
			
			// setters
			GaborFilter& setDeviation(const float deviation) { this->deviation = deviation; return *this; }
			GaborFilter& setFrequencyMap(const cv::Mat& frequencies) { this->frequencies = frequencies; return *this; }
			GaborFilter& setOrientationMap(const cv::Mat& orientations) { this->orientations = orientations; return *this; }
			GaborFilter& setBlockSize(const int blockSize) { this->blockSize = blockSize; return *this; }
			GaborFilter& verbose(const bool verboseOutput = true) { this->verboseOutput = verboseOutput; return *this; }
			
		};
	}
}
