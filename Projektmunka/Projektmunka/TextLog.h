#pragma once

#include <fstream>
#include <iostream>
#include <fstream>
#include <string>

class TextLog
{
	std::ofstream log_file;
public:
	TextLog(std::string filename) : log_file(filename) {
	}

	void clearScreen();
	void putInfo(std::string info);
	void close();

};

