#pragma once

#include <string>


class Exception
{
public:
	inline Exception(std::string msg = "") : _msg(msg) {}
	inline std::string what() const { return _msg; }

private:
	std::string _msg;
};
