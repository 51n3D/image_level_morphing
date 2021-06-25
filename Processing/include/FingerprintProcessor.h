#pragma once

#include "utils/OrientationsEstimator.h"
#include "utils/FrequenciesEstimator.h"
#include "utils/GaborFilter.h"
#include "utils/MinutiaeEstimator.h"
#include "utils/FakeMinutiaeDetector.h"

#include <string>
	
namespace processing
{
	namespace storage
	{
		class Fingerprint;
	}
	
	/**
	 * \brief Top-level ovladac. Trieda je urcenia na zabuzdrenie cinnosti
	 * jednotlivych nastrajov a pritup k datam objektu Fingerprint.
	 */
	class FingerprintProcessor
	{
	private:
		// members
		/**
		 * \brief Nastroj na extrakciu lokalnych orientacii.
		 */
		utils::OrientationsEstimator orientations;
		/**
		 * \brief Nastroj na extrakciu lokalnych frekvencii
		 */
		utils::FrequenciesEstimator frequencies;
		/**
		 * \brief Nastroj na vylepsenie kvality odtlacku.
		 */
		utils::GaborFilter filter;
		/**
		 * \brief Nastroj na identifikaciu markantov
		 */
		utils::MinutiaeEstimator minutiaes;
		/**
		 * \brief Nastroj na odfiltrovanie falosnych markantov
		 */
		utils::FakeMinutiaeDetector detector;

		// static members
		/**
		 * \brief Pocet zobrazeni v jednom behu.
		 */
		static int displayed;
		/**
		 * \brief Pocet zobrazeni normalizovaneho odtlacku v jednom behu.
		 */
		static int displayedN;
		/**
		 * \brief Pocet zobrazi vylepseneho odtlacku v jednom behu.
		 */
		static int displayedE;

		// methods
		/**
		 * \brief Zobrazi obrazok s cestou odkial bola metoda zavolana. 
		 * \param img obrazok
		 * \param trace cesta odkial bola metoda volana
		 */
		void display(const cv::Mat& img, const std::string& trace) const;
		/**
		 * \brief Prida k orientaciam informaciu o regione.
		 * \param fingerprint odtlacok
		 */
		void correctOrientations(storage::Fingerprint& fingerprint);
		
	public:
		// static members
		/**
		 * \brief Pre testovacie ucely, cesta k miestu kde ma odtlacky hladat.
		 */
		static std::string path;
		
		// constructors
		FingerprintProcessor(const utils::OrientationsEstimator& orientations, const utils::FrequenciesEstimator& frequencies, 
			const utils::GaborFilter& filter, const utils::MinutiaeEstimator& minutiaes, const utils::FakeMinutiaeDetector& detector);

		// methods
		/**
		 * \brief Zmenezuje normalizaciu.
		 * \param fingerprint odtlacok
		 * \param verbose kontrolny vystup
		 */
		void normalize(storage::Fingerprint& fingerprint, bool verbose = false) const;
		/**
		 * \brief Zmenezuje odhad orientacii.
		 * \param fingerprint odtlacok
		 * \param verbose kontrolny vystup
		 */
		void estimateOrientations(storage::Fingerprint& fingerprint, bool verbose = false);
		/**
		 * \brief Zmenezuje extrakciu frekvencii.
		 * \param fingerprint odtlacok
		 * \param verbose kontrolny vystup
		 */
		void estimateFrequencies(storage::Fingerprint& fingerprint, bool verbose = false);
		/**
		 * \brief Aplikuje regionalnu masku na extrahovane vlastnosti.
		 * \param fingerprint odtlacok
		 * \param verbose kontrolny vystup
		 */
		void applyRegionMask(storage::Fingerprint& fingerprint, bool verbose = false);
		/**
		 * \brief Zmenezuje identifikaciu markantov. Zahrna aj vsetky kroky od vylepsenia
		 * kvality.
		 * \param fingerprint odtlacok
		 * \param verbose kontrolny vystup
		 */
		void findMinutiaes(storage::Fingerprint& fingerprint, bool verbose = false);
		/**
		 * \brief Zmenezuje odfiltrovanie falosnych markantov.
		 * \param fingerprint odtlacok
		 */
		void handleFakeMinutiaes(storage::Fingerprint& fingerprint) const;
		/**
		 * \brief Zmenezuje odhad markantov s kontrolnym vystupom do suboru.
		 * \param fingerprint odtlacok
		 * \param verbose kontrolny vystup
		 */
		void findMinutiaesW(storage::Fingerprint& fingerprint, bool verbose = false);
		
