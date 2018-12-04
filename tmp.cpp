#include <iostream>
#include "ncu.h"
#include <unistd.h>
using namespace std;

int main() {
	NCU ncu;

	ncu.start();


	ncu.addElement("box", NCU_BORDER_BOX, 10, 10, 0, 0);
	ncu.showElement("box");
	usleep(1000000);
	ncu.notice("yeet");
	usleep(1000000);
	ncu.stop();

	return 0;
}
