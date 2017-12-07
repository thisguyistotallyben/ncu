#include "ncu.h"
using namespace std;

int main() {
	char c;
	NCU ncu;

	// setup
	ncu.start();
	ncu.hideCursor();

	// add elements
	ncu.addElement("navbar", NCU_BORDER_BOX,     ncu.width(), 3, 0, 0);
	ncu.addElement("intext", NCU_BORDERLESS_BOX, 9, 3, 0, 3);
	ncu.addElement("input",  NCU_BASIC_INPUT,    20, 3, 9, 3);
	ncu.addElement("box1",   NCU_BORDER_BOX,     ncu.width()/2, ncu.height()/2, 0, 6);
	ncu.addElement("box2",   NCU_BORDER_BOX,     ncu.width()/2, ncu.height()/2, ncu.width()/2, 6);

	// add group
	ncu.addGroup("group", 3, "intext", "input", "box1");

	// fill elements
	ncu.write("navbar",  "TEST PROGRAM: Press 's' to show some more stuff", 2, 1);
	ncu.write("intext",  "Input",        2, 1);
	ncu.write("box2",    "You typed yes, now have a box.", 2, 1);

	ncu.addTitle("box1", "Output Box");

	// show elements
	ncu.showElement("navbar");
	ncu.wait('s');
	ncu.cwrite("navbar", "TEST PROGRAM: Enter some text into the field and hit enter or ESC to cancel", 2, 1);

	// show groups
	ncu.showGroup("group");

	// get input
	string s = ncu.read("input");
	ncu.clearElement("input");

	// update box1 with text from input
	ncu.write("box1", s, 2, 2);

	// alert 
	c = ncu.alert("This is an alert.\n\nType 'y' for a box or anything for not a box");
	if (c == 'y') ncu.showElement("box2");

	// update the navbar
	ncu.cwrite("navbar", "TEST PROGRAM: Test finished. Press 'q' to exit.", 2, 1);
	
	// wait
	ncu.wait('q');

	// cleanup
	ncu.end();

	return 0;
}