		/**
		 * \brief Zmenezuje vylepsenie kvality odtlacku.
		 * \param fingerprint odtlacok
		 * \param verbose kontrolny vystup 
		 */
		void filterFingerprint(storage::Fingerprint& fingerprint, bool verbose = false);
		/**
		 * \brief Zmenezuje binarizaciu odtlacku.
		 * \param fingerprint odtlacok
		 */
		static void binarize(storage::Fingerprint& fingerprint);
		/**
		 * \brief Zmenezuje ztensenie linii odtlacku.
		 * \param fingerprint odtlacok
		 */
		static void thinning(storage::Fingerprint& fingerprint);
		/**
		 * \brief Zmenezuje odhad markantov zo stenceneho odtlacku.
		 * \param fingerprint odtlacok
		 * \param verbose kontrolny vystup
		 */
		void estimateMinutiaes(storage::Fingerprint& fingerprint, bool verbose = false);

		/*
		 * Metody zmenezuju ziskanie obrazkov extrahovanych informacii.
		 * Prevolanie je intuitivne podla nazvov metod vzhladom na clenov triedy.
		 */
		cv::Mat getOrientationsImage(const storage::Fingerprint& fingerprint);
		cv::Mat getFrequenciesImage(const storage::Fingerprint& fingerprint);
		cv::Mat getEnhancedImage(const storage::Fingerprint& fingerprint);
		cv::Mat getThinnedImage(const storage::Fingerprint& fingerprint);
		cv::Mat getMinutiaeImage(const storage::Fingerprint& fingerprint);
		cv::Mat getMinutiaeImage(const cv::Mat& fingerprint, const std::vector<utils::storage::Minutiae>& minutiae);
		cv::Mat getMinutiaeTracing(const storage::Fingerprint& fingerprint);
		
		/*
		 * Metody zmenezuju volanie metod inych objektov.
		 * Prevolanie je intuitivne podla nazvov metod vzhladom na clenov triedy.
		 */
		void displayNormalized(const storage::Fingerprint& fingerprint, const std::string& trace) const;
		void displayOrientations(const storage::Fingerprint& fingerprint, const std::string& trace, bool blank = false) const;
		void displayFrequencies(const storage::Fingerprint& fingerprint, const std::string& trace) const;
		void displayFiltered(const storage::Fingerprint& fingerprint, const std::string& trace) const;
		void displayBinarized(const storage::Fingerprint& fingerprint, const std::string& trace) const;
		void displayThinned(const storage::Fingerprint& fingerprint, const std::string& trace) const;
		void displayMinutiaes(const storage::Fingerprint& fingerprint, const std::string& trace) const;

		/* Metody pre zapis informacii o odtlacku suboru */
		void writeNormalized(const storage::Fingerprint& fingerprint, const std::string& filename) const;
		void writeOrientations(const storage::Fingerprint& fingerprint, const std::string& filename) const;
		void writeFrequencies(const storage::Fingerprint& fingerprint, const std::string& filename) const;
		void writeEnhanced(const storage::Fingerprint& fingerprint, const std::string& filename) const;
		void writeThinning(const storage::Fingerprint& fingerprint, const std::string& filename) const;
		void writeMinutiaes(const storage::Fingerprint& fingerprint, const std::string& filename) const;
		static void write(const cv::Mat& img, const std::string& path);
		
		// static methods
		/**
		 * \brief Adaptuje objekt Fingerprint, tzn. prisposobi ho k potrebam tejto kniznice.
		 * \param fingerprint odtlacok
		 * \param blockSize velkost bloku
		 * \param windowSize velkost orientovaneho okna
		 * \param trashHold prah
		 */
		static void adapt(storage::Fingerprint& fingerprint, int blockSize, int windowSize, float trashHold);
		
		// static methods
		/**
		 * \brief Vytvori instanciu odtlacku, na zaklade obrazku predaneho parametrom.
		 * \param fingerprintImg obrazok
		 * \return objekt odtlacku prstu
		 */
		static storage::Fingerprint getFingerprint(const cv::Mat& fingerprintImg);

	};
}
