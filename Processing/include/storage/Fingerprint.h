#pragma once

#include "storage/Minutiae.h"
#include "storage/RegionMask.h"

#include <opencv2/opencv.hpp>

namespace processing
{
	namespace storage
	{
		/**
		 * \brief Storage for a fingerprint.
		 */
		class Fingerprint : public cv::Mat
		{

		protected:
			/**
			 * \brief Prah segmentacie.
			 */
			float trashHold = 0.9f;
			/**
			 * \brief Velkost bloku.
			 */
			int blockSize = 16;
			/**
			 * \brief Velkost orientovaneho okna.
			 */
			int windowSize = 32;
			/**
			 * \brief Pocet blokov popredia odtlacku.
			 */
			int blocks = 0;
			/**
			 * \brief Maximalna frekvencia odtlacku.
			 */
			float maxF = INFINITY;
			/**
			 * \brief Minimalna frekvencia odtlacku,
			 */
			float minF = -1;
			
			/**
			 * \brief Indikator vylepsenia odtlacku.
			 */
			bool enhanced = false;

			/**
			 * \brief Normalizovany odtlacok.
			 */
			Mat normalized;
			/**
			 * \brief Segmentacia odtlacku.
			 */
			Mat segmentation;
			/**
			 * \brief Lokalne orientacie odtlacku.
			 */
			Mat orientations;
			/**
			 * \brief Lokalne frekvencie odtlacku.
			 */
			Mat frequencies;
			/**
			 * \brief Stenceny odtlacok.
			 */
			Mat thinned;
			/**
			 * \brief Vylepseny odtlacok.
			 */
			Mat filtered;
			/**
			 * \brief Binarizovany odtlacok.
			 */
			Mat binarized;
			/**
			 * \brief Markanty odtlacku.
			 */
			std::vector<utils::storage::Minutiae> minutiaes;
			/**
			 * \brief Tracovanie markantov odtlacku.
			 */
			Mat minutiaeTracing;

			/**
			 * \brief Regionalna maska.
			 */
			utils::RegionMask regionMask;

		public:
			// members
			/**
			 * \brief Nszov odtlacku.
			 */
			std::string fingerName = "";
			
			// constructors
			Fingerprint() = default;
			explicit Fingerprint(Mat fingerprint) : Mat(std::move(fingerprint)) {};

			// methods
			/**
			 * \brief Vyprazdni odtlacok.
			 */
			void clear();
			
			// getters
			Mat get() const { return Mat(*this); }
			Mat getNormalized() const { return this->normalized; }
			Mat getSegmentation() const { return static_cast<Mat>(this->regionMask); }
			Mat getOrientations() const { return this->orientations; }
			Mat getFrequencies() const { return this->frequencies; }
			Mat getFiltered() const { return this->filtered; }
			Mat getBinarized() const { return this->binarized; }
			Mat getThinned() const { return this->thinned; }
			int getBlockSize() const { return this->blockSize; }
			int getWindowsSize() const { return this->windowSize; }
			bool isEnhanced() const { return this->enhanced; }
			std::vector<utils::storage::Minutiae> getMinutiae() const { return this->minutiaes; }
			int getBlocks();
			float getMaxF() const { return this->maxF; }
			float getMinF() const { return this->minF; }
			float getTrashHold() const { return this->trashHold; }
			utils::RegionMask getRegionMask() const { return this->regionMask; }
			Mat getMinutiaeTracing() const { return this->minutiaeTracing; }

			// setters
			Fingerprint& setNormalized(const Mat& normalized) { this->normalized = normalized; return *this; }
			Fingerprint& setSegmentation(const Mat& segmentation) { this->segmentation = segmentation; return *this; }
			Fingerprint& setOrientations(const Mat& orientations) { this->orientations = orientations; return *this; }
			Fingerprint& setFrequencies(const Mat& frequencies) { this->frequencies = frequencies; return *this; }
			Fingerprint& setFiltered(const Mat& filtered) { this->filtered = filtered; return *this; }
			Fingerprint& setBinarized(const Mat& binarized) { this->binarized = binarized; return *this; }
			Fingerprint& setThinned(Mat& thinned) { this->thinned = thinned; return *this; }
			Fingerprint& setBlockSize(const int blockSize) { this->blockSize = blockSize; return *this; }
			Fingerprint& setWindowSize(const int windowSize) { this->windowSize = windowSize; return *this; }
			Fingerprint& setEnhanced(const bool enhanced) { this->enhanced = enhanced; return *this; }
			Fingerprint& setMinutiae(const std::vector<utils::storage::Minutiae>& minutiaes) { this->minutiaes = minutiaes; return *this; }
			Fingerprint& setMaxF(const float maxF) { this->maxF = maxF; return *this; }
			Fingerprint& setMinF(const float minF) { this->minF = minF; return *this; }
			Fingerprint& setTrashHold(const float trashHold) { this->trashHold = trashHold; return *this; }
			Fingerprint& setRegionMask(const utils::RegionMask& regionMask) { this->regionMask = regionMask; return *this; }
			Fingerprint& setMinutiaeTracking(const Mat& minutiaeTracing) { this->minutiaeTracing = minutiaeTracing; return *this; }
			
		};
	}
}

inline void processing::storage::Fingerprint::clear()
{
	this->blocks = 0;
	this->maxF = INFINITY;
	this->minF = -1;

	this->enhanced = false;

	this->normalized = Mat(10, 10, CV_32F);
	this->segmentation = Mat(10, 10, CV_32F);
	this->orientations = Mat(10, 10, CV_32F);
	this->frequencies = Mat(10, 10, CV_32F);
	this->thinned = Mat(10, 10, CV_32F);
	this->filtered = Mat(10, 10, CV_32F);
	this->binarized = Mat(10, 10, CV_32F);
	this->minutiaes = std::vector<utils::storage::Minutiae>();

	this->regionMask = processing::utils::RegionMask(Mat(10, 10, CV_32F));
}

inline int processing::storage::Fingerprint::getBlocks()
{
	if (this->blocks == 0 && this->orientations.size().height > 0)
	{
		for (auto i = this->blockSize / 2; i < this->orientations.rows; i += this->blockSize)
			for (auto j = this->blockSize / 2; j < this->orientations.cols; j += this->blockSize)
				if (this->orientations.at<cv::Vec2f>(i, j)[1] > .0f)
					this->blocks++;
	}

	return this->blocks;
}
