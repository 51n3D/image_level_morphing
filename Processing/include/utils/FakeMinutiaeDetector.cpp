#include "FakeMinutiaeDetector.h"
#include "MinutiaeEstimator.h"
#include "ImageProcessor.h"

using namespace processing::utils::storage;
using namespace processing::utils;
using namespace processing;
using namespace cv;

int FakeMinutiaeDetector::displayed = 0;
const std::string FakeMinutiaeDetector::class_name = "FakeMinutiaeDetector::";

bool FakeMinutiaeDetector::isFacingPair(const Minutiae& m1, const Minutiae& m2) const
{
	const auto beta = this->minutiaeAngle(m1, m2);

	return beta > (CV_PI / 2);
}

double FakeMinutiaeDetector::minutiaeAngle(const Minutiae& m1, const Minutiae& m2) const
{
	const auto v1 = abs(static_cast<double>(m1.getDirection() - m2.getDirection()));
	const auto v2 = CV_2PI - v1;
	
	return min(v1, v2);
}

bool FakeMinutiaeDetector::isAround(const Minutiae& m1, const Minutiae& m2) const
{
	const auto& p1 = m1.getPosition();
	const auto& p2 = m2.getPosition();

	const auto distance = this->distance(p1, p2);
	
	return distance < (m1.getThreshold() * 2.0f);
}

double FakeMinutiaeDetector::distance(const Point& p1, const Point& p2) const
{
	const auto& e1 = pow(p2.x - p1.x, 2);
	const auto& e2 = pow(p2.y - p1.y, 2);
	
	return sqrt(e1 + e2);
}

bool FakeMinutiaeDetector::areConnected(const Minutiae& m1, const Minutiae& m2, const Mat& map) const
{
	const auto base = m1.getPosition();
	auto positions = this->getStartingTracingPositions(m1.getPosition(), map);

	const auto length = this->maximalTracingLength(m1);

	std::vector<Point> processed = { base };

	for (const auto& actual : positions)
	{
		std::vector<Point> currents = { actual };
		processed.insert(processed.end(), actual);

		for (auto t = 0; t < length; t++)
		{
			std::vector<Point> ones;

			for (const auto& current : currents)
			{
				for (const auto index : MinutiaeEstimator::indices)
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
						if (m2.getPosition() == point)
						{
							return true;
						}

						// nasiel som
						ones.emplace_back(point);
					}
				}
			}

			// pridam momentalne prehladane body
			processed.insert(processed.end(), ones.begin(), ones.end());
			currents = ones;
		}
	}
	
	return false;
}

std::vector<Point> FakeMinutiaeDetector::getStartingTracingPositions(const Point& base, const Mat& map) const
{
	std::vector<Point> positions;

	const auto indices = MinutiaeEstimator::indices;

	// najdem prve body linie z pociatocneho
	for (auto i = 1; i <= indices.size(); i++)
	{
		const auto index = indices[i % indices.size()];

		// skontrolujem ci nie som mimo obrazku
		if ((base.y + index[0]) >= map.rows && base.x + index[1] >= map.cols) continue;

		// je to linia? je to 01 prechod? nejedna sa o bod ktory chcem vynechat?
		if (map.at<float>(base.y + index[0], base.x + index[1]) == 1
			&& map.at<float>(base.y + indices[i - 1][0], base.x + indices[i - 1][1]) == 0)
		{
			positions.emplace_back(base.x + index[1], base.y + index[0]);

			if (positions.size() == 3)
			{
				break;
			}
		}
	}

	return positions;
}

float FakeMinutiaeDetector::maximalTracingLength(const Minutiae& m) const
{
	switch (m.getType())
	{
		case Minutiae::Type::BIFURCATION:
		{
			return m.getThreshold() * 2;
		}
		case Minutiae::Type::TERMINATION:
		{
			return m.getThreshold();
		}
		default:
		{
			return 0;
		}
	}
}

