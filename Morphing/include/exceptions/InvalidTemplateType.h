#pragma once

#include <exception>
#include <string>

namespace exception
{
	class InvalidTemplateType final : public std::exception
	{
		std::string msg;

	public:
		InvalidTemplateType() : msg("Invalid type of template. Please choose from TemplateGenerator::Type!") {}

		const char* what() const noexcept override
		{
			return msg.c_str();
		}
	};
}
