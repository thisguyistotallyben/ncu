#include "ncu.h"
using namespace std;

int main() {
	NCU ncu;

	// setup
	ncu.start();
	ncu.hideCursor();

	// add elements
	ncu.addElement("navbar", NCU_BORDER_BOX,     ncu.width(), 3, 0, 0);
	ncu.addElement("intext", NCU_BORDERLESS_BOX, 9, 3, 0, 3);
	ncu.addElement("input",  NCU_BASIC_INPUT,    20, 3, 9, 3);
	ncu.addElement("box1",   NCU_BORDER_BOX,     ncu.width()/2, ncu.height()/2, 0, 6);

	// add group
	ncu.addGroup("group", 3, "intext", "input", "box1");

	// fill elements
	ncu.write("navbar",  "THIS IS TEXT", 2, 1);
	ncu.write("intext",  "Input",        2, 1);

	ncu.addTitle("box1", "Output Box");

	// show elements
	ncu.showElement("navbar");
	ncu.wait('a');
	ncu.showGroup("group");

	string s = ncu.read("input");
	ncu.clearElement("input");
	ncu.write("box1", s, 2, 2);

	// wait
	ncu.wait('q');

	// cleanup
	ncu.end();

	return 0;
}
