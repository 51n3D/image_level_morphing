#pragma once

#include <exception>
#include <string>

namespace exception
{
	class UnknownMorphingSeparation final : public std::exception
	{
		std::string msg;

	public:
		UnknownMorphingSeparation() : msg("Unknown separation of morphed fingerprints. Please choose from Cutline::SeparationType!") {}

		const char* what() const noexcept override
		{
			return msg.c_str();
		}
	};
}
