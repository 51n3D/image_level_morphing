#include "TemplateGenerator.h"
#include "exceptions/InvalidTemplateType.h"
#include "storage/AlignedFingerprint.h"
#include "FingerprintAligner.h"
#include "CutlineEstimator.h"
#include "exceptions/UnknownMorphingSeparation.h"

#include <storage/Fingerprint.h>
#include <utils/ImageProcessor.h>

#include <fstream>
#include <iostream>

using namespace processing::utils::storage;
using namespace processing::utils;
using namespace processing::storage;
using namespace morphing::utils::storage;
using namespace morphing::storage;
using namespace morphing::utils;
using namespace morphing;
using namespace cv;


int TemplateGenerator::displayed = 0;
const std::string TemplateGenerator::class_name = "TemplateGenerator::";

Fingerprint TemplateGenerator::imageLevelTemplate(const AlignedFingerprint& af, const Fingerprint& f, const bool colored)
{
	auto aligned = af.getAlignment();

	const auto afPos = FingerprintAligner::aFingPos(aligned[0], aligned[1]);
	const auto fPos = FingerprintAligner::fingPos(aligned[0], aligned[1]);

	const auto bb = FingerprintAligner::overlay(fPos, afPos, f.size(), af.getAligned().size());

	const auto line = af.getCutline();

	const auto r = f.getSegmentation();
	const auto ra = af.getSegmentation();

	Fingerprint morphed;
	switch (line.getSeparation())
	{
		case Cutline::SeparationType::POS_NEG:
		{
			if (colored)
			{
				// prevediem na farebny
				Mat coloredPositive(f.size(), CV_8UC3);
				ImageProcessor::convertTo(f, coloredPositive, CV_8U);
				ImageProcessor::addColorBreathToImage(coloredPositive, Scalar(1, 1.7, 1, 1));

				// prevediem na farebny
				Mat coloredNegative(af.getAligned().size(), CV_8UC3);
				ImageProcessor::convertTo(af.getAligned(), coloredNegative, CV_8U);
				ImageProcessor::addColorBreathToImage(coloredNegative, Scalar(1.7, 1, 1, 1));
				
				*static_cast<Mat*>(&morphed) = this->morphedFingerprintILColored(coloredPositive, coloredNegative, r, ra, bb, fPos, afPos, line);
			}
			else
			{
				*static_cast<Mat*>(&morphed) = this->morphedFingerprintIL(f, af.getAligned(), r, ra, bb, fPos, afPos, line);
			}

			auto minutiae = line.getPosFMin();
			auto otherPart = line.getNegAfMin();
			minutiae.insert(minutiae.end(), otherPart.begin(), otherPart.end());
				
			morphed.setMinutiae(minutiae);

			break;
		}
		case Cutline::SeparationType::NEG_POS:
		{
			if (colored)
			{
				// prevediem na farebny
				Mat coloredPositive(af.getAligned().size(), CV_8UC3);
				ImageProcessor::convertTo(af.getAligned(), coloredPositive, CV_8U);
				ImageProcessor::addColorBreathToImage(coloredPositive, Scalar(1.7, 1, 1, 1));

				// prevediem na farebny
				Mat coloredNegative(f.size(), CV_8UC3);
				ImageProcessor::convertTo(f, coloredNegative, CV_8U);
				ImageProcessor::addColorBreathToImage(coloredNegative, Scalar(1, 1.7, 1, 1));
				
				*static_cast<Mat*>(&morphed) = this->morphedFingerprintILColored(coloredPositive, coloredNegative, ra, r, bb, afPos, fPos, line);
			}
			else
			{
				*static_cast<Mat*>(&morphed) = this->morphedFingerprintIL(af.getAligned(), f, ra, r, bb, afPos, fPos, line);
			}

			auto minutiae = line.getPosAfMin();
			auto otherPart = line.getNegFMin();
			minutiae.insert(minutiae.end(), otherPart.begin(), otherPart.end());

			morphed.setMinutiae(minutiae);

			break;
		}
		case Cutline::SeparationType::NONE:
		default:
			throw exception::UnknownMorphingSeparation();
	}

	return morphed;
}

