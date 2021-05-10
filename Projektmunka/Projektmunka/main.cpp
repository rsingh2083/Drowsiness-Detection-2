#include <iostream>

#include "DrowsinessDetector.h"

int main()
{
	try
	{
		DrowsinessDetector det;
		det.start("0.mov");
		det.start("5.mov");
		det.start("10.mov");
	}
	catch (Exception e)
	{
		cout << e.what() << endl;
		return -1;
	}
	return 0;
}
