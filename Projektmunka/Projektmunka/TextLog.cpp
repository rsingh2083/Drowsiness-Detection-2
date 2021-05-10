#include "TextLog.h"
using namespace std;


void TextLog::clearScreen() {
	TextLog::log_file << "Clear" << endl;
	log_file.flush();
}

void TextLog::putInfo(string info) {
	log_file << "Info" << "\t" << info << endl;
	log_file.flush();
}

void TextLog::close()
{
	log_file.close();
}