Mat TemplateGenerator::morphedFingerprintIL(const Mat& positive, const Mat& negative, const Mat& pR, const Mat& nR,
	const std::vector<Point>& bb, const Point& pPos, const Point& nPos, const Cutline& line)
{
	const auto size = Size(bb[1].x - bb[0].x, bb[1].y - bb[0].y);
	Mat morphed = Mat::ones(size, CV_32F);
	
	for (auto k = bb[0].y; k < bb[1].y; k++)
	{
		for (auto l = bb[0].x; l < bb[1].x; l++)
		{
			// premapujem poziciu na posunutie v zarovnani
			const auto mappedPPos = Point(l - pPos.x, k - pPos.y);
			const auto mappedNPos = Point(l - nPos.x, k - nPos.y);

			// ziskam regiony
			const auto r1 = pR.at<float>(mappedPPos);
			const auto r2 = nR.at<float>(mappedNPos);

			// skontrolujem ci som v oblasti
			if (r1 > .0f && r2 > .0f)
			{
				const auto current = Point(l, k);
				const auto d = CutlineEstimator::distance(line, current);

				const auto weight = this->calculateWeightingFactor(line, line.getDMax(), d, current);

				// zistim na ktorej som strane
				if (CutlineEstimator::isPositive(line, current))
				{
					const auto p = this->applyBorderWeightToPixel(positive, mappedPPos, mappedNPos, pR, nR);

					// pozriem ci som v priereze, ak hej aplikujem prechod na pixely
					if (d <= line.getDMax())
					{
						const auto n = this->applyBorderWeightToPixel(negative, mappedNPos, mappedPPos, nR, pR);
						
						morphed.at<float>(k - bb[0].y, l - bb[0].x) = this->calculateWeightedPixelValue(p, n, weight);
					}
					else
					{
						morphed.at<float>(k - bb[0].y, l - bb[0].x) = p;
					}
				}
				else
				{
					const auto n = this->applyBorderWeightToPixel(negative, mappedNPos, mappedPPos, nR, pR);

					// pozriem ci som v priereze, ak hej aplikujem prechod na pixely
					if (d <= line.getDMax())
					{
						const auto p = this->applyBorderWeightToPixel(positive, mappedPPos, mappedNPos, pR, nR);
						
						morphed.at<float>(k - bb[0].y, l - bb[0].x) = this->calculateWeightedPixelValue(p, n, weight);
					}
					else
					{
						morphed.at<float>(k - bb[0].y, l - bb[0].x) = n;
					}
				}
			}
			else
			{
				morphed.at<float>(k - bb[0].y, l - bb[0].x) = this->backgroundColor;
			}
		}
	}

	return morphed;
}

Mat TemplateGenerator::morphedFingerprintILColored(const Mat& positive, const Mat& negative, const Mat& pR, const Mat& nR,
	const std::vector<Point>& bb, const Point& pPos, const Point& nPos, const Cutline& line)
{
	const auto size = Size(bb[1].x - bb[0].x, bb[1].y - bb[0].y);
	
	Mat morphed = Mat::ones(size, positive.type());
	for (auto k = bb[0].y; k < bb[1].y; k++)
	{
		for (auto l = bb[0].x; l < bb[1].x; l++)
		{
			// premapujem poziciu na posunutie v zarovnani
			const auto mappedPPos = Point(l - pPos.x, k - pPos.y);
			const auto mappedNPos = Point(l - nPos.x, k - nPos.y);

			// ziskam regiony
			const auto r1 = pR.at<float>(mappedPPos);
			const auto r2 = nR.at<float>(mappedNPos);

			// skontrolujem ci som v oblasti
			if (r1 > .0f && r2 > .0f)
			{
				const auto current = Point(l, k);
				const auto d = CutlineEstimator::distance(line, current);

				const auto weight = this->calculateWeightingFactor(line, line.getDMax(), d, current);

				// zistim na ktorej som strane
				if (CutlineEstimator::isPositive(line, current))
				{
					const auto p = positive.at<Vec4f>(mappedPPos);

					// pozriem ci som v priereze, ak hej aplikujem prechod na pixely
					if (d <= line.getDMax())
					{
						const auto n = negative.at<Vec4f>(mappedNPos);

						morphed.at<Vec4f>(k - bb[0].y, l - bb[0].x) = this->calculateWeightedColoredPixelValue(p, n, weight);
					}
					else
					{
						morphed.at<Vec4f>(k - bb[0].y, l - bb[0].x) = p;
					}
				}
				else
				{
					const auto n = negative.at<Vec4f>(mappedNPos);

					// pozriem ci som v priereze, ak hej aplikujem prechod na pixely
					if (d <= line.getDMax())
					{
						const auto p = positive.at<Vec4f>(mappedPPos);

						morphed.at<Vec4f>(k - bb[0].y, l - bb[0].x) = this->calculateWeightedColoredPixelValue(p, n, weight);
					}
					else
					{
						morphed.at<Vec4f>(k - bb[0].y, l - bb[0].x) = n;
					}
				}
			}
			else
			{
				morphed.at<Vec4f>(k - bb[0].y, l - bb[0].x) = Vec4f(1, 1, 1, 1);
			}
		}
	}

	return morphed;
}

