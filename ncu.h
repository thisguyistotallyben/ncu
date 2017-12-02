// Author:  Benjamin Johnson
// Date:    2 December, 2017
// Version: 0.2
// Purpose: Provide an abstraction over ncurses to aid in
//          relatively simple command line inteface creation


#ifndef NCU_H
#define NCU_H

#include <string>
#include <vector>
#include <map>

using namespace std;


enum elementType {
    NCU_BASIC_BOX,
    NCU_BASIC_INPUT
};

class Element {
    public:
    private:
        string name;
};

class NCU {
    public:
        void start();
        void end();

        void addElement(string name, elementType type,
                        int sizex, int sizey,
                        int posx, int posy);
    private:
        map<string, Element*> elementList;
};

#endif
