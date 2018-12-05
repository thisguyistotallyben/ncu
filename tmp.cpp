#include <iostream>
#include "ncu.h"
#include <unistd.h>
using namespace std;

int main() {
	string s;
	NCU ncu;

	ncu.start();

	ncu.addElement("topbar", NCU_BORDER_BOX, ncu.width(), 3, 0, 0);
	ncu.addElement("box", NCU_BORDER_BOX, 40, 10, 0, 4);
	ncu.addTitle("box", "This box");
	ncu.write("topbar", "YEETIFIER-2000", 2, 1);
	ncu.write("box", "Type 'y' to yeet", 2, 3);
	ncu.showElement("box");
	ncu.showElement("topbar");
	usleep(2000000);
	ncu.notice("!!! WARNING !!!\n\nyeet", 2, NCU_WHITE_ON_GREEN);
	cin >> s;
	ncu.stop();

	return 0;
}
