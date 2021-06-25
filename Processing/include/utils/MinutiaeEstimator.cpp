#include "MinutiaeEstimator.h"
#include "ImageProcessor.h"
#include "exceptions/UnknownMinutiaeType.h"
#include "exceptions/MapOutOfBound.h"

using namespace processing::utils::storage;
using namespace processing::utils;
using namespace cv;

int MinutiaeEstimator::displayed = 0;
const std::string MinutiaeEstimator::class_name = "MinutiaeEstimator::";

const std::array<std::array<int, 2>, 8> MinutiaeEstimator::indices{ {
	{0, 1}, {-1, 1},
	{-1, 0}, {-1, -1},
	{0, -1}, {1, -1},
	{1, 0}, {1, 1},
} };

std::vector<Minutiae> MinutiaeEstimator::estimate(const Mat& img)
{
	this->minutiaes = std::vector<Minutiae>();
	
	this->tmp = Mat(img.size(), CV_8UC3);
	ImageProcessor::convertTo(img, this->tmp, CV_8U);

	this->lineTracking = Mat(img.size(), CV_8UC3);
	ImageProcessor::convertTo(img, this->lineTracking, CV_8U);
	
	this->compute(img);

	return this->minutiaes;
}

void MinutiaeEstimator::display(const Mat& img, const std::vector<Minutiae>& minutiaes, const std::string& trace) const
{
	const auto tmp = this->getMinutiaeImage(img, minutiaes);
	
	std::stringstream ss; ss << ++displayed << ": Minutiaes" << " TRACE: " << trace;
	imshow(ss.str(), tmp);

	ss.str("");
	ss << ++displayed << ": Minutiaes Tracing" << " TRACE: " << trace;
	imshow(ss.str(), this->tmp);
	
	ss.str("");
	ss << ++displayed << ": Ridge Tracking" << " TRACE: " << trace;
	imshow(ss.str(), this->lineTracking);
}

void MinutiaeEstimator::write(const Mat& img, const std::vector<Minutiae>& minutiaes, const std::string& path, const std::string& filename) const
{
	auto tmp = this->tmp;
	tmp.convertTo(tmp, CV_8UC3, 255);

	std::stringstream ss; ss.str(""); ss << path << "4_tracing\\" << filename << ".jpg";
	imwrite(ss.str(), tmp);
	
	auto tmp2 = this->getMinutiaeImage(img, minutiaes);
	tmp2.convertTo(tmp2, CV_8UC3, 255);

	ss.str(""); ss.str(""); ss << path << "5_minutiae\\" << filename << ".jpg";
	imwrite(ss.str(), tmp2);
}

void MinutiaeEstimator::compute(const Mat& img)
{
	for (auto i = 1; i < img.rows - 1; i++)
	{
		for (auto j = 1; j < img.cols - 1; j++)
		{
			// ak sa nenachadzam na linii, nezaujima ma to
			if (img.at<float>(i, j) == 0) continue;

			// vezmem pixely okolia
			std::vector<float> pixels;
			pixels.push_back(img.at<float>(i - 1, j) == 1 ? 0 : 1);
			pixels.push_back(img.at<float>(i - 1, j + 1) == 1 ? 0 : 1);
			pixels.push_back(img.at<float>(i, j + 1) == 1 ? 0 : 1);
			pixels.push_back(img.at<float>(i + 1, j + 1) == 1 ? 0 : 1);
			pixels.push_back(img.at<float>(i + 1, j) == 1 ? 0 : 1);
			pixels.push_back(img.at<float>(i + 1, j - 1) == 1 ? 0 : 1);
			pixels.push_back(img.at<float>(i, j - 1) == 1 ? 0 : 1);
			pixels.push_back(img.at<float>(i - 1, j - 1) == 1 ? 0 : 1);

			// spocitam vysky 01 parov
			auto sum = 0.0f;
			for (auto n = 1; n <= pixels.size(); n++)
			{
				sum += abs(pixels.at(n % 8) - pixels.at(n - 1));
			}
			sum *= .5f;

			// ak jeden jedna sa o rozdvojenie (vychadzam zo stencenych priestorov, nie linii)
			if (sum == 1.0f)
			{
				auto pos = Point(j, i);
				if (!this->isValid(pos))
				{
					continue;
				}
		
				const auto direction = this->calculateDirection(img, pos, Minutiae::Type::BIFURCATION);
				const auto adptThreshold = this->calculateAdaptiveThreshold(img, pos, direction, true);
				const auto adaptiveDirection = this->calculateDirection(img, pos, Minutiae::Type::BIFURCATION, adptThreshold, true);
				
				auto minutiae = Minutiae(pos, adaptiveDirection, adptThreshold, Minutiae::Type::BIFURCATION);
				this->minutiaes.emplace_back(minutiae);
			}
			// ak 3 ukoncenie (vychadzam zo stencenych priestorov, nie linii)
			else if (sum >= 3.0f)
			{
				auto pos = Point(j, i);
				if (!this->isValid(pos))
				{
					continue;
				}

				const auto direction = this->calculateDirection(img, pos, Minutiae::Type::TERMINATION);
				const auto adptThreshold = this->calculateAdaptiveThreshold(img, pos, direction, true);
				const auto adaptiveDirection = this->calculateDirection(img, pos, Minutiae::Type::TERMINATION, adptThreshold, true);
				
				auto minutiae = Minutiae(pos, adaptiveDirection, adptThreshold, Minutiae::Type::TERMINATION);
				this->minutiaes.emplace_back(minutiae);
			}
		}
	}
}

