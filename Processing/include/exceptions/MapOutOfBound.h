#pragma once

#include <exception>
#include <string>

namespace exception
{
	class MapOutOfBound final : public std::exception
	{
		std::string msg;

	public:
		MapOutOfBound() : msg("Map is out of bounds!") {}

		const char* what() const noexcept override
		{
			return msg.c_str();
		}
	};
}
