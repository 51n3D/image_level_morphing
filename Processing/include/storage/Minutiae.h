#pragma once

#include <opencv2/opencv.hpp>

namespace processing
{
	namespace utils
	{
		namespace storage
		{
			class Minutiae
			{
			private:
				// members
				/**
				 * \brief Pozicia markantu.
				 */
				cv::Point position;
				/**
				 * \brief Smer markantu.
				 */
				float direction = -1;
				/**
				 * \brief Dlzka tracovania markantu.
				 */
				float threshold = 10;
				/**
				 * \brief Typ markantu.
				 */
				int type = NONE;
				/**
				 * \brief Indikator falosnosti.
				 */
				bool fake = false;
				/**
				 * \brief Definicia falosnej strukury proti sebe stojacich markantov.
				 */
				std::map<int, std::vector<Minutiae>> facingPairs{ { Type::TERMINATION, std::vector<Minutiae>() }, { Type::BIFURCATION, std::vector<Minutiae>() } };
				/**
				 * \brief Definicia falosnej struktury prepojenych markantov.
				 */
				std::map<int, std::vector<Minutiae>> connectedPairs{ { Type::TERMINATION, std::vector<Minutiae>() }, { Type::BIFURCATION, std::vector<Minutiae>() } };
				
			public:
				enum Type { NONE = -1, BIFURCATION = 1, TERMINATION };

				// constructors
				Minutiae() = default;
				Minutiae(cv::Point& position, const float direction, const float threshold, const int type)
					: position(position), direction(direction), threshold(threshold), type(type) {}

				// methods
				/**
				 * \brief Zisti ci by mohol byt markant falosny na zaklade obsihnutych dat.
				 * \return falosnost markantu
				 */
				bool canBeFake();
				/**
				 * \brief Zisti uhol voci inemu markantu.
				 * \param m iny markant
				 * \return uhol
				 */
				float betaValueWith(const Minutiae& m) const;
				/**
				 * \brief Zisti ci je markant vydlicova struktura.
				 * \return ci je vidlicova struktura
				 */
				bool isFork();
				/**
				 * \brief Zisti ci je struktura Bur.
				 * \return ci je struktura Bur
				 */
				bool isBur();
				/**
				 * \brief Prida proti sebe stojaci markant do informacii o falosnej
				 * strukture, na zaklade typu markantu predavaneho parametrom.
				 * \param minutiae 
				 * \return referenciu na this
				 */
				Minutiae& addN(const Minutiae& minutiae);
				/**
				 * \brief Prida prepojeny markant do informacii o falosnej
				 * strukture, na zaklade typu markantu predavaneho parametrom.
				 * \param minutiae
				 * \return referenciu na this
				 */
				Minutiae& addC(const Minutiae& minutiae);
				/**
				 * \brief Odoberie proti sebe stojaci markant z informacii o falosnej
				 * strukture, na zaklade typu markantu predavaneho parametrom.
				 * \param minutiae
				 * \return referenciu na this
				 */
				Minutiae& stripN(const Minutiae& minutiae);
				/**
				 * \brief Odoberie prepojeny markant z informacii o falosnej
				 * strukture, na zaklade typu markantu predavaneho parametrom.
				 * \param minutiae
				 * \return referenciu na this
				 */
				Minutiae& stripC(const Minutiae& minutiae);
				/**
				 * \brief Odoberie markant z mapy.
				 * \param map mapa
				 * \param minutiae markant
				 */
				void strip(std::map<int, std::vector<Minutiae>>& map, const Minutiae& minutiae) const;
				
				// getters
				cv::Point getPosition() const { return this->position; }
				float getDirection() const { return this->direction; }
				int getType() const { return this->type; }
				bool isFake() const { return this->fake; }
				float getThreshold() const { return this->threshold; }
				int getN(const int type) { return this->facingPairs[type].size(); }
				int getC(const int type) { return this->connectedPairs[type].size(); }
				std::vector<Minutiae> getFacing(const int type) { return this->facingPairs[type]; }
				std::vector<Minutiae> getConnected(const int type) { return this->connectedPairs[type]; }
				
				// setters
				Minutiae& setPosition(const cv::Point& position) { this->position = position; return *this; }
				Minutiae& setDirection(const float direction) { this->direction = direction; return *this; }
				Minutiae& setType(const int type) { this->type = type; return *this; }
				Minutiae& setFake(const bool fake = true) { this->fake = fake; return *this; }
				Minutiae& setThreshold(const float threshold) { this->threshold = threshold; return *this; }
				
				// operators
				bool operator == (const Minutiae& m) const {
					return (position == m.getPosition() && type == m.getType() && direction == m.getDirection());
				}
				bool operator != (const Minutiae& m) const {
					return (position != m.getPosition() || type != m.getType() || direction != m.getDirection());
				}
			};
		}
	}
}


inline bool processing::utils::storage::Minutiae::canBeFake()
{
	return (
		this->getN(Type::TERMINATION) != 0 
		|| this->getN(Type::BIFURCATION) != 0 
		|| this->getC(Type::TERMINATION) != 0 
		|| this->getC(Type::BIFURCATION) != 0
	);
}

inline float processing::utils::storage::Minutiae::betaValueWith(const Minutiae& m) const
{
	const auto v1 = abs(static_cast<double>(this->getDirection() - m.getDirection()));
	const auto v2 = CV_2PI - v1;

	return std::min(v1, v2);
}

inline bool processing::utils::storage::Minutiae::isFork()
{
	switch (this->type)
	{
		case Type::TERMINATION:
		{
			if (this->getN(Type::BIFURCATION) != 0)
			{
				return true;
			}

			break;
		}	
		case Type::BIFURCATION:
		{
			if (this->getC(Type::BIFURCATION) != 0 
				|| this->getN(Type::TERMINATION) != 0)
			{
				return true;
			}

			break;
		}
		default:
			return false;
	}
	
	return false;
}

inline bool processing::utils::storage::Minutiae::isBur()
{
	switch (this->type)
	{
	case Type::TERMINATION:
	{
		if (this->getC(Type::BIFURCATION) != 0)
		{
			return true;
		}

		break;
	}
	case Type::BIFURCATION:
	{
		if (this->getC(Type::TERMINATION) != 0)
		{
			return true;
		}

		break;
	}
	default:
		return false;
	}

	return false;
}

inline processing::utils::storage::Minutiae& processing::utils::storage::Minutiae::addN(const Minutiae& minutiae)
{
	this->facingPairs[minutiae.getType()].emplace_back(minutiae);
	
	return *this;
}

inline processing::utils::storage::Minutiae& processing::utils::storage::Minutiae::addC(const Minutiae& minutiae)
{
	this->connectedPairs[minutiae.getType()].emplace_back(minutiae);

	return *this;
}

inline processing::utils::storage::Minutiae& processing::utils::storage::Minutiae::stripN(const Minutiae& minutiae)
{
	this->strip(this->facingPairs, minutiae);
	
	return *this;
}

inline processing::utils::storage::Minutiae& processing::utils::storage::Minutiae::stripC(const Minutiae& minutiae)
{
	this->strip(this->connectedPairs, minutiae);
	
	return *this;
}

inline void processing::utils::storage::Minutiae::strip(std::map<int, std::vector<Minutiae>>& map, const Minutiae& minutiae) const
{
	const auto type = minutiae.getType();
	
	if (std::find(map[type].begin(), map[type].end(), minutiae) != map[type].end())
	{
		map[type].erase(
			std::remove(map[type].begin(), map[type].end(), minutiae),
			map[type].end()
		);
	}
}
