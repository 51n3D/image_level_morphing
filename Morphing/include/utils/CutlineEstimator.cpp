#include "CutlineEstimator.h"
#include "storage/AlignedFingerprint.h"
#include "FingerprintAligner.h"
#include "MorphingProcessor.h"

#include <utils/ImageProcessor.h>
#include <storage/Fingerprint.h>

using namespace processing::storage;
using namespace processing::utils::storage;
using namespace processing::utils;
using namespace processing;
using namespace morphing::utils::storage;
using namespace morphing::storage;
using namespace morphing::utils;
using namespace cv;

int CutlineEstimator::displayed = 0;
const std::string CutlineEstimator::class_name = "CutlineEstimator::";

CutlineEstimator::CutlineEstimator(FingerprintProcessor& processor) { }

Cutline CutlineEstimator::estimate(AlignedFingerprint& af, Fingerprint& f)
{
	if (this->dynamicCutline)
	{
		this->computeDynamic(af, f);
	}
	else
	{
		this->compute(af, f);
	}

	if (this->isVerbose())
	{
		std::stringstream ss; ss << class_name << "estimate";
		this->display(af, f, ss.str());
	}

	return this->cutline;
}

void CutlineEstimator::compute(AlignedFingerprint& af, Fingerprint& f)
{
	auto alignment = af.getAlignment();

	// toto je spravne 
	const auto afPos = FingerprintAligner::aFingPos(alignment[0], alignment[1]);
	const auto fPos = FingerprintAligner::fingPos(alignment[0], alignment[1]);

	// toto je tiez fajn
	const auto bb = FingerprintAligner::overlay(fPos, afPos, f.size(), af.getAligned().size());

	auto filtered = this->filterMorphedMinutiaes(af, f);
	const auto mins = filtered[0];
	const auto aMins = filtered[1];

	auto maxCutlineScore = .0;
	for (auto rotation = .0f; rotation < CV_PI; rotation += this->rotStep)
	{
		// toto je spravne
		const auto line = this->estimateCutline(rotation, bb[0].x + (bb[1].x - bb[0].x) / 2, bb[0].y + (bb[1].y - bb[0].y) / 2);

		Cutline cLine(line);

		const auto s = this->score(af, f, afPos, fPos, cLine, bb, mins, aMins);

		if (s > maxCutlineScore)
		{
			maxCutlineScore = s;
			this->cutline = cLine;
			this->cutline.setDMax(this->dmax);
		}
	}
}

void CutlineEstimator::computeDynamic(AlignedFingerprint& af, Fingerprint& f)
{
	auto alignment = af.getAlignment();

	const auto afPos = FingerprintAligner::aFingPos(alignment[0], alignment[1]);
	const auto fPos = FingerprintAligner::fingPos(alignment[0], alignment[1]);

	const auto bb = FingerprintAligner::overlay(fPos, afPos, f.size(), af.getAligned().size());

	auto filtered = this->filterMorphedMinutiaes(af, f);
	const auto mins = filtered[0];
	const auto aMins = filtered[1];

	const std::array<std::array<int, 2>, 9> shifts = { {
		{0, 1}, {-1, 1}, {-1, 0},
		{-1, -1}, {0, -1}, {0, 0},
		{1, -1}, {1, 0}, {1, 1},
	} };

	auto maxCutlineScore = .0;
	auto centroid = Vec2b(bb[0].x + (bb[1].x - bb[0].x) / 2, bb[0].y + (bb[1].y - bb[0].y) / 2);
	for (const auto& shift: shifts)
	{
		// posunutie od taziska
		const auto shift1 = centroid[0] + shift[0] * this->area;
		const auto shift2 = centroid[1] + shift[1] * this->area;

		// kontrola ci sa nenachadzam mimo obrazku
		auto region = Vec2b();
		region[0] = shift1 < bb[0].x ? bb[0].x : (shift1 > bb[1].x ? bb[1].x : shift1);
		region[1] = shift2 < bb[0].y ? bb[0].y : (shift2 > bb[1].y ? bb[1].y : shift2);

		for (auto rotation = .0f; rotation < CV_PI; rotation += this->rotStep)
		{
			const auto line = this->estimateCutline(rotation, region[0], region[1]);

			Cutline cLine(line);

			const auto s = this->score(af, f, afPos, fPos, cLine, bb, mins, aMins);

			if (s > maxCutlineScore)
			{
				maxCutlineScore = s;
				this->cutline = cLine;
				this->cutline.setDMax(this->dmax);
			}
		}
	}
}

Vec3f CutlineEstimator::estimateCutline(const float rotation, const float regionLength, const float regionStep) const
{
	const auto a = sin(rotation);
	const auto b = cos(rotation);
	const auto c = - regionLength * a - regionStep * b;
	
	return Vec3f(a, b, c);
}

