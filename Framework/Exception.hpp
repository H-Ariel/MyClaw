#pragma once

#include <string>


class Exception
{
public:
	Exception(const std::string& msg = "") : message(msg) {}

	std::string message;
};
