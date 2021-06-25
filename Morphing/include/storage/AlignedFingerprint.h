#pragma once

#include "Cutline.h"

#include <storage/Fingerprint.h>

namespace morphing
{
	namespace storage
	{
		/**
		 * \brief Trieda rozsirujuca triedu Fingerprint o dodatocne ulozisko
		 * dát potrebnych k zarovnaniu odtlackov pri morfovani.
		 */
		class AlignedFingerprint : public processing::storage::Fingerprint
		{
		private: 
			/**
			 * \brief Zarovnany odtlacok.
			 */
			Mat aligned;

			/**
			 * \brief Offset zarovnania.
			 */
			cv::Vec3f alignment;

			/**
			 * \brief Idealna rezna linia.
			 */
			utils::storage::Cutline cutline;

		public:
			// constructors
			AlignedFingerprint() = default;
			explicit AlignedFingerprint(const Mat& fingerprint) : Fingerprint(fingerprint) {}

			// copy constructors
			AlignedFingerprint(const AlignedFingerprint& af);

			// methods
			/**
			 * \brief Vycisti odtlacok of vsetkych dat.
			 */
			void clear();
			
			// getters
			Mat getAligned() const { return this->aligned; }
			cv::Vec3f getAlignment() const { return this->alignment; }
			utils::storage::Cutline getCutline() const { return this->cutline; }
			
			// setters
			AlignedFingerprint& setAligned(const Mat& aligned) { this->aligned = aligned; return *this; }
			AlignedFingerprint& setAlignment(const float x, const float y, const float angle) { this->alignment = cv::Vec3f(x, y, angle); return *this; }
			AlignedFingerprint& setCutline(const utils::storage::Cutline& line) { this->cutline = line; return *this; }
			
		};
	}
}

inline morphing::storage::AlignedFingerprint::AlignedFingerprint(const AlignedFingerprint& af) : Fingerprint(af.get())
{
	this->aligned = af.aligned;
	this->alignment = af.alignment;

	this->blockSize = af.blockSize;
	this->windowSize = af.windowSize;
}

inline void morphing::storage::AlignedFingerprint::clear()
{
	this->aligned = Mat(10, 10, CV_32F);
	this->alignment = cv::Vec3f();
	this->cutline = utils::storage::Cutline();

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
	this->minutiaes = std::vector<processing::utils::storage::Minutiae>();

	this->regionMask = processing::utils::RegionMask(Mat(10, 10, CV_32F));
}
