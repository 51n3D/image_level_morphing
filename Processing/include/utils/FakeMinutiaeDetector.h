#pragma once

#include "storage/Minutiae.h"

#include <opencv2/opencv.hpp>

#include <vector>

namespace processing
{
	namespace utils
	{
		/**
		 * \brief Trieda urcena k vyhladaniu a premazaniu falosnych markantov.
		 */
		class FakeMinutiaeDetector
		{
		private:
			// members
			cv::Mat tmp;
			
			// static members
			static int displayed;
			
			// methods
			/**
			 * \brief Zisti ci dvaja markanty su proti sebe stojace.
			 * \param m1 markant
			 * \param m2 markant
			 * \return ci su proti sebe stojace
			 */
			bool isFacingPair(const storage::Minutiae& m1, const storage::Minutiae& m2) const;
			/**
			 * \brief Zisti uhol medzi markantmi.
			 * \param m1 markant
			 * \param m2 markant
			 * \return uhol
			 */
			double minutiaeAngle(const storage::Minutiae& m1, const storage::Minutiae& m2) const;
			/**
			 * \brief Zisti ci sa markant m2 nachadza v okoli m1.
			 * \param m1 markant
			 * \param m2 markant
			 * \return ci je v okoli
			 */
			bool isAround(const storage::Minutiae& m1, const storage::Minutiae& m2) const;
			/**
			 * \brief Vzdialenost medzi dvoma bodmi.
			 * \param p1 bod
			 * \param p2 bod
			 * \return vzdialenost
			 */
			double distance(const cv::Point& p1, const cv::Point& p2) const;
			/**
			 * \brief Zisti ci su markanty prepojene.
			 * \param m1 markant
			 * \param m2 markant
			 * \param map mapa
			 * \return ci su prepojene
			 */
			bool areConnected(const storage::Minutiae& m1, const storage::Minutiae& m2, const cv::Mat& map) const;
			/**
			 * \brief Ziska startovacie pozicie prveho kroku pre tracing,
			 * na zaklade pozicie kde sa zacina.
			 * \param base zaciatok 
			 * \param map mapa
			 * \return startovacie pozicie tracingu
			 */
			std::vector<cv::Point> getStartingTracingPositions(const cv::Point& base, const cv::Mat& map) const;
			/**
			 * \brief Zisti dlzku tracovania podla typu markantu.
			 * \param m markant
			 * \return dlzka tracovania
			 */
			float maximalTracingLength(const storage::Minutiae& m) const;

			/**
			 * \brief Odstrani markanty vzniknute prerusenim linie.
			 * \param minutiaes markanty
			 */
			void repairBrokenRidges(std::vector<storage::Minutiae>& minutiaes) const;
			/**
			 * \brief Odstrani markanty vzniknute nespravnym prepojenim linii.
			 * \param minutiaes 
			 */
			void repairForks(std::vector<storage::Minutiae>& minutiaes) const;
			/**
			 * \brief Odstrani markanty vytvorene falosnymi vybezkami a prepojeniami.
			 * \param minutiaes markanty
			 */
			void removeBurs(std::vector<storage::Minutiae>& minutiaes) const;
			
			/**
			 * \brief Zisti kolko maximalne cely markantov typu m1 oproti typu m2.
			 * \param minutiaes markanty
			 * \param m1 typ markantu
			 * \param m2 typ markantu
			 * \return maximalny pocet celiacich markantov
			 */
			int maxFacingPairs(std::vector<storage::Minutiae>& minutiaes, int m1, int m2) const;
			/**
			 * \brief Najvacsi uhol celiach markantov.
			 * \param minutiae markant
			 * \param facing celiace markanty
			 * \return maximalny uhol
			 */
			int maxBetaAt(const storage::Minutiae& minutiae, const std::vector<storage::Minutiae>& facing) const;

			/**
			 * \brief Zmaze markanty urcene k zmazaniu z markantov.
			 * \param minutiaes markanty
			 * \param minutiaesToDelete markanty urcene k zmazaniu
			 * \return novy iterator na vektor marakantov
			 */
			std::_Vector_iterator<std::_Vector_val<std::_Simple_types<storage::Minutiae>>>
				handleRemove(std::vector<storage::Minutiae>& minutiaes, const std::vector<storage::Minutiae>& minutiaesToDelete) const;
			/**
			 * \brief Zmaze jeden markant z markantov
			 * \param minutiaes markanty
			 * \param minutiaeToDelete markant k zmazaniu
			 * \return novy iterator na vektor marakantov
			 */
			std::_Vector_iterator<std::_Vector_val<std::_Simple_types<storage::Minutiae>>>
				handleRemove(std::vector<storage::Minutiae>& minutiaes, const storage::Minutiae& minutiaeToDelete) const;
			/**
			 * \brief Premaze registrovane falosne struktry o markanty urcene k zmazaniu.
			 * \param minutiaes markanty
			 * \param minutiaesToDelete markanty urcene k zmazaniu
			 */
			void removeFromStructures(std::vector<storage::Minutiae>& minutiaes, const std::vector<storage::Minutiae>& minutiaesToDelete) const;
			/**
			 * \brief Premaze registrovane falosne struktry o markant urceny k zmazaniu.
			 * \param minutiaes markanty
			 * \param minutiaesToDelete markant urcene k zmazaniu
			 */
			void removeFromStructures(std::vector<storage::Minutiae>& minutiaes, const storage::Minutiae& minutiaeToDelete) const;
			
		public:
			// static members
			static const std::string class_name;
			
			// constructors
			FakeMinutiaeDetector() = default;

			// methods
			/**
			 * \brief Zomrazi markanty urcene k zmazaniu.
			 * \param img obrazok odtlacku
			 * \param minutiaes markanty
			 * \param trace miesto odkial bola metoda volana
			 */
			void display(const cv::Mat& img, std::vector<storage::Minutiae> minutiaes, const std::string& trace) const;
			
			// methods
			/**
			 * \brief Najde falosne markanty.
			 * \param img mapa odtlacku
			 * \param minutiaes markanty
			 */
			void find(const cv::Mat& img, std::vector<storage::Minutiae>& minutiaes) const;
			/**
			 * \brief Odstrani falosne markanty.
			 * \param minutiaes markanty
			 */
			void remove(std::vector<storage::Minutiae>& minutiaes) const;
			
		};
	}
}
