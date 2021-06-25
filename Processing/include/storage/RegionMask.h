#pragma once

#include <opencv2/opencv.hpp>

namespace processing
{
	namespace utils
	{
		class RegionMask : public cv::Mat
		{
		private:
			// members
			/**
			 * \brief Celkova vzdialenost vrcholov sinusoidy.
			 */
			float totalPeakDistance = 0;
			/**
			 * \brief Pocet ulozenych vzdialenosti.
			 */
			int peakDistances = 0;

			/**
			 * \brief "2D bounding box" popredia odtlacku.
			 */
			std::vector<cv::Point> region;
			
		public:
			// constructors
			RegionMask() = default;
			explicit RegionMask(const Mat& fingerprint);

			// methods
			/**
			 * \brief Odhadne idealnu velkost bloku Gaborovho filtra.
			 * \return 
			 */
			int idealGaborBlock() const;

			// static methods
			/**
			 * \brief Ziska amplitudu zo sinusoidy.
			 * \param peaks vrcholy
			 * \param signal sinusoida
			 * \return amplituda
			 */
			static float amplitude(const std::array<std::vector<int>, 2>& peaks, const std::vector<float>& signal);

			// getters
			/**
			 * \brief Ziska priemernu vzdialenost vrcholov sinusoidy zo vsetkych ziskanych.
			 * \return 
			 */
			float getAveragePeakDistance() const { return this->totalPeakDistance / this->peakDistances; }
			std::vector<cv::Point> getRegion() const { return this->region; }

			// setters
			RegionMask& addPeaksDistance(const float distance) { this->totalPeakDistance += distance; this->peakDistances++; return *this; }
			RegionMask& setRegion(const std::vector<cv::Point>& region) { this->region = region; return *this; }
		};
	}
}

inline processing::utils::RegionMask::RegionMask(const Mat& fingerprint)
	: Mat(fingerprint.size(), fingerprint.type()) {}

inline int processing::utils::RegionMask::idealGaborBlock() const
{
	const auto blockSize = static_cast<int>(std::round(this->getAveragePeakDistance()));

	return blockSize % 2 == 1 ? blockSize : blockSize + 1;
}

inline float processing::utils::RegionMask::amplitude(const std::array<std::vector<int>, 2>& peaks, const std::vector<float>& signal)
{
	std::array<float, 2> avgPeak;
	for (auto i = 0; i < peaks.size(); i++)
	{
		float sum = 0;
		
		for (const auto& index: peaks[i])
		{
			sum += signal.at(index);
		}

		avgPeak[i] = sum / peaks[i].size();
	}

	return avgPeak[0] - avgPeak[1];
}