float CutlineEstimator::distance(const Vec3f& line, const Point& p)
{
	return abs(line[0] * p.x + line[1] * p.y + line[2]) / sqrt(pow(line[0], 2) + pow(line[1], 2));
}

float CutlineEstimator::score(const AlignedFingerprint& af, const Fingerprint& f, const Point& afPos, const Point& fPos,
	Cutline& cLine, const std::vector<Point>& bb, const std::vector<Minutiae>& minutiae, const std::vector<Minutiae>& aMinutiae) const
{
	const auto o = f.getOrientations();
	const auto oa = af.getOrientations();
	const auto v = f.getFrequencies();
	const auto va = af.getFrequencies();

	auto orientationValidity = .0f;
	auto orientationSimilarity = .0f;

	auto frequencySimilarity = 0.f;
	
	auto oNum = .0f;
	auto oNom = .0f;
	auto vNum = .0f;
	auto vNom = .0f;
	
	const auto blockSize = f.getBlockSize();
	for (auto k = bb[0].y + blockSize / 2; k < bb[1].y; k += blockSize)
	{
		for (auto l = bb[0].x + blockSize / 2; l < bb[1].x; l += blockSize)
		{
			const auto r1 = oa.at<Vec2f>(k - afPos.y, l - afPos.x)[1];
			const auto r2 = o.at<Vec2f>(k - fPos.y, l - fPos.x)[1];

			const auto d = this->distance(cLine, Point(l, k));

			// ak sme v popredi odtlacku a do vzdialenosti dmax, ohodnotim reznu liniu v ramci orientacii a frekvencii
			if (d <= this->dmax && r1 > .0f && r2 > .0f)
			{
				const auto o1 = oa.at<Vec2f>(k - afPos.y, l - afPos.x)[0];
				const auto o2 = o.at<Vec2f>(k - fPos.y, l - fPos.x)[0];

				const auto v1 = va.at<float>(k - afPos.y, l - afPos.x);
				const auto v2 = v.at<float>(k - fPos.y, l - fPos.x);
				
				const auto maxF = af.getMaxF() > f.getMaxF() ? af.getMaxF() : f.getMaxF();
				const auto minF = af.getMinF() < f.getMinF() ? af.getMinF() : f.getMinF();

				orientationValidity = (r1 + r2);
				orientationSimilarity = static_cast<float>(1.0 - 2.0 * abs(o1 - o2) / CV_PI);
				
				oNum += orientationValidity * orientationSimilarity;
				oNom += orientationValidity;

				frequencySimilarity = 1 - abs(v1 - v2) / (maxF - minF);
				
				vNum += frequencySimilarity;
				vNom += 1;
			}
		}
	}
	
	// zistim kardinalitu markantov 
	const auto minutiaesCard = this->minutiaeCardinality(cLine, minutiae);
	const auto aMinutiaesCard = this->minutiaeCardinality(cLine, aMinutiae);
	cLine
		.setPosFMin(minutiaesCard[0]).setNegFMin(minutiaesCard[1])
		.setPosAfMin(aMinutiaesCard[0]).setNegAfMin(aMinutiaesCard[1]);

	float fAfScore = 0;
	float afFScore = 0;
	float sM = 0;
	if (this->adaptiveMethod)
	{
		fAfScore = this->minutiaeScore2(cLine.getPosFCardinality(), cLine.getNegAfCardinality());
		afFScore = this->minutiaeScore2(cLine.getPosAfCardinality(), cLine.getNegFCardinality());
		std::cout << "adaptive" << std::endl;
	}
	else
	{
		// zistim ohodnotenie kardinalit markantov pre rozdelenia reznou liniou
		fAfScore = this->minutiaeScore(cLine.getPosFCardinality(), cLine.getNegAfCardinality());
		afFScore = this->minutiaeScore(cLine.getPosAfCardinality(), cLine.getNegFCardinality());
		std::cout << "original" << std::endl;
	}

	cLine.setSeparation(fAfScore, afFScore);

	const auto sO = oNum / oNom;
	const auto sV = vNum / vNom;
	sM = std::max(fAfScore, afFScore);

	const auto eval = this->oWeight * sO + this->vWeight * sV + this->mWeight * sM;
	
	cLine
		.setSO(this->oWeight * sO)
		.setSV(this->vWeight * sV)
		.setSM(this->mWeight * sM)
		.setEval(eval);
	
	// aplikujem vahy
	return eval;
}

bool CutlineEstimator::isPositive(const Vec3f& line, const Point& pos)
{
	if (line[0] * pos.x + line[1] * pos.y + line[2] < 0)
	{
		return true;
	}
	
	return false;
}

