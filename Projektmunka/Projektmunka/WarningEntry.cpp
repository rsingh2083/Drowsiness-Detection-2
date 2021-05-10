#include "WarningEntry.h"

WarningEntry::WarningEntry(const char* s) : msg()
{
	if (strlen(s) < 100)
	{
		strcpy_s(msg, sizeof(s) * strlen(s), s);
		time = std::chrono::system_clock::now();
	}

}

std::string WarningEntry::to_String()
{
	std::stringstream ss;
	ss << this;
    return ss.str();
}