float TemplateGenerator::applyBorderWeightToPixel(const Mat& map, const Point& pos1, const Point& pos2, const Mat& r1, const Mat& r2)
{
	// posunutia
	const std::array<std::array<float, 2>, 8> shifts = { {
		{0, 1}, {-1, 0},
		{0, -1}, {1, 0}
	} };

	// sikme posunutia
	const auto cosDir = std::cos(CV_PI / 4);
	const auto sinDir = std::cos(CV_PI / 4);
	const std::array<std::array<float, 2>, 8> slantShifts = { {
		{1, 1}, {-1, 1},
		{-1, -1}, {1, -1},
	} };

	// ak je jeden z regionov na aktualnej pozicii 0, (t.j. pozadie), nezaujima ma to
	if (r1.at<float>(pos1) == 0 || r2.at<float>(pos2) == 0)
	{
		return map.at<float>(pos1);
	}

	// ziskam vzdialenost od okraja odtlacku
	auto distance = -1;
	this->distanceInBorderRegion(pos1, pos2, r1, r2, shifts, 1, 1, distance);
	this->distanceInBorderRegion(pos1, pos2, r1, r2, slantShifts, cosDir, sinDir, distance);

	
	if (distance > -1)
	{
		// vyvazim pixelovu hodnotu na zaklade vzdialenosti
		auto weight = static_cast<float>(this->border - distance) / (this->border);
		weight = std::max(.0f, weight);
		
		return this->calculateWeightedPixelValue(map.at<float>(pos1), this->backgroundColor, weight);
	}

	return map.at<float>(pos1);
}

void TemplateGenerator::distanceInBorderRegion(const Point& pos1, const Point& pos2, const Mat& r1, const Mat& r2,
                                               const std::array<std::array<float, 2>, 8>& shifts, const float cosRadius, const float sinRadius, int& distance)
{
	// prehladavam posunutia
	for (const auto& shift : shifts)
	{
		auto currentDistance = this->border;

		auto checkPos1 = Point();
		auto checkPos2 = Point();
		// shiftnem poziciu
		auto shiftTo1 = Point(pos1.x + shift[0] * currentDistance * cosRadius, pos1.y + shift[1] * currentDistance * sinRadius);
		auto shiftTo2 = Point(pos2.x + shift[0] * currentDistance * cosRadius, pos2.y + shift[1] * currentDistance * sinRadius);
		this->shiftCurrentPosition(checkPos1, shiftTo1, r1);
		this->shiftCurrentPosition(checkPos2, shiftTo2, r2);

		// skontrolujem ci sa v okoli nachadza oblast mimo morfovanej oblasti
		if (r1.at<float>(checkPos1) == 0 || (r1.at<float>(checkPos1) == 1 && r2.at<float>(checkPos2) == 0))
		{
			// poctam dlzku pokial som mimo morfovanej oblasti
			auto length = 0;
			while (r1.at<float>(checkPos1) == 0 || (r1.at<float>(checkPos1) == 1 && r2.at<float>(checkPos2) == 0))
			{
				length++;
				currentDistance--;

				if (currentDistance < 0)
				{
					break;
				}

				// posuniem sa o bod spat
				shiftTo1 = Point(pos1.x + shift[0] * currentDistance * cosRadius, pos1.y + shift[1] * currentDistance * sinRadius);
				shiftTo2 = Point(pos2.x + shift[0] * currentDistance * cosRadius, pos2.y + shift[1] * currentDistance * sinRadius);
				this->shiftCurrentPosition(checkPos1, shiftTo1, r1);
				this->shiftCurrentPosition(checkPos2, shiftTo2, r2);
			}

			if (currentDistance < 0)
			{
				continue;
			}
			
			distance = length > distance ? length : distance;
		}
	}
}

