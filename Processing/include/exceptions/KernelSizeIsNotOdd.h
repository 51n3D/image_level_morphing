#pragma once

#include <exception>
#include <string>

namespace exception
{
	class KernelSizeIsNotOdd final : public std::exception
	{
		std::string msg;

	public:
		KernelSizeIsNotOdd() : msg("Kernel size cannot be even. Required are odd sizes. (e. g. 5, 7, 9...)") {}

		const char* what() const noexcept override
		{
			return msg.c_str();
		}
	};
}
