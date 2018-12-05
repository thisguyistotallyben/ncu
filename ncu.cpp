// ncu.cpp
// Author:  Benjamin Johnson
// Purpose: Abstract the ncurses library to streamline command line interface creation


#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
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

void NCU::setColorScheme(string id, colorScheme cs) {
	Command *c = new Command(SET_COLOR);
	c->id = id;
	c->cScheme = cs;
	commands.push(c);
}

void NCU::internalSetColorScheme(Command *c) {
	wbkgd(getWin(c->id), COLOR_PAIR(c->cScheme));
}

// pop-ups ---------------------------------------------------------------------------


void NCU::notice(string s, int sec) {
	Command *c = new Command(NOTICE);
	c->text = s;
	c->val = sec;
	c->cScheme = NCU_BLACK_ON_WHITE;
	commands.push(c);
}

void NCU::notice(string s, int sec, colorScheme cs) {
	Command *c = new Command(NOTICE);
	c->text = s;
	c->val = sec;
	c->cScheme = cs;
	commands.push(c);
}


void NCU::internalNotice(Command *c) {
	thread tn(noticeThread, c, this);
	tn.detach();
}

void NCU::noticeThread(Command *c, NCU *ncu) {
	int nl;
	
	// setup
	ncu->mNotice.lock();
	nl = count(c->text.begin(), c->text.end(), '\n') + 1;

	ncu->addElement("NCU_DO_NOT_USE_NOTICE", NCU_BORDERLESS_BOX, COLS, nl+2, 0, 0);
	ncu->setColorScheme("NCU_DO_NOT_USE_NOTICE", c->cScheme);
	ncu->write("NCU_DO_NOT_USE_NOTICE", c->text, 2, 1);

	ncu->showElement("NCU_DO_NOT_USE_NOTICE");

	usleep(c->val*1000000);

	// cleanup
	ncu->hideElement("NCU_DO_NOT_USE_NOTICE");
	ncu->mNotice.unlock();
}


void NCU::write(string id, string s, int posx, int posy) {
	Command *c = new Command(WRITE);

	c->id = id;
	c->text = s;
	c->posx = posx;
	c->posy = posy;

	commands.push(c);
}

void NCU::internalWrite(Command *c) {
    WINDOW *win = getWin(c->id);
    if (win != NULL) mvwprintw(win, c->posy, c->posx, c->text.c_str());
	else cout << "EMPTY";
    
    // update
    update_panels();
    doupdate();
}

// thready things --------------------------------------------------------------------


void NCU::mainThread(NCU *ncu) {
	cout << "BOOP";
	PANEL *p;

	ncu->startView();

	while(1) {
		if (commands.size() == 0) continue;


		ncu->mMain.lock();
		Command *c = commands.front();
		commands.pop();

		// do things
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
				break;
			case NOTICE:
				ncu->internalNotice(c);
				break;
			case WRITE:
				ncu->internalWrite(c);
				break;
			case SET_COLOR:
				ncu->internalSetColorScheme(c);
			default:
				// literally no idea
				break;

		}
		ncu->mMain.unlock();

		// kill this thread and ncurses when ready
		// all mutexes are necessary here
		if (!ncu->NCU_STARTED) {
			ncu->mNotice.lock();
			ncu->mMain.lock();

			endwin();
			break;

			ncu->mNotice.unlock();
			ncu->mMain.unlock();
		}
	}
}

void NCU::controlThread() {
	// "Good luck"
	//   -me, probably
}

void NCU::startView() {
	mMain.lock();

    initscr();
    raw();
    keypad(stdscr, TRUE);
    start_color();
    init_pair(NCU_BLACK_ON_WHITE, COLOR_BLACK, COLOR_WHITE);
    init_pair(NCU_WHITE_ON_BLACK, COLOR_WHITE, COLOR_BLACK);
    init_pair(NCU_BLACK_ON_YELLOW, COLOR_BLACK, COLOR_YELLOW);
    init_pair(NCU_WHITE_ON_RED, COLOR_WHITE, COLOR_RED);
    init_pair(NCU_WHITE_ON_GREEN, COLOR_WHITE, COLOR_GREEN);
    //noecho();
    refresh();
    curs_set(0);
    NCU_STARTED = true;
    r = LINES;
    c = COLS;
	
	// make the notice element
	//addElement("NCU_DO_NOT_USE_NOTICE", NCU_BORDER_BOX, c, 3, 0, 0);
	//hideElement("NCU_DO_NOT_USE_NOTICE");

	mMain.unlock();
}


// utilitarian stuff -----------------------------------------------------------------


PANEL* NCU::getPanel(string id) {
    map<string, Element*>::iterator e;

    e = elementList.find(id);
    if (e != elementList.end()) return e->second->panel;
    else return NULL;
}

WINDOW* NCU::getWin(string id) {
    map<string, Element*>::iterator e;

    e = elementList.find(id);
    if (e != elementList.end()) return e->second->win;
    else return NULL;
}

int NCU::width() {
	return c;
}

int NCU::height() {
	return r;
}