void TemplateGenerator::shiftCurrentPosition(Point& current, const Point& where, const Mat& r)
{
	current = where;
	current.x = current.x < 0 ? 0 : current.x;
	current.y = current.y < 0 ? 0 : current.y;
	current.x = current.x >= r.cols ? r.cols - 1 : current.x;
	current.y = current.y >= r.rows ? r.rows - 1 : current.y;
}

float TemplateGenerator::calculateWeightingFactor(const Vec3f& line, const int dmax, const float distance, const Point& current) const
{
	const auto weight = (dmax - distance) / (2 * dmax);
	
	if (CutlineEstimator::isPositive(line, current))
	{
		return 1 - std::max(.0f, weight);
	}

	return std::max(.0f, weight);
}

float TemplateGenerator::calculateWeightedPixelValue(const float p, const float n, const float weight)
{
	return weight * p + (1 - weight) * n;
}

Vec4f TemplateGenerator::calculateWeightedColoredPixelValue(const Vec4f& p, const Vec4f& n, const float weight)
{
	return {
		weight * p[0] + (1 - weight) * n[0],
		weight* p[1] + (1 - weight) * n[1],
		weight * p[2] + (1 - weight) * n[2],
		1
	};
}

Fingerprint TemplateGenerator::featureLevelTemplate(const AlignedFingerprint& af, const Fingerprint& f)
{
	/*
	 * TODO
	 * Určené pre syntetické generovanie odtlačku.
	 * Momentálne to však zabezpečuje externá aplikácia SyFDaS,
	 * metódou TemplateGenerator::createMinutiaeTemplateToFile()
	 * je možné vygenerovať .txt templatu ktorá je kompatibilná
	 * s formátom markantov syntetického generátora SyFDaS.
	 */
	
	return static_cast<Fingerprint>(Mat::zeros(af.size(), af.type()));
}

Fingerprint TemplateGenerator::generate(const AlignedFingerprint& af, const Fingerprint& f, const int type)
{
	switch (type)
	{
		case IMAGE_LEVEL:
		{
			return this->imageLevelTemplate(af, f);
		}
		case FEATURE_LEVEL:
		{
			return this->featureLevelTemplate(af, f);
		}
		case IMAGE_LEVEL_COLORED:
		{
			return this->imageLevelTemplate(af, f, true);
		}
		default:
		{
			throw exception::InvalidTemplateType();
		}
	}
}

void TemplateGenerator::display(const Fingerprint& fingerprint, const std::string& trace) const
{
	std::stringstream ss; ss << ++displayed << ": Template" << " TRACE: " << trace;
	imshow(ss.str(), fingerprint);
}

void TemplateGenerator::write(const Fingerprint& fingerprint, const std::string& path) const
{
	Mat image;
	fingerprint.copyTo(image);
	
	image.convertTo(image, CV_8UC3, 255);

	std::stringstream ss; ss << path << ".jpg";
	imwrite(ss.str(), image);
}

bool TemplateGenerator::createSyFDaSTemplate(const Fingerprint& fingerprint)
{
	std::ofstream f("sfydas_template.txt", std::ofstream::out | std::ofstream::trunc);

	if (f.is_open())
	{
		auto counter = 0;
		for (const auto& minutia : fingerprint.getMinutiae())
		{
			const auto pos = minutia.getPosition();
			auto dir = minutia.getDirection();

			// normalizujem smery z [0..2pi] na [-pi..pi]
			dir = fmodf(dir + CV_2PI, CV_2PI);

			f << "{"
				<< "m:" << counter << ","
				<< "x:" << static_cast<int>((static_cast<double>(pos.x) / fingerprint.cols) * 320) << ","
				<< "y:" << static_cast<int>((static_cast<double>(pos.y) / fingerprint.rows) * 440) << ","
				<< "a:" << static_cast<int>(dir * 180 / CV_PI) << ","
				<< "t:" << (minutia.getType() == Minutiae::Type::TERMINATION ? 0 : 1)
			<< "}," << std::endl;

			counter++;
		}
		f.close();
	}
	else
	{
		std::cerr << "Could not write to file! Some error occured!" << std::endl;
		return false;
	}

	return true;
}
