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

class Element {
    public:
        string id;
        string title;
        borderType btype;
        WINDOW *win;
        PANEL  *panel;

        bool visible;
};

class NCU {
    public:
        void start();
        void end();

        void addElement(string id, borderType bt,
                        int sizex, int sizey,
                        int posx, int posy);
        void addGroup(string id, ...);
        void borderElement(string id, borderType bt);
        void addTitle(string id, string title);
        void showElement(string id);
        void clearElement(string id);
        void write(string id, string data, int posx, int posy);
        string read(string id);
        void updateElement(string id);

        void hideCursor();

        void wait(char key);

        int width();
        int height();
    private:
        void check_if_started();
        WINDOW* getWin(string id);
        Element* getElement(string id);
        void borderElement(string id);

        map<string, Element*> elementList;
        bool NCU_STARTED = false;
        bool cursor = true;
        string focus;
        int r = 0;
        int c = 0;
};

#endif
