#include <stdbool.h>

#ifndef MOUSE_H
#define MOUSE_H

struct MouseState {
    int x;
    int y;
    bool leftButton;
    bool rightButton;
    bool centerButton;
};

void dispCursor(void);
void hideCursor(void);
void getMousePos(struct MouseState* m);

#endif // MOUSE_H