float MinutiaeEstimator::calculateAdaptiveThreshold(const Mat& map, const Point& pos, const float direction, const bool verbose)
{
	const auto defaultThreshold = 7;
	
	Point ridge1, ridge2;
	float distance1 = defaultThreshold, distance2 = defaultThreshold;
	auto canSkip1 = false, canSkip2 = false;
	auto r1Failed = false, r2Failed = false;

	std::vector<Point> line;
	const auto positions = this->getStartingTracingPositions(pos, map, Point(-1, -1), true);
	const auto endPoints = this->trace(pos, positions, 15, map, line);
	
	// maximalne pojdem do vzdialenosti 50 pixelov od markantu
	for (auto bound = 1; bound < 50; bound++)
	{
		try
		{
			// hladam najblizsiu liniu
			if (!r1Failed && !canSkip1
				&& this->locationContainsRidge(map, pos, bound, direction - CV_PI / 2, ridge1, line))
			{
				if (verbose)
				{
					cv::line(this->tmp, pos, ridge1, Scalar(0, 0, 1), 2);
				}

				// nasiel som, najdem najkratsiu vzdialenost
				distance1 = this->lookForClosest(map, pos, ridge1);
				
				if (ridge1 == pos || std::find(line.begin(), line.end(), ridge1) != line.end())
				{
					r1Failed = true;
				}
				else
				{
					canSkip1 = true;
				}

				if (verbose)
				{
					cv::line(this->tmp, pos, ridge1, Scalar(0, 1, 0), 2);
				}
			}
		}
		catch (exception::MapOutOfBound&)
		{
			r1Failed = true;
		}
		

		try
		{
			// hladam najblizsiu liniu na druhej strane
			if (!r2Failed && !canSkip2
				&& this->locationContainsRidge(map, pos, bound, direction + CV_PI / 2, ridge2, line))
			{
				if (verbose)
				{
					cv::line(this->tmp, pos, ridge2, Scalar(0, 0, 1), 2);
				}

				// nasiel som, najdem najkratsiu vzdialenost
				distance2 = this->lookForClosest(map, pos, ridge2);
				
				if (ridge2 == pos || std::find(line.begin(), line.end(), ridge2) != line.end())
				{
					r2Failed = true;
				}
				else
				{
					canSkip2 = true;
				}

				if (verbose)
				{
					cv::line(this->tmp, pos, ridge2, Scalar(0, 1, 0), 2);
				}
			}
		}
		// dosiel som nakoniec obrazka, linia tam nie je
		catch (exception::MapOutOfBound&)
		{
			r2Failed = true;
		}
		
	}

	// zistim kolko mam v okoli linii a podla toho nastavim prah tracovania pre
	// odhad smeru markantu
	if (!r1Failed && !r2Failed) return defaultThreshold;
	if (r1Failed && !r2Failed) return distance2;
	if (!r1Failed && r2Failed) return distance1;

	return (distance1 + distance2) / 2;
}

float MinutiaeEstimator::lookForClosest(const Mat& map, const Point& pos, Point& ridge) const
{
	auto d = this->distance(pos, ridge);
	auto checkForCloser = true;

	while (checkForCloser)
	{
		checkForCloser = false;

		// posuvam sa po okoli, podla toho kde ziskam blizsiu vzdialenost
		for (const auto& index : indices)
		{
			const auto around = Point(ridge.x + index[1], ridge.y + index[0]);

			// skontrolujem pre istotu ci nie som mimo obrazku
			if (around.x >= map.cols || around.y >= map.rows || around.x < 0 || around.y < 0) continue;

			if (map.at<float>(around.y, around.x) == 1)
			{
				const auto newDistance = this->distance(pos, around);

				// nasiel som blizsiu vzdialenost?
				if (newDistance < d)
				{
					d = newDistance;
					ridge = around;
					checkForCloser = true;
				}
			}
		}
	}

	return d;
}

