#pragma once

#include <storage/Minutiae.h>

#include <opencv2/core/matx.hpp>
#include <vector>

namespace morphing
{
	namespace utils
	{
		namespace storage
		{
			/**
			 * \brief Trieda sluzi ako ulozisko informacii o morfovani.
			 */
			class Cutline : public cv::Vec3f
			{
			private:
				// members
				/**
				 * \brief Markanty pozitivnej strany odtlacku.
				 */
				std::vector<processing::utils::storage::Minutiae> posFMin;
				/**
				 * \brief Markanty negativnej strany odtlacku.
				 */
				std::vector<processing::utils::storage::Minutiae> negFMin;
				/**
				 * \brief Markanty pozitivnej strany zarovnaneho odtlacku.
				 */
				std::vector<processing::utils::storage::Minutiae> posAfMin;
				/**
				 * \brief Markanty negativnej strany zarovnaneho odtlacku.
				 */
				std::vector<processing::utils::storage::Minutiae> negAfMin;

				float sO;
				float sV;
				float sM;
				float eval;

				/**
				 * \brief Typ rozdelenia
				 */
				int separationType = NONE;
				/**
				 * \brief Maximalna vzdialenost od reznej linie.
				 */
				int dmax = 32;

			public:
				enum SeparationType { NONE = -1, POS_NEG = 1, NEG_POS };
				
				// constructors
				Cutline() = default;
				explicit Cutline(const cv::Vec3f& line) : cv::Vec3f(line) {}

				// getters
				int getDMax() const { return this->dmax; }
				std::vector<processing::utils::storage::Minutiae> getPosFMin() const { return this->posFMin; }
				std::vector<processing::utils::storage::Minutiae> getNegFMin() const { return this->negFMin; }
				std::vector<processing::utils::storage::Minutiae> getPosAfMin() const { return this->posAfMin; }
				std::vector<processing::utils::storage::Minutiae> getNegAfMin() const { return this->negAfMin; }
				int getSeparation() const { return this->separationType; }

				/*
				 * Metody urcene pre ziskanie kardinality marknatov na jednotlivych
				 * stranach odtlackov
				 */
				int getPosFCardinality() const { return this->posFMin.size(); }
				int getNegFCardinality() const { return this->negFMin.size(); }
				int getPosAfCardinality() const { return this->posAfMin.size(); }
				int getNegAfCardinality() const { return this->negAfMin.size(); }
				float getSO() const { return this->sO; }
				float getSV() const { return this->sV; }
				float getSM() const { return this->sM; }
				float getEval() const { return this->eval; }
				
				//setters
				Cutline& setDMax(const int dmax) { this->dmax = dmax; return *this; }
				Cutline& setPosFMin(const std::vector<processing::utils::storage::Minutiae>& mins) { this->posFMin = mins; return *this; }
				Cutline& setNegFMin(const std::vector<processing::utils::storage::Minutiae>& mins) { this->negFMin = mins; return *this; }
				Cutline& setPosAfMin(const std::vector<processing::utils::storage::Minutiae>& mins) { this->posAfMin = mins; return *this; }
				Cutline& setNegAfMin(const std::vector<processing::utils::storage::Minutiae>& mins) { this->negAfMin = mins; return *this; }
				Cutline& setSO(const float sO) { this->sO = sO; return *this; }
				Cutline& setSV(const float sV) { this->sV = sV; return *this; }
				Cutline& setSM(const float sM) { this->sM = sM; return *this; }
				Cutline& setEval(const float eval) { this->eval = eval; return *this; }
				Cutline& setSeparation(const float fAf, const float afF) { this->separationType = (fAf >= afF) ? SeparationType::POS_NEG : SeparationType::NEG_POS; return *this; }

				/*
				 * Metody urcene k navyseniu kardinality markantov na jednotlivych
				 * stranach odtlackov.
				 */
				Cutline& addPosFMin(const processing::utils::storage::Minutiae& min) { this->posFMin.push_back(min); return *this; }
				Cutline& addNegFMin(const processing::utils::storage::Minutiae& min) { this->negFMin.push_back(min); return *this; }
				Cutline& addPosAfMin(const processing::utils::storage::Minutiae& min) { this->posAfMin.push_back(min); return *this; }
				Cutline& addNegAfMin(const processing::utils::storage::Minutiae& min) { this->negAfMin.push_back(min); return *this; }
				
			};
		}
	}
}