void FakeMinutiaeDetector::display(const Mat& img, std::vector<Minutiae> minutiaes, const std::string& trace) const
{
	Mat tmp(img.size(), CV_8UC3);

	ImageProcessor::convertTo(img, tmp, CV_8U);

	const auto real = Scalar(0, 255, 0);
	const auto fake = Scalar(0, 0, 255);

	const auto identificatorSize = 5;

	const auto length = 20;

	for (auto& minutiae : minutiaes)
	{
		const auto type = minutiae.getType();

		const auto pos1 = minutiae.getPosition();

		const auto dir = minutiae.getDirection();
		const auto pos2 = Point(pos1.x + length * std::cos(dir), pos1.y + length * std::sin(dir));

		auto color = real;
		if (minutiae.canBeFake())
		{
			color = fake;
		}
		
		if (type == Minutiae::Type::TERMINATION)
		{
			circle(tmp, pos1, identificatorSize, color, 2);
			
		}
		else if (type == Minutiae::Type::BIFURCATION)
		{
			rectangle(tmp, Point(pos1.x - identificatorSize, pos1.y - identificatorSize), Point(pos1.x + identificatorSize, pos1.y + identificatorSize), color, 2);
		}

		line(tmp, pos1, pos2, color, 2);
	}


	ImageProcessor::resize(tmp, 500);

	std::stringstream ss; ss << ++displayed << ": FakeMinutiaeDetector" << " TRACE: " << trace;
	imshow(ss.str(), tmp);
}

void FakeMinutiaeDetector::find(const Mat& img, std::vector<Minutiae>& minutiaes) const
{
	if (minutiaes.size() > 65)
	{
		return;
	}
	
	auto i = 0;
	for (auto analyzedMinutiae = minutiaes.begin(); analyzedMinutiae != minutiaes.end(); ++analyzedMinutiae)
	{
		i++;
		for (auto minutiae = minutiaes.begin(); minutiae != minutiaes.end(); ++minutiae)
		{
			if (analyzedMinutiae == minutiae)
			{
				continue;
			}
			
			auto connected = false;
			if (this->areConnected(*analyzedMinutiae, *minutiae, img))
			{
				connected = true;
			}
			
			if (this->isAround(*analyzedMinutiae, *minutiae)
				&& this->isFacingPair(*analyzedMinutiae, *minutiae)
				&& !connected)
			{
				// pridam falosnu strukturu - voci sebe stojaci par
				analyzedMinutiae->addN(*minutiae);
				analyzedMinutiae->setFake();
			}
			else if (connected)
			{
				// pridam falosnu strukturu prepojenych
				analyzedMinutiae->addC(*minutiae);
				analyzedMinutiae->setFake();
			}
		}
	}
}

void FakeMinutiaeDetector::remove(std::vector<Minutiae>& minutiaes) const
{
	this->repairBrokenRidges(minutiaes);
	this->repairForks(minutiaes);
	this->removeBurs(minutiaes);
}

void FakeMinutiaeDetector::removeBurs(std::vector<Minutiae>& minutiaes) const
{
	for (auto minutiae = minutiaes.begin(); minutiae != minutiaes.end();)
	{
		if (minutiae->isFake() && minutiae->isBur())
		{
			auto type = Minutiae::Type::NONE;

			switch(minutiae->getType())
			{
				case Minutiae::Type::TERMINATION:
					type = Minutiae::Type::BIFURCATION;
					break;
				case Minutiae::Type::BIFURCATION:
					type = Minutiae::Type::TERMINATION;
					break;
				default:
					minutiae = minutiaes.erase(minutiae);
					continue;
			}
			
			auto minutiaesToDelete = minutiae->getConnected(type);
			minutiaesToDelete.insert(minutiaesToDelete.end(), *minutiae);
			
			minutiae = this->handleRemove(minutiaes, minutiaesToDelete);
		}
		else
		{
			++minutiae;
		}
	}
}

void FakeMinutiaeDetector::repairForks(std::vector<Minutiae>& minutiaes) const
{
	for (auto minutiae = minutiaes.begin(); minutiae != minutiaes.end();)
	{
		if (minutiae->isFake() && minutiae->isFork())
		{
			switch (minutiae->getType())
			{
				case Minutiae::Type::TERMINATION:
				{
					auto facing = minutiae->getFacing(Minutiae::Type::BIFURCATION);

					std::vector<Minutiae> fakes;
					if (!facing.empty())
					{
						const auto maxAt = this->maxBetaAt(*minutiae, facing);
						fakes.insert(fakes.end(), facing.at(maxAt));
					}

					fakes.insert(fakes.end(), *minutiae);
					
					minutiae = this->handleRemove(minutiaes, fakes);

					break;
				}
				case Minutiae::Type::BIFURCATION:
				{
					auto facingForks = minutiae->getFacing(Minutiae::Type::TERMINATION);
					auto minutiaesToDelete = minutiae->getConnected(Minutiae::Type::BIFURCATION);

					std::vector<Minutiae> fakes;
					if (!facingForks.empty())
					{
						const auto maxAt = this->maxBetaAt(*minutiae, facingForks);
						fakes.insert(fakes.end(), facingForks.at(maxAt));
					}
						
					minutiaesToDelete.insert(minutiaesToDelete.end(), fakes.begin(), fakes.end());
					minutiaesToDelete.insert(minutiaesToDelete.end(), *minutiae);
						
					minutiae = this->handleRemove(minutiaes, minutiaesToDelete);

					break;
				}
				default:
					minutiae = minutiaes.erase(minutiae);
					continue;
			}
		}
		else
		{
			++minutiae;
		}
	}
}

