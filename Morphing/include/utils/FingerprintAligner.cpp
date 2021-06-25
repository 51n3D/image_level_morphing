#include "FingerprintAligner.h"
#include "storage/AlignedFingerprint.h"

#include <utils/ImageProcessor.h>
#include <storage/Fingerprint.h>

using namespace morphing::storage;
using namespace morphing::utils;
using namespace processing::storage;
using namespace processing::utils;
using namespace processing;
using namespace cv;

int FingerprintAligner::displayed = 0;
const std::string FingerprintAligner::class_name = "FingerprintAligner::";

FingerprintAligner::FingerprintAligner(FingerprintProcessor& processor) : processor(processor) { }

void FingerprintAligner::accurateAlign(AlignedFingerprint& af, Fingerprint& f)
{	
	this->processor.estimateOrientations(f);
	auto o = f.getOrientations();

	auto maxSimilarity = .0f;
	auto pos = Vec3f(.0f, .0f, .0f);

	auto aligned = AlignedFingerprint(af);

	for (auto angle = -90; angle <= 90; angle += this->rotationStep)
	{
		*static_cast<Mat*>(&aligned) = ImageProcessor::rotate(af, angle);

		this->processor.normalize(aligned);
		
		this->processor.estimateOrientations(aligned);
		auto oa = aligned.getOrientations();

		// o polovicu zmensim prehladavanie
		const auto rows = aligned.rows / this->translationStep / 2 + f.rows / this->translationStep / 2;
		const auto cols = aligned.cols / this->translationStep / 2 + f.cols / this->translationStep / 2;

		// o polovicu odtlacku shiftnem poziciu odtlacku
		const Point fPos(cols / 2 * this->translationStep, rows / 2 * this->translationStep);
		
		for (auto i = 0; i < rows; i++)
		{
			for (auto j = 0; j < cols; j++)
			{
				// pozicia zarovnavaneho
				const Point afPos(j * this->translationStep, i * this->translationStep);

				const auto s = similarity(aligned, f, afPos, fPos);
	
				if (maxSimilarity < s)
				{
					maxSimilarity = s;
					pos = Vec3f(afPos.x - fPos.x, afPos.y - fPos.y, angle);
					
					af.setNormalized(aligned.getNormalized());
					af.setAligned(aligned);
					af.setOrientations(oa);
					af.setAlignment(pos[0], pos[1], pos[2]);
				}
			}
		}
	}

	if (this->isVerbose())
	{
		std::stringstream ss; ss << class_name << "align";
		
		this->processor.displayOrientations(f, ss.str());
		this->processor.displayOrientations(af, ss.str());

		this->display(af, f, ss.str());
	}
}

void FingerprintAligner::align(AlignedFingerprint& af, Fingerprint& f) const
{
	auto o = f.getOrientations();
	auto oa = af.getOrientations();
	auto na = af.getNormalized();
	auto va = af.getFrequencies();
	auto ra = af.getRegionMask();

	auto maxSimilarity = .0f;
	auto pos = Vec3f(.0f, .0f, .0f);

	auto aligned = AlignedFingerprint(af);
	std::vector<Point> identity;
	for (auto angle = -90; angle <= 90; angle += this->rotationStep)
	{
		// segmentacia odtlacku
		std::vector<Point> ol;
		auto oar = rotateOrientations(oa, angle, ol);
		*static_cast<Mat*>(&aligned) = Mat::zeros(oar.size(), af.type());
		aligned.setOrientations(oar);

		// zaistenie velkosti celej oblasti zarovnanvani vramci oboch odtlackov
		const auto rows = oar.rows / this->translationStep / 2 + f.rows / this->translationStep / 2;
		const auto cols = oar.cols / this->translationStep / 2 + f.cols / this->translationStep / 2;

		const Point fPos(cols / 2 * this->translationStep, rows / 2 * this->translationStep);

		for (auto i = 0; i < rows; i++)
		{
			for (auto j = 0; j < cols; j++)
			{
				// skacem po blokoch
				const Point afPos(j * this->translationStep, i * this->translationStep);

				const auto bb = overlay(fPos, afPos, f.size(), oar.size());
				const auto overlapped = ((bb[1].x - bb[0].x) / f.getBlockSize()) * ((bb[1].y - bb[0].y) / f.getBlockSize());

				if (this->negligibleArea(overlapped, aligned, f))
				{
					continue;
				}
				
				const auto s = similarity(aligned, f, afPos, fPos);

				if (maxSimilarity < s)
				{
					maxSimilarity = s;
					pos = Vec3f(afPos.x - fPos.x, afPos.y - fPos.y, angle);

					identity = ol;
					
					af.setOrientations(oar);
					af.setAlignment(pos[0], pos[1], pos[2]);
				}
			}
		}
	}

	// otocim a orezem ostatne vlastnosti tak aby boli vhodne k zarovnanemu odtlacku
	const auto angle = af.getAlignment()[2];
	auto car = ImageProcessor::rotate(af, angle, identity, true);
	auto nar = ImageProcessor::rotate(na, angle, identity, true);
	auto var = ImageProcessor::rotate(va, angle, identity);
	auto rar = ImageProcessor::rotate(ra, angle, identity);

	*static_cast<Mat*>(&ra) = rar;
	ra.setRegion(identity);
	
	af.setAligned(car);
	af.setNormalized(nar);
	af.setFrequencies(var);
	af.setRegionMask(ra);
	
	if (this->isVerbose())
	{
		std::stringstream ss; ss << class_name << "align";

		this->processor.displayOrientations(f, ss.str());
		this->processor.displayOrientations(af, ss.str());

		this->display(af, f, ss.str());
	}
}