std::vector<std::vector<Minutiae>> CutlineEstimator::minutiaeCardinality(const Vec3f& line, const std::vector<Minutiae>& minutiaes) const
{
	std::vector<std::vector<Minutiae>> posNegMin(2, std::vector<Minutiae>());

	for (auto& minutia: minutiaes)
	{
		const auto pos = minutia.getPosition();

		if (!this->adaptiveMethod || this->distance(line, pos) >= this->dmax)
		{
			if (this->isPositive(line, pos))
			{
				posNegMin[0].push_back(minutia);
			}
			else
			{
				posNegMin[1].push_back(minutia);
			}	
		}
	}

	return posNegMin;
}

float CutlineEstimator::minutiaeScore(const int positive, const int negative) const
{
	return (this->zSigmoid(positive, -15.0f, -3.0 / 10.0) + this->zSigmoid(negative, -15.0f, -3.0 / 10.0)) / 2;
}

float CutlineEstimator::minutiaeScore2(int positive, int negative) const
{
	// evaluation od sides
	const auto posEval = this->zSigmoid(positive, -15.0f, -3.0 / 10.0);
	const auto negEval = this->zSigmoid(negative, -15.0f, -3.0 / 10.0);

	// f1 score
	return 2 * (posEval * negEval) / (posEval + negEval);

	/* not working, one of first tries
	const auto posEval = this->zSigmoid(positive, -15.0f, -3.0 / 10.0);
	const auto negEval = this->zSigmoid(negative, -15.0f, -3.0 / 10.0);

	const auto diff = std::abs(posEval - negEval);
	
	const auto strength = this->zSigmoid(diff, 12, -2.0 / 5.0);
	
	return strength * (posEval + negEval) / 2;*/
}

float CutlineEstimator::zSigmoid(const int v, const float u, const float t) const
{
	return 1 / (1 + exp(t * (v + u)));
}

void CutlineEstimator::renderMinutiaes(const std::vector<Minutiae>& minutiaes,
                                       const Point& offset, const Scalar& color, Mat& container) const
{
	const auto identificatorSize = 5;
	const auto length = 20;

	for (auto const& minutiae : minutiaes)
	{
		if (minutiae.isFake()) continue;
		
		auto pos = minutiae.getPosition();

		const auto type = minutiae.getType();
		const auto dir = minutiae.getDirection();

		if (type == Minutiae::Type::TERMINATION)
		{
			pos.x += offset.x;
			pos.y += offset.y;

			const auto pos2 = Point(pos.x + length * cos(dir), pos.y + length * sin(dir));

			circle(container, pos, identificatorSize, color, 1);
			line(container, pos, pos2, color, 2);
		}
		else if (type == Minutiae::Type::BIFURCATION)
		{
			pos.x += offset.x;
			pos.y += offset.y;

			const auto pos2 = Point(pos.x + length * cos(dir), pos.y + length * sin(dir));
			
			rectangle(container,
				Point(pos.x - identificatorSize, pos.y - identificatorSize),
				Point(pos.x + identificatorSize, pos.y + identificatorSize),
				color, 1
			);
			line(container, pos, pos2, color, 2);
		}
	}
}

Mat CutlineEstimator::getCutlineImage(const AlignedFingerprint& af, const Fingerprint& f)
{
	auto aligned = af.getAlignment();

	auto afPos = FingerprintAligner::aFingPos(aligned[0], aligned[1]);
	auto fPos = FingerprintAligner::fingPos(aligned[0], aligned[1]);

	const auto o = f.getOrientations();
	const auto oa = af.getOrientations();

	const auto width = FingerprintAligner::unitLength(Vec2f(afPos.x, fPos.x), af.getAligned().cols, f.cols);
	const auto height = FingerprintAligner::unitLength(Vec2f(afPos.y, fPos.y), af.getAligned().rows, f.rows);

	const auto bb = FingerprintAligner::overlay(fPos, afPos, f.size(), af.getAligned().size());

	Point p1;
	Point p2;
	if (abs(this->cutline[1]) < 0.01)
	{
		p1 = Point(-this->cutline[2] / this->cutline[0], 0);
		p2 = Point(p1.x, height);
	}
	else
	{
		p1 = Point(0, -this->cutline[2] / this->cutline[1]);
		p2 = Point(width, (-this->cutline[0] * width - this->cutline[2]) / this->cutline[1]);
	}

	const Mat a = Mat::ones(height, width, CV_32F);
	const Mat b = Mat::ones(height, width, CV_32F);

	f.copyTo(a(Rect(fPos, Size(f.cols, f.rows))));
	af.getAligned().copyTo(b(Rect(afPos, Size(af.getAligned().cols, af.getAligned().rows))));

	const auto alpha = 0.5f;
	Mat display;
	addWeighted(a, alpha, b, (1.0 - alpha), 0.0, display);

	Mat tmp(display.size(), CV_8UC3);
	ImageProcessor::convertTo(display, tmp, CV_8U);

	auto blockSize = f.getBlockSize();
	for (auto k = bb[0].y + blockSize / 2; k < bb[1].y; k += blockSize)
	{
		for (auto l = bb[0].x + blockSize / 2; l < bb[1].x; l += blockSize)
		{
			const auto r1 = oa.at<Vec2f>(k - afPos.y, l - afPos.x)[1];
			const auto r2 = o.at<Vec2f>(k - fPos.y, l - fPos.x)[1];

			const auto d = this->distance(this->cutline, Point(l, k));

			if (d <= this->dmax && r1 > .0f && r2 > .0f)
			{
				rectangle(
					tmp,
					Point(l - blockSize / 2, k - blockSize / 2),
					Point(l + blockSize / 2, k + blockSize / 2),
					Scalar(0, 120.0f / 255.0f, 0),
					2
				);
			}
		}
	}

	line(tmp, p1, p2, Scalar(0, 0, 0), 2);

	if (this->displayMinutiaes)
	{
		auto filtered = this->filterMorphedMinutiaes(af, f, false);
		const auto mins = filtered[0];
		const auto aMins = filtered[1];
		
		const auto yellow = Scalar(0, 1, 120.0f / 255.0f);
		this->renderMinutiaes(mins, fPos, yellow, tmp);

		const auto violet = Scalar(1, 120.0f / 255.0f, 0);
		this->renderMinutiaes(aMins, afPos, violet, tmp);
	}

	return tmp;
}

