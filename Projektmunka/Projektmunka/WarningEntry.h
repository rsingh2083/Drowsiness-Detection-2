#pragma once
#include <string>
#include <chrono>

class WarningEntry
{
public:
	WarningEntry(const char* s);
	std::string to_String();
private:
	char msg[100];
	std::chrono::system_clock::time_point time;
};

