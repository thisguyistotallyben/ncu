// Author:  Benjamin Johnson
// Date:    2 December, 2017
// Version: 0.2
// Purpose: Provide an abstraction over ncurses to aid in
//          relatively simple command line inteface creation


#ifndef NCU_H
#define NCU_H

#include <string>
#include <vector>
#include <ncurses.h>
#include <panel.h>
#include <map>
#include <cstdarg>

using namespace std;


enum borderType {
    NCU_BORDER_BOX,
    NCU_BORDERLESS_BOX,
    NCU_BASIC_INPUT,
    NCU_NO_CHANGE // users: don't use this
};

enum subType {
    NCU_SUB_TITLE_OUTPUT,
    NCU_SUB_TITLE_INPUT,
    NCU_SUB_OUTPUT,
    NCU_SUB_INPUT
};

class SubElement {
    public:
        string id;
        string text;
        SubElement *link;
};

class Element {
    public:
        string id;
        string title;
        borderType btype;
        WINDOW *win;
        PANEL  *panel;

        bool visible;

        int width;
};

class Group {
    public:
        string id;
        vector<string> elements;
};

class NCU {
    public:
        // alpha and omega
        void start();
        void end();

        // subelement stuff
        void addSubElement(string id, subType, ...);

        // element stuff
        void addElement(string id, borderType bt,
                        int sizex, int sizey,
                        int posx, int posy);
        void borderElement(string id, borderType bt);
        void addTitle(string id, string title);
        void showElement(string id);
		void hideElement(string id);
        void clearElement(string id);
        void updateElement(string id);

        // group stuff
        void addGroup(string id, int num, ...);
        void showGroup(string id);
		void hideGroup(string id);

        // i/o
        void write(string id, string data, int posx, int posy);
        void cwrite(string id, string data, int posx, int posy);
        string read(string id);

        // utilities
        void hideCursor();
        void wait(char key);
        int width();
        int height();
		int alert(string s);

    private:
        void check_if_started();
        WINDOW* getWin(string id);
        Element* getElement(string id);
        PANEL* getPanel(string id);
        void borderElement(string id);

        map<string, Element*> elementList;
        map<string, Group*> groupList;
        bool NCU_STARTED = false;
        bool cursor = true;
        string focus;
        int r = 0;
        int c = 0;

		// alert message
		WINDOW *alertw;
		PANEL *alertp;
};

#endif