void CutlineEstimator::display(AlignedFingerprint& af, Fingerprint& f, const std::string& trace)
{
	const auto image = this->getCutlineImage(af, f);
	
	std::stringstream ss; ss << ++displayed << ": Cutline" << " TRACE: " << trace;
	imshow(ss.str(), image);
}

void CutlineEstimator::write(const AlignedFingerprint& af, const Fingerprint& f, const std::string& path)
{
	auto image = this->getCutlineImage(af, f);

	image.convertTo(image, CV_8UC3, 255);

	std::stringstream ss; ss << path << ".jpg";
	imwrite(ss.str(), image);
}

std::array<std::vector<Minutiae>, 2> CutlineEstimator::filterMorphedMinutiaes(const AlignedFingerprint& af, const Fingerprint& f, const bool adaptPosition)
{
	const auto o = f.getOrientations();
	const auto oa = af.getOrientations();

	auto alignment = af.getAlignment();

	const auto afPos = FingerprintAligner::aFingPos(alignment[0], alignment[1]);
	const auto fPos = FingerprintAligner::fingPos(alignment[0], alignment[1]);

	const auto bb = FingerprintAligner::overlay(fPos, afPos, f.size(), af.getAligned().size());

	auto minutiaes = f.getMinutiae();
	this->morphedAreaMinutiaes(minutiaes, fPos, bb, afPos, fPos, oa, o, adaptPosition);

	auto alignedMinutiaes = af.getMinutiae();
	this->morphedAreaMinutiaes(alignedMinutiaes, afPos, bb, afPos, fPos, oa, o, adaptPosition);

	std::array<std::vector<Minutiae>, 2> mins = { minutiaes, alignedMinutiaes };
	
	return mins;
}

void CutlineEstimator::morphedAreaMinutiaes(std::vector<Minutiae>& minutiaes,
	const Point& offset, const std::vector<Point>& bb, const Point& afPos, const Point& fPos,
	const Mat& oa, const Mat& o, const bool adaptPosition)
{
	auto minutiae = minutiaes.begin();
	while (minutiae != minutiaes.end())
	{
		auto pos = minutiae->getPosition();

		// shiftnem marknato o offset zarovnavaneho odtlacku
		pos.x += offset.x;
		pos.y += offset.y;

		// indikator zmazania, pre markant mimo oblasti
		auto erase = false;

		if (pos.x > bb[0].x && pos.x < bb[1].x
			&& pos.y > bb[0].y && pos.y < bb[1].y)
		{
			const auto r1 = oa.at<Vec2f>(pos.y - afPos.y, pos.x - afPos.x)[1];
			const auto r2 = o.at<Vec2f>(pos.y - fPos.y, pos.x - fPos.x)[1];

			if (!(r1 > .0f && r2 > .0f))
			{
				// som mimo oblasti, indikujem mazanie
				erase = true;
			}
			else if (adaptPosition)
			{
				// som v oblasti, premapujem poziciu v ramci zarovnania
				pos.x -= bb[0].x;
				pos.y -= bb[0].y;
				minutiae->setPosition(pos);
			}
		}
		else
		{
			// som mimo oblasti, indikujem mazanie
			erase = true;
		}

		if (erase)
		{
			minutiae = minutiaes.erase(minutiae);
		}
		else
		{
			++minutiae;
		}
	}
}
