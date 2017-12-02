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

using namespace std;


enum elementType {
    NCU_BORDER_BOX,
    NCU_BORDERLESS_BOX,
    NCU_BASIC_INPUT
};

class Element {
    public:
        string id;
        elementType etype;
        WINDOW *win;
        PANEL  *panel;

        bool visible;
};

class NCU {
    public:
        void start();
        void end();

        void addElement(string id, elementType type,
                        int sizex, int sizey,
                        int posx, int posy);
        void showElement(string id);
        void write(string id, string data, int posx, int posy);
        int cols();
        int rows();
    private:
        void check_if_started();
        WINDOW* getWin(string id);

        map<string, Element*> elementList;
        bool NCU_STARTED = false;
        string focus;
        int r = 0;
        int c = 0;
};

#endif
