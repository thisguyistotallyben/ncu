#include <iostream>
#include "ncu.h"
using namespace std;

int main() {
	NCU ncu;

	ncu.start();

	ncu.notice("yeet");

	ncu.stop();

	return 0;
}
