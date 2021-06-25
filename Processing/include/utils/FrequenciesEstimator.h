#pragma once

#include "storage/RegionMask.h"

#include <opencv2/opencv.hpp>

namespace processing
{
	namespace utils
	{
		/**
		 * \brief Trieda urcena na odhad lokalnych frekvencii odlacku.
		 * Behom odhadu frekvencii, je bocnym produktom aj regionalna
		 * mask odtlacku.
		 */
		class FrequenciesEstimator final
		{
		private:
			// members
			/**
			 * \brief Velkost spracovavaneho bloku.
			 */
			int blockSize = 8;
			/**
			 * \brief Velkost orientovaneho okna.
			 */
			int windowSize = 16;

			/**
			 * \brief Lokalne orientacie.
			 */
			cv::Mat orientations;
			/**
			 * \brief Regionalna maska.
			 */
			RegionMask regionMask;
			
			/**
			 * \brief Indikator interpolacie.
			 */
			bool interpolation = false;

			/**
			 * \brief Lokalne frekvencie.
			 */
			cv::Mat frequencies;
			/**
			 * \brief Pomocne ulozisko pre frekvencie.
			 */
			cv::Mat tmpFrequencies;

			/**
			 * \brief Indikator vizualnych vystupov.
			 */
			bool verboseOutput = false;

			/**
			 * \brief Maximalna frekvencia.
			 */
			float maxF = -1;
			/**
			 * \brief Minimalna frekvencia.
			 */
			float minF = INFINITY;

			// static members
			/**
			 * \brief Pocet zobrazeni frekvencii.
			 */
			static int displayed;

			// methods
			/**
			 * \brief Extrahuje frekvencie z odtlacku.
			 * \param normalizovany odtlacok
			 */
			void compute(const cv::Mat& fingerprint);
			/**
			 * \brief Odhadne x-signaturu na pozicii [i,j].
			 * \param i pozicia i
			 * \param j pozicia j
			 * \param fingerprint odtlacok 
			 * \return sinusoidu
			 */
			std::vector<float> computeXSignature(int i, int j, const cv::Mat& fingerprint);
			/**
			 * \brief Najde vrcholy sinusoida.
			 * \param signal sinusoida
			 * \return 
			 */
			static std::array<std::vector<int>, 2> findPeaks(const std::vector<float>& signal);
			/**
			 * \brief Zisti ci dany bod na sinusoide moze byt vrchol.
			 * \param index pozicia
			 * \param indices pozicie
			 * \param signal sinusoida
			 * \param valley idikator priehlbiny
			 */
			static void validatePeak(int index, std::vector<int>& indices, const std::vector<float>& signal, bool valley = false);
			/**
			 * \brief Priemerna vzdialenost vrcholov.
			 * \param peaks vrcholy
			 * \return vzdialenost
			 */
			static float averagePeakDistance(const std::vector<int>& peaks);
			/**
			 * \brief Rozlozi frekvenciu na mapu na pozicii [i, j].
			 * \param map mapa
			 * \param i pozicia i
			 * \param j pozicia j
			 * \param freq frekvencia
			 */
			void placeFrequency(cv::Mat& map, int i, int j, float freq) const;
			/**
			 * \brief Zisti ci ide o maximalnu/minimalnu frekvenciu
			 * na pozicii [i, j].
			 * \param i pozicia i
			 * \param j pozicia j
			 */
			void maxMinFrequency(int i, int j);
			/**
			 * \brief Interpoluje frekvencie, ktore sa nepodarilo odhadnut.
			 * \param fingerprint obrazok odtlacku
			 */
			void interpolateFrequencies(const cv::Mat& fingerprint);
			/**
			 * \brief Zisti informacie o prehladavanom aktualnom bloku a ulozi ich do regionalnej masky.
			 * \param img obrazok odtlacku
			 * \param i pozicia i
			 * \param j pozicia j
			 * \param frequency frekvencia
			 * \param peakDistance vzdialenost vrcholov
			 * \param peaks vrcholy
			 * \param signal sinusoida
			 */
			void addBlockInfoToRegionMask(const cv::Mat& img, int i, int j, float frequency, float peakDistance, 
				const std::array<std::vector<int>, 2>& peaks, const std::vector<float>& signal);
			/**
			 * \brief Aplikuje inofrm=acie o regióne na frekvencie.
			 */
			void applyRegionMask();

		public:
			// static members
			static const std::string class_name;
			
			// constructors
			FrequenciesEstimator() = default;
			FrequenciesEstimator(const cv::Size& size, int type);

			// methods
			/**
			 * \brief Odhadne frekencie a odtlacku.
			 * \param fingerprint odtlacok
			 * \return frekvencie
			 */
			cv::Mat estimate(const cv::Mat& fingerprint);
			
			/**
			 * \brief Zobrazi frekvencie spolu s cestou odkial bola metoda zavolana.
			 * \param frequencies frekvencie
			 * \param trace cesta odkial bola metoda volana
			 */
			void display(const cv::Mat& frequencies, const std::string& trace) const;
			/**
			 * \brief Zapise frekvencie do suboru v danej ceste.
			 * \param frequencies frekvencie
			 * \param path cesta 
			 */
			void write(const cv::Mat& frequencies, const std::string& path) const;

			// getters
			int getBlockSize() const { return this->blockSize; }
			int getWindowSize() const { return this->windowSize; }
			cv::Mat getFrequencies() const { return this->frequencies; }
			cv::Mat getOrientations() const { return this->orientations; }
			bool isInterpolated() const { return this->interpolation; }
			float getMaxFrequency() const { return this->maxF; }
			float getMinFrequency() const { return this->minF; }
			bool isVerbose() const { return this->verboseOutput; }
			RegionMask getRegionMask() const { return this->regionMask; }
			
			// setters
			FrequenciesEstimator& setBlockSize(const int blockSize) { this->blockSize = blockSize; return *this; }
			FrequenciesEstimator& setOrientations(const cv::Mat& orientations) { this->orientations = orientations; return *this; }
			FrequenciesEstimator& setWindowSize(const int windowSize) { this->windowSize = windowSize; return *this; }
			FrequenciesEstimator& interpolate(const bool interpolation = true) { this->interpolation = interpolation; return  *this; }
			FrequenciesEstimator& verbose(const bool verboseOutput = true) { this->verboseOutput = verboseOutput; return *this; }

		};
	}
}
