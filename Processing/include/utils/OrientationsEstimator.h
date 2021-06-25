#pragma once

#include <opencv2/opencv.hpp>

namespace processing
{
	namespace utils
	{
		/**
		 * \brief Trieda urcena pre odhad lokalnych orientacii z odtlacku.
		 */
		class OrientationsEstimator
		{
		private:
			// members
			/**
			 * \brief Velkost spracovavaneho bloku.
			 */
			int blockSize = 16;
			/**
			 * \brief Velkost jadra pre filter.
			 */
			int kernelSize = 3;

			/**
			 * \brief Orientacie dotlacku.
			 */
			cv::Mat orientations;
			/**
			 * \brief Gradient odtlacku X.
			 */
			cv::Mat gradX;
			/**
			 * \brief Gradient odtlacku Y.
			 */
			cv::Mat gradY;

			/**
			 * \brief Indikator pouzitia dolnopriepustneho filtra.
			 */
			bool lowPassFilter = false;
			/**
			 * \brief Indikator kontrolneho vystup.
			 */
			bool verboseOutput = false;

			// static members
			/**
			 * \brief Pocet zobrazeni v ramci jedneho behu.
			 */
			static int displayed;

			// methods
			/**
			 * \brief Ohadne lokalne orientacie odtlacku.
			 * \param fingerprint normalizovany odtlacok
			 */
			void compute(const cv::Mat& fingerprint);
			/**
			 * \brief Zisti gradient odtlacku.
			 * \param fingerprint normalizovany odtlaock
			 */
			void computeGradients(const cv::Mat& fingerprint);
			/**
			 * \brief Zfiltruje orientacie dolnopriepustnym filtrom.
			 * \param phiX x-ove suradnice orientacii prevedenych na vektor 
			 * \param phiY y-ove suradnice orientacii prevedenych na vektor 
			 */
			void filterOrientations(cv::Mat& phiX, cv::Mat& phiY);

		public:
			// static members
			static const std::string class_name;
			
			// constructors
			OrientationsEstimator() = default;

			// methods
			/**
			 * \brief Zahaji odhad lokalnych orientacii s pociatocnou konfiguraciou.
			 * \param fingerprint normalizovany odtlacok
			 * \return lokalne orientacie odtlacku
			 */
			cv::Mat estimate(const cv::Mat& fingerprint);
			
			/**
			 * \brief Zobrazi lokalne orientacie s cestou odkial bola metoda zavolana.
			 * \param fingerprint odtlacok
			 * \param orientations orientacie
			 * \param trace miesto okdial bola metoda zavolana
			 * \param blank idikator vyplne pozadia
			 */
			void display(const cv::Mat& fingerprint, const cv::Mat& orientations, const std::string& trace, bool blank = false) const;
			/**
			 * \brief Zapise do suboru lokalne orientacie s cestou odkial bola metoda zavolana.
			 * \param fingerprint odtlacok
			 * \param orientations orientacie
			 * \param trace miesto okdial bola metoda zavolana
			 * \param blank idikator vyplne pozadia
			 */
			void write(const cv::Mat& fingerprint, const cv::Mat& orientations, const std::string& path, bool blank = false) const;

			/* Metoda ziska reprezentaciu obrazku orientacii v podobe cv::Mat struktury */
			cv::Mat getOrientationImage(const cv::Mat& fingerprint, const cv::Mat& orientations, bool blank = false) const;

			// getters
			cv::Mat getOrientationMap() const { return this->orientations; }
			int getBlockSize() const { return this->blockSize; }
			cv::Mat getGradX() const { return this->gradX; }
			cv::Mat getGradY() const { return this->gradY; }
			int getKernelSize() const { return this->kernelSize; }
			bool isVerbose() const { return this->verboseOutput; }
			
			// setters
			OrientationsEstimator& setOrientationMap(const cv::Mat& orientations) { this->orientations = orientations; return *this; }
			OrientationsEstimator& setBlockSize(const int blockSize) { this->blockSize = blockSize; return *this; }
			OrientationsEstimator& setKernelSize(const int kernelSize) { this->kernelSize = kernelSize; return  *this; }
			OrientationsEstimator& useLowPassFilter(const bool use = true) { lowPassFilter = use; return *this; }
			OrientationsEstimator& verbose(const bool verbose = true) { this->verboseOutput = verbose; return *this; }

		};
	}
}