float MinutiaeEstimator::distance(const Point& p1, const Point& p2) const
{
	const auto& e1 = pow(p2.x - p1.x, 2);
	const auto& e2 = pow(p2.y - p1.y, 2);

	return sqrtf(e1 + e2);
}

Mat MinutiaeEstimator::getMinutiaeImage(const Mat& img, const std::vector<Minutiae>& minutiaes) const
{
	Mat tmp(img.size(), CV_8UC3);

	ImageProcessor::convertTo(img, tmp, CV_8U);

	const auto bColor = Scalar(1, 0, 0);
	const auto tColor = Scalar(0, 0, 1);

	const auto identificatorSize = 5;

	const auto length = 20;

	for (auto const& minutiae : minutiaes)
	{
		if (minutiae.isFake()) continue;

		const auto type = minutiae.getType();

		const auto pos1 = minutiae.getPosition();

		const auto dir = minutiae.getDirection();
		const auto pos2 = Point(pos1.x + length * std::cos(dir), pos1.y + length * std::sin(dir));

		if (type == Minutiae::Type::TERMINATION)
		{
			circle(tmp, pos1, identificatorSize, tColor, 2);
			line(tmp, pos1, pos2, tColor, 2);
		}
		else if (type == Minutiae::Type::BIFURCATION)
		{
			rectangle(tmp, Point(pos1.x - identificatorSize, pos1.y - identificatorSize), Point(pos1.x + identificatorSize, pos1.y + identificatorSize), bColor, 2);
			line(tmp, pos1, pos2, bColor, 2);
		}
	}

	return tmp;
}

bool MinutiaeEstimator::locationContainsRidge(const Mat& map, const Point& pos, const int bound, const float direction, Point& ridge, const std::vector<Point>& line) const
{
	const auto pos2 = Point(pos.x + bound * std::cos(direction), pos.y + bound * std::sin(direction));

	// ak som mimo oblasti vyhodim vynimku
	if (pos2.x >= map.cols || pos2.y >= map.rows || pos2.x < 0 || pos2.y < 0)
	{
		throw exception::MapOutOfBound();
	}

	if (map.at<float>(pos2.y, pos2.x) == 1
		&& std::find(line.begin(), line.end(), pos2) == line.end())
	{
		// nasiel som liniu
		ridge = pos2;
		return true;
	}

	// pozriem sa aj ci nahodou nie je linia v okoli
	for (const auto& index: indices)
	{
		const auto around = Point(pos2.x + index[1], pos2.y + index[0]);
		
		if (around.x >= map.cols || around.y >= map.rows || around.x < 0 || around.y < 0) continue;
		
		if (map.at<float>(around.y, around.x) == 1
			&& std::find(line.begin(), line.end(), around) == line.end())
		{
			// nasiel som liniu
			ridge = around;
			return true;
		}
	}

	ridge = Point(-1, -1);
	
	return false;
}

bool MinutiaeEstimator::isValid(const Point& pos)
{
	// sontrolujem ci sa nahodou nejedna o okraj stenceneho obrazka, potom nejde o markant
	for (const auto index : indices)
	{
		const Point seg(pos.x + index[1] * this->blockSize * 2, pos.y + index[0] * this->blockSize * 2);

		if (seg.x < 0 || seg.y < 0 
			|| seg.x >= this->segmentation.cols 
			|| seg.y >= this->segmentation.rows
			|| this->segmentation.at<Vec2f>(seg.y, seg.x)[1] == .0f)
		{
			return false;
		}
	}

	return true;
}

float MinutiaeEstimator::calculateDirection(const Mat& img, const Point& pos, const int type, const float threshold, const bool verbose)
{
	const auto positions = this->getStartingTracingPositions(pos, img, Point(-1, -1), true);

	std::vector<Point> line;
	const auto points = this->trace(pos, positions, threshold, img, line, verbose);
	
	switch (type)
	{
		case Minutiae::Type::TERMINATION:
		{
			// ide o rozdvojenie prietoru medzi liniami, spocitam priemer
			// z dvoch linii ktore su pri sebe najblizsie
			std::vector<float> directions;
			for (const auto& point: points)
			{
				const auto dir = this->direction(point, pos);
				directions.emplace_back(dir);
			}

			const auto indices = this->findSmallestDifferenceOfAngles(directions);

			const auto y = sin(directions[indices[0]]) + sin(directions[indices[1]]);
			const auto x = cos(directions[indices[0]]) + cos(directions[indices[1]]);
			const auto avgOfDirs = atan2(y, x);

			const auto normalizedDir = avgOfDirs + CV_PI > CV_PI ? avgOfDirs - CV_PI : avgOfDirs + CV_PI;
				
			return normalizedDir;
		}
		case Minutiae::Type::BIFURCATION:
		{
			// ide len o jednu liniu stenceneho priestoru, vratim jej smer
			return this->direction(points.back(), pos);
		}
		case Minutiae::Type::NONE:
		default:
			throw exception::UnknownMinutiaeType();
	}
}

