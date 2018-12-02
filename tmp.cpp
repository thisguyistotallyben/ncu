#include <iostream>
#include "ncu.h"
using namespace std;

int main() {
	int curpos, strpos;
	string s, sa, sb, sn;
	NCU ncu;

	// setup
	curpos = 0;
	strpos = 0;
	s = "";
	
	ncu.start();

	ncu.addElement("t", NCU_BASIC_INPUT, 40, 3, 0, 0);
	ncu.addElement("b", NCU_BORDER_BOX, 40, 40, 0, 3);

	ncu.showElement("t");
	ncu.showElement("b");

	while(1) {
		ncu.clearElement("t");
		s = ncu.testRead("t", "b");
		if (s == "q") break;
	}

	ncu.end();

	return 0;
}
