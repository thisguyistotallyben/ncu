#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <ncurses.h>
#include <panel.h>

using namespace std;


// command class
// I have no idea how I am going to do this,
// so bear with me here
class Command {
	public:
		string command;
		vector<string> args;
};

enum borderType {
    NCU_BORDER_BOX,
    NCU_BORDERLESS_BOX,
    NCU_BASIC_INPUT,
    NCU_NO_CHANGE // users: don't use this
};

enum formType {
    NCU_FORM_TITLE,
    NCU_FORM_NO_TITLE
};

class Form {
    public:
        string id;
        string *data;
		formType ftype;

		int posx;
		int posy;
};

class Element {
    public:
        string id;
        string title;
        borderType btype;
        WINDOW *win;
        PANEL  *panel;

		// visual things
        bool visible;
        int width;
        int sizex, sizey;
        int posx, posy;

		// form things
		map<string, Form*> formList;
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
		void stop();

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

		// pop-ups
		void notice(string s);

	private:
		// thread functions
		static void modelThread(NCU *ncu);
		static void viewThread(NCU *ncu);
		static void controlThread();

		// go-getters
        WINDOW* getWin(string id);
        Element* getElement(string id);
        PANEL* getPanel(string id);
		Form* getForm(string eid, string fid);

		// other
        void check_if_started();
        void borderElement(string id);

		// object records
        map<string, Element*> elementList;
        map<string, Group*> groupList;

		// parameters
        int r = 0;
        int c = 0;
		bool NCU_STARTED = false;
        bool cursor = true;
		
		// thread variables
		thread thModel;
		thread thView;
		thread thControl;

		mutex mMain;
		mutex mNotice;

		// states
        string focus;

		// alert message
		WINDOW *alertw;
		PANEL *alertp;
};