float FingerprintAligner::similarity(AlignedFingerprint& af, Fingerprint& f, const Point& afPos, const Point& fPos) const
{
	auto o = f.getOrientations();
	auto oa = af.getOrientations();
	
	const auto blockSize = af.getBlockSize();
	
	auto numerator = .0f;
	auto nominator = .0f;

	auto overlapped = 0;

	const auto bb = overlay(fPos, afPos, o.size(), oa.size());

	for (auto k = bb[0].y + blockSize / 2; k < bb[1].y; k += blockSize)
	{
		for (auto l = bb[0].x + blockSize / 2; l < bb[1].x; l += blockSize)
		{
			// regiony odtlackov
			const auto r1 = oa.at<Vec2f>(k - afPos.y, l - afPos.x)[1];
			const auto r2 = o.at<Vec2f>(k - fPos.y, l - fPos.x)[1];

			// skontrolujem ci sa nachadzam v morfovanej oblasti
			if (r1 > .0f && r2 > .0f)
			{
				overlapped++;

				const auto o1 = oa.at<Vec2f>(k - afPos.y, l - afPos.x)[0];
				const auto o2 = o.at<Vec2f>(k - fPos.y, l - fPos.x)[0];

				numerator += (r1 + r2) * static_cast<float>(1.0 - 2.0 * abs(o1 - o2) / CV_PI);
				nominator += (r1 + r2);
			}
		}
	}

	if (this->negligibleArea(overlapped, af, f))
	{
		return -1;
	}
	
	return numerator / nominator;;
}

int FingerprintAligner::unitLength(const Vec2f& pos, const int length1, const int length2)
{
	const auto offset = pos[0] - pos[1];
	
	int variant1, variant2;
	if (offset > 0)
	{
		variant1 = offset + length1;
		variant2 = length2;
	}
	else
	{
		variant1 = -offset + length2;
		variant2 = length1;
	}

	return variant1 > variant2 ? variant1 : variant2;
}

std::vector<Point> FingerprintAligner::overlay(const Point& fPos, const Point& afPos, const Size& fSize, const Size& afSize)
{
	std::vector<Point> boundingBox(2, Point());
	
	boundingBox[0].x = max(fPos.x, afPos.x);
	boundingBox[0].y = max(fPos.y, afPos.y);
	boundingBox[1].x = min(afPos.x + afSize.width, fPos.x + fSize.width);
	boundingBox[1].y = min(afPos.y + afSize.height, fPos.y + fSize.height);

	return boundingBox;
}

Point FingerprintAligner::aFingPos(const int xOffset, const int yOffset)
{
	const auto afx = xOffset > 0 ? xOffset : 0;
	const auto afy = yOffset > 0 ? yOffset : 0;

	return {afx, afy};
}

Point FingerprintAligner::fingPos(const int xOffset, const int yOffset)
{
	const auto fx = xOffset > 0 ? 0 : -xOffset;
	const auto fy = yOffset > 0 ? 0 : -yOffset;
	
	return {fx, fy};
}

bool FingerprintAligner::negligibleArea(const int overlapped, AlignedFingerprint& af, Fingerprint& f) const
{
	int blocks;
	// get smaller one
	if (af.getOrientations().rows * af.getOrientations().cols
		> f.getOrientations().rows * f.getOrientations().cols)
	{
		blocks = af.getBlocks();
	}
	else
	{
		blocks = f.getBlocks();
	}

	if (overlapped < blocks * this->trashHold)
	{
		return true;
	}


	return false;
}