std::vector<int> MinutiaeEstimator::findSmallestDifferenceOfAngles(const std::vector<float>& directions) const
{
	std::vector<int> indices[3] = { {0, 1}, {0, 2}, {1, 2} };
	
	float diffs[3];
	diffs[0] = directions.at(indices[0].at(0)) - directions.at(indices[0].at(1));
	diffs[1] = directions.at(indices[1].at(0)) - directions.at(indices[1].at(1));
	diffs[2] = directions.at(indices[2].at(0)) - directions.at(indices[2].at(1));

	auto min = 0;
	for (auto i = 0; i < 3; i++)
	{
		// normalize
		diffs[i] += diffs[i] > CV_PI ? -2 * CV_PI : diffs[i] < -CV_PI ? 2 * CV_PI : 0;
		diffs[i] = fabsf(diffs[i]);
		
		if (diffs[min] > diffs[i])
		{
			min = i;
		}
	}

	return indices[min];
}

float MinutiaeEstimator::direction(const Point& offset, const Point& center) const
{
	return atan2(offset.y - center.y, offset.x - center.x);
}

std::vector<Point> MinutiaeEstimator::getStartingTracingPositions(const Point& base, const Mat& map, const Point& except, const bool verbose)
{
	std::vector<Point> positions;

	// najdem prve body linie z pociatocneho
	for (auto i = 1; i <= indices.size(); i++)
	{
		const auto index = indices[i % indices.size()];

		// skontrolujem ci nie som mimo obrazku
		if ((base.y + index[0]) >= map.rows && base.x + index[1] >= map.cols) continue;

		// je to linia? je to 01 prechod? nejedna sa o bod ktory chcem vynechat?
		if (map.at<float>(base.y + index[0], base.x + index[1]) == 1
			&& map.at<float>(base.y + indices[i - 1][0], base.x + indices[i - 1][1]) == 0
			&& Point(base.x + index[1], base.y + index[0]) != except)
		{
			positions.emplace_back(base.x + index[1], base.y + index[0]);

			if (verbose)
			{
				circle(this->tmp, positions.back(), 1, Scalar(1, 0, 1));
			}

			if (positions.size() == 3)
			{
				break;
			}
		}
	}

	return positions;
}

std::vector<Point> MinutiaeEstimator::trace(const Point& base, std::vector<Point> positions, const int length, const Mat& map, std::vector<Point>& processed, const bool verbose)
{
	std::vector<Point> result;

	// vektorova reprezentacia linie
	processed = { base };

	for (const auto& actual : positions)
	{
		// pridam co som nasiel
		std::vector<Point> currents = { actual };
		processed.insert(processed.end(), actual);
		
		for (auto t = 0; t < length; t++)
		{
			std::vector<Point> ones;

			for (const auto& current: currents)
			{
				for (const auto index : indices)
				{
					const auto point = Point(current.x + index[1], current.y + index[0]);

					// pozriem ci nie som mimo obrazku
					if (point.y >= map.rows || point.x >= map.cols || point.y < 0 || point.x < 0) continue;

					// nie je bod v spracovanych? nie je jednym zo startovacich? nie je jeden z aktualne najdenych?
					// nie je jeden z prave prehladavanych? je to vobec linia?
					if (std::find(processed.begin(), processed.end(), point) == processed.end()
						&& std::find(positions.begin(), positions.end(), point) == positions.end()
						&& std::find(ones.begin(), ones.end(), point) == ones.end()
						&& std::find(currents.begin(), currents.end(), point) == currents.end()
						&& map.at<float>(point.y, point.x) == 1)
					{
						// nasiel som
						ones.emplace_back(point);

						if (verbose)
						{
							circle(this->tmp, point, 1, Scalar(1, 0, 1));
						}
					}
				}
			}

			// pridam momentalne prehladane body
			processed.insert(processed.end(), ones.begin(), ones.end());
			currents = ones;
		}

		// koncove body dam do vysledku
		result.emplace_back(processed.back());
	}

	return result;
}