void FakeMinutiaeDetector::repairBrokenRidges(std::vector<Minutiae>& minutiaes) const
{
	const auto termination = Minutiae::Type::TERMINATION;
	auto maxEe = this->maxFacingPairs(minutiaes, termination, termination);

	// zacinam od markantov s najviac celiacimi markantmi
	while (maxEe > 0)
	{
		for (auto minutiae = minutiaes.begin(); minutiae != minutiaes.end();)
		{
			// je falosny? je ukoncenie? ma aktualne kontrolovany pocet celiacich markantov?
			if (minutiae->isFake() && minutiae->getType() == termination && minutiae->getN(termination) == maxEe)
			{
				auto facing = minutiae->getFacing(minutiae->getType());

				std::vector<Minutiae> fakes;
				if (!facing.empty())
				{
					const auto maxAt = this->maxBetaAt(*minutiae, facing);
					fakes.insert(fakes.end(), facing.at(maxAt));
				}

				fakes.insert(fakes.end(), *minutiae);
				
				minutiae = this->handleRemove(minutiaes, fakes);
			}
			else
			{
				++minutiae;
			}
		}

		maxEe--;
	}
}

int FakeMinutiaeDetector::maxBetaAt(const Minutiae& minutiae, const std::vector<Minutiae>& facing) const
{
	auto maxAt = 0;
	for (auto i = 0; i < facing.size(); i++)
	{
		if (minutiae.betaValueWith(facing.at(i)) > minutiae.betaValueWith(facing.at(maxAt)))
		{
			maxAt = i;
		}
	}

	return maxAt;
}

int FakeMinutiaeDetector::maxFacingPairs(std::vector<Minutiae>& minutiaes, const int m1, const int m2) const
{
	auto max = 0;
	for (auto& minutiae : minutiaes)
	{
		if (minutiae.isFake() && minutiae.getType() == m1)
		{
			if (minutiae.getN(m2) > max)
			{
				max = minutiae.getN(m2);
			}
		}
	}

	return max;
}

std::_Vector_iterator<std::_Vector_val<std::_Simple_types<Minutiae>>> FakeMinutiaeDetector::handleRemove(std::vector<Minutiae>& minutiaes, const std::vector<Minutiae>& minutiaesToDelete) const
{

	std::_Vector_iterator<std::_Vector_val<std::_Simple_types<Minutiae>>> minutiae;
	for (const auto& minutiaeToDelete : minutiaesToDelete)
	{
		minutiae = minutiaes.erase(
			std::remove(minutiaes.begin(), minutiaes.end(), minutiaeToDelete),
			minutiaes.end()
		);
	}

	this->removeFromStructures(minutiaes, minutiaesToDelete);

	return minutiae;
}

std::_Vector_iterator<std::_Vector_val<std::_Simple_types<Minutiae>>> FakeMinutiaeDetector::handleRemove(std::vector<Minutiae>& minutiaes, const Minutiae& minutiaeToDelete) const
{
	auto minutiae = minutiaes.erase(
		std::remove(minutiaes.begin(), minutiaes.end(), minutiaeToDelete),
		minutiaes.end()
	);

	this->removeFromStructures(minutiaes, minutiaeToDelete);

	return minutiae;
}

void FakeMinutiaeDetector::removeFromStructures(std::vector<Minutiae>& minutiaes, const std::vector<Minutiae>& minutiaesToDelete) const
{
	for (auto& min : minutiaes)
	{
		if (min.isFake())
		{
			for (auto& minutiaeToDelete : minutiaesToDelete)
			{
				min.stripN(minutiaeToDelete);
			}
		}
	}
}

void FakeMinutiaeDetector::removeFromStructures(std::vector<Minutiae>& minutiaes, const Minutiae& minutiaeToDelete) const
{
	for (auto& min : minutiaes)
	{
		if (min.isFake())
		{
			min.stripN(minutiaeToDelete);
		}
	}
}
