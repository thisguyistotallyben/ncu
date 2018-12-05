// ncu.cpp
// Author:  Benjamin Johnson
// Purpose: Abstract the ncurses library to streamline command line interface creation


#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <ncurses.h>
#include <panel.h>
#include <unistd.h>
#include "ncu.h"

using namespace std;


// global bois
queue<Command*> commands;


Command::Command(commandType ct) {
	command = ct;
}

Command::Command() {}


// alpha and omega -------------------------------------------------------------------


void NCU::start() {
	// start model thread
	thMain = thread(mainThread, this);
	
	// start controller thread
	thControl = thread(controlThread);
}

void NCU::stop() {
    map<string, Element*>::iterator eit;

	// tell the threads to stop
	Command *c = new Command;
	c->command = KILL;
	commands.push(c);

	// wait for the threads to stop
	thMain.join();
	thControl.join();
    
    // no memory leaks here... probably
    for (eit = elementList.begin(); eit != elementList.end(); eit++) {
        delete eit->second;
    }
}


// element things --------------------------------------------------------------------


/* USER FUNCTIONS */


// addElement
// creates a new element and adds it to the element list
void NCU::addElement(string id, borderType bt, int sizex, int sizey, int posx, int posy) {
	Command *c = new Command(CREATE_ELEMENT);

	// fill in info
	c->id = id;
	c->bType = bt;
	c->x = sizex;
	c->y = sizey;
	c->posx = posx;
	c->posy = posy;

	commands.push(c);
}

void NCU::removeElement(string id) {
	Command *c = new Command(REMOVE_ELEMENT);
	c->id = id;
	commands.push(c);
}

// showElement
// initially shows an element
void NCU::showElement(string id) {
	Command *c = new Command(SHOW_ELEMENT);
	c->id = id;
	commands.push(c);
}

// hideElement
// hides an element
void NCU::hideElement(string id) {
	Command *c = new Command;
	c->command = HIDE_ELEMENT;
	c->id = id;
	commands.push(c);
}


/* ACTUAL WORK FUNCTIONS */


void NCU::internalAddElement(Command *c) {
    Element *e;

    // error checking
    //check_if_started();

    // setup
    e = new Element;

    // fill in info
    e->id = c->id;
    e->title = "";
    e->btype = c->bType;
    e->width = c->x;

    // dimensions
    e->sizex = c->x;
    e->sizey = c->y;
    e->posx = c->posx;
    e->posy = c->posy;

    // win creation
    e->win = newwin(c->y, c->x, c->posy, c->posx);
    wrefresh(e->win);
    e->panel = new_panel(e->win);
    hide_panel(e->panel);

    // add to map
    elementList.insert(make_pair(c->id, e));

    // make the proper borders
    switch(e->btype) {
        case NCU_BORDERLESS_BOX:
            wborder(e->win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
            break;
        case NCU_BORDER_BOX:
            box(e->win, 0, 0);
            break;
        case NCU_BASIC_INPUT:
            wborder(e->win, ' ', ' ', ' ', 0, ' ', ' ', 0, 0);
            break;
        default: break;
    }
}

void NCU::internalRemoveElement(Command *c) {
    map<string, Element*>::iterator eit;
    
	eit = elementList.find(c->id);
	if (eit == elementList.end()) return;
	delete eit->second;
	elementList.erase(eit);
}

void NCU::internalShowElement(Command *c) {
	PANEL *p;

	p = getPanel(c->id);
	if (p != NULL) show_panel(p);

	// update
	update_panels();
	doupdate();
}

void NCU::internalHideElement(Command *c) {
	PANEL *p;

	p = getPanel(c->id);
	if (p != NULL) hide_panel(p);

	// update
	update_panels();
	doupdate();
}


// pop-ups ---------------------------------------------------------------------------


void NCU::notice(string s, int sec) {
	mNotice.lock();
	thread tn(noticeThread, s, sec, this);
	tn.join();
	mNotice.unlock();
}

void NCU::noticeThread(string s, int sec, NCU *ncu) {
	Command *c = new Command;

	// create the box
	ncu->addElement("NCU_DO_NOT_USE_NOTICE", NCU_BORDER_BOX, ncu->width(), 3, 0, 0);
	
	ncu->showElement("NCU_DO_NOT_USE_NOTICE");

	usleep(sec*1000000);

	ncu->hideElement("NCU_DO_NOT_USE_NOTICE");
	ncu->removeElement("NCU_DO_NOT_USE_NOTICE");
}


// thready things --------------------------------------------------------------------


void NCU::mainThread(NCU *ncu) {
	PANEL *p;

	ncu->startView();
	//cout << "model\n";

	while(1) {
		if (commands.size() == 0) continue;


		Command *c = commands.front();
		commands.pop();

		// do things
		ncu->mMain.lock();
		switch(c->command) {
			case KILL:
				ncu->NCU_STARTED = false;
				break;
			case CREATE_ELEMENT:
				ncu->internalAddElement(c);
				break;
			case REMOVE_ELEMENT:
				ncu->internalRemoveElement(c);
				break;
			case SHOW_ELEMENT:
				ncu->internalShowElement(c);
				break;
			case HIDE_ELEMENT:
				ncu->internalHideElement(c);
			default:
				// literally no idea
				break;

		}
		ncu->mMain.unlock();

		// kill this thread and ncurses  when ready
		if (!ncu->NCU_STARTED) {
			// get all the mutexes
			ncu->mNotice.lock();
			ncu->mMain.lock();

			endwin();
			break;

			// release the kraken
			ncu->mNotice.unlock();
			ncu->mMain.unlock();
		}
	}
}

void NCU::controlThread() {

}

void NCU::startView() {
	mMain.lock();

    initscr();
    raw();
    keypad(stdscr, TRUE);
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_WHITE);
    init_pair(2, COLOR_BLACK, COLOR_YELLOW);
    init_pair(3, COLOR_WHITE, COLOR_RED);
    init_pair(4, COLOR_GREEN, COLOR_BLACK);
    noecho();
    refresh();
    curs_set(0);
    NCU_STARTED = true;
    r = LINES;
    c = COLS;
	
	// make the notice element

	mMain.unlock();
}


// utilitarian stuff -----------------------------------------------------------------


PANEL* NCU::getPanel(string id) {
    map<string, Element*>::iterator e;

    e = elementList.find(id);
    if (e != elementList.end()) return e->second->panel;
    else return NULL;
}

int NCU::width() {
	return c;
}

int NCU::height() {
	return r;
}
