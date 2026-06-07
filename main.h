#ifndef TYPES_H
#define TYPES_H

#include <X11/Xlib.h>
#include <X11/keysym.h>

/* Arg */
typedef union {
    int i;
    unsigned int ui;
    float f;
    const char *s;
    const void *v;
} Arg;

/* Key */
typedef struct {
    unsigned int mod;
    KeySym key;
    void (*func)(const Arg *);
    const Arg arg;
} Key;

/* Modes */
enum Modes {
    NONE,
    MOVE,
    RESIZE
};

/* Client */
typedef struct Client Client;

struct Client {
    Window win;
    int fullscreen;

    int old_x, old_y;
    int old_w, old_h;

    enum Modes mode;
};

#endif