Mat FingerprintAligner::rotateOrientations(const Mat& orientations, const float angle, std::vector<Point>& bb)
{
	const Point2f center((orientations.cols - 1) / 2.0, (orientations.rows - 1) / 2.0);
	auto rot = getRotationMatrix2D(center, -angle, 1.0);

	const auto bbox = RotatedRect(Point2f(), orientations.size(), -angle).boundingRect2f();

	rot.at<double>(0, 2) += bbox.width / 2.0 - orientations.cols / 2.0;
	rot.at<double>(1, 2) += bbox.height / 2.0 - orientations.rows / 2.0;

	Mat dst(bbox.size(), orientations.type(), Scalar(0, 0));
	warpAffine(orientations, dst, rot, bbox.size());

	const auto rows = dst.rows;
	const auto cols = dst.cols;

	auto minX = cols, minY = rows, maxX = 0, maxY = 0;

	// musim este otocit orientacie, o pacny uhol k uhlu otocenia celej mapy
	for (auto i = 0; i < rows; i++)
	{
		for (auto j = 0; j < cols; j++)
		{
			if (dst.at<Vec2f>(i, j)[1] < 0 || dst.at<Vec2f>(i, j)[1] > 1)
			{
				dst.at<Vec2f>(i, j)[1] = 0;
				dst.at<Vec2f>(i, j)[0] = 0;
			}

			if (dst.at<Vec2f>(i, j)[1] != 0)
			{
				dst.at<Vec2f>(i, j)[0] = fmodf((dst.at<Vec2f>(i, j)[0] + angle * CV_PI / 180.0f), CV_PI);

				// ziskam segmentaciu
				if (minX > j) { minX = j; }
				if (minY > i) { minY = i; }
				if (maxX < j) { maxX = j; }
				if (maxY < i) { maxY = i; }
			}
		}
	}

	bb.emplace_back(minX, minY);
	bb.emplace_back(maxX, maxY);

	dst(Rect(minX, minY, maxX - minX, maxY - minY)).copyTo(dst);

	return dst;
}

Mat FingerprintAligner::getAlignedFingersImage(const AlignedFingerprint& af, const Fingerprint& f) const
{
	const auto o = f.getOrientations();
	const auto oa = af.getOrientations();

	auto alignment = af.getAlignment();

	const auto afPos = aFingPos(alignment[0], alignment[1]);
	const auto fPos = fingPos(alignment[0], alignment[1]);

	const auto width = unitLength(Vec2f(afPos.x, fPos.x), af.getAligned().cols, f.cols);
	const auto height = unitLength(Vec2f(afPos.y, fPos.y), af.getAligned().rows, f.rows);

	const Mat a = Mat::ones(height, width, CV_32F);
	const Mat b = Mat::ones(height, width, CV_32F);

	f.copyTo(a(Rect(fPos, Size(f.cols, f.rows))));
	af.getAligned().copyTo(b(Rect(afPos, Size(af.getAligned().cols, af.getAligned().rows))));

	Mat tmp1(a.size(), CV_8UC3);
	ImageProcessor::convertTo(a, tmp1, CV_8U);
	ImageProcessor::addColorBreathToImage(tmp1, Scalar(1, 1.7, 1, 1));
	
	Mat tmp2(b.size(), CV_8UC3);
	ImageProcessor::convertTo(b, tmp2, CV_8U);
	ImageProcessor::addColorBreathToImage(tmp2, Scalar(1.7, 1, 1, 1));
	
	const auto alpha = 0.5f;
	Mat aligned;
	addWeighted(tmp1, alpha, tmp2, (1.0 - alpha), 0.0, aligned);

	const auto bb = overlay(fPos, afPos, f.size(), af.getAligned().size());

	auto blockSize = f.getBlockSize();

	if (this->displayCommonArea())
	{
		for (auto k = bb[0].y + blockSize / 2; k < bb[1].y; k += blockSize)
		{
			for (auto l = bb[0].x + blockSize / 2; l < bb[1].x; l += blockSize)
			{
				const auto r1 = oa.at<Vec2f>(k - afPos.y, l - afPos.x)[1];
				const auto r2 = o.at<Vec2f>(k - fPos.y, l - fPos.x)[1];

				if (r1 > .0f && r2 > .0f)
				{
					rectangle(
						aligned,
						Point(l - blockSize / 2, k - blockSize / 2),
						Point(l + blockSize / 2, k + blockSize / 2),
						Scalar(0, 0, 0, 1),
						1
					);
				}
			}
		}
	}

	return aligned;
}

void FingerprintAligner::display(const AlignedFingerprint& af, const Fingerprint& f, const std::string& trace) const
{
	auto fing = f.get();
	auto aFing = af.get();
	auto aligned = this->getAlignedFingersImage(af, f);
	ImageProcessor::resize(fing, 400);
	ImageProcessor::resize(aFing, 400);
	ImageProcessor::resize(aligned, 400);

	const auto resWidth = fing.cols + aFing.cols + aligned.cols;
	const auto resHeight = fing.rows;
	Mat result = Mat::zeros(resHeight, resWidth, CV_32F);

	fing.copyTo(result(Rect(Point(0, 0), fing.size())));
	aFing.copyTo(result(Rect(Point(fing.cols, 0), aFing.size())));
	aligned.copyTo(result(Rect(Point(fing.cols + aFing.cols, 0), aligned.size())));

	std::stringstream ss; ss << ++displayed << ": Aligned" << " TRACE: " << trace;
	imshow(ss.str(), result);
}

void FingerprintAligner::write(const AlignedFingerprint& af, const Fingerprint& f, const std::string& path) const
{
	auto image = this->getAlignedFingersImage(af, f);

	image.convertTo(image, CV_8UC3, 255);

	std::stringstream ss; ss << path << ".jpg";
	imwrite(ss.str(), image);
}
