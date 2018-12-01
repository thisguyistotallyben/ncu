# NCU - NCursesUtils

Provides a second, simpler layer on top of ncurses that streamlines code writing by creating elements, groups, and several functions.

### How it works

All interface related actions must take place between the `start()` and `end()` functions.  `start()` handles the many ncurses setup procedures.  `end()` cleans up everything and exits ncurses.  If you do not call `end()`, your command line will be messed up.

```
// initialize NCU class
NCU ncu;

ncu.start();

...

ncu.end();
```

### Structure

NCU deviates from ncurses' naming scheme:

**Elements** are areas that show or do things.  In ncurses, these are called windows.  However, these elements can have pre-determined uses, such as a scrolling box or a text input.

**Groups** are groups of elements.

You can show or hide individual elements or whole groups.

Addionally, all elements and groups are created with string IDs.  This allows for readable code and takes the burden of storage off the programmer.

### Object creation

Elements are created with the `addElement(id, type, width, height, x, y)` command.  To show these elements, the `showElement(id)` command must be called.

Groups are created with `addGroup(id, number_of_elements, ...)`.  To show a whole group, call `showGroup(id)`.

### Interaction

To interact with a text box, call `read(id)` which focuses the text box and returns a string of text.

# The Future

I intend to implement form data, scroll boxes, tab moving, and probably plenty more than that.  I'll have a more cemented list soon.

# Compiling
Compile with flags: -std=c++11 -lpanel -lncurses
