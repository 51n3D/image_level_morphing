#pragma once

#include <exception>
#include <string>

namespace exception
{
	class NoImageFoundException final : public std::exception
	{
		std::string msg;

	public:
		NoImageFoundException() : msg("Image not found!") {}

		const char* what() const noexcept override
		{
			return msg.c_str();
		}
	};
}
