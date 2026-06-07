#include <X11/Xlib.h>
#include "main.h"

extern int idx;

extern Window active;
extern Window old_active;

extern int offset_x;
extern int offset_y;

extern int minr_x;
extern int minr_y;

extern int start_x;
extern int start_y;

extern int wi, h;

Display *d;
Window root;
#define MAX_CLIENTS 100

Client w[MAX_CLIENTS];

/* Down here is functions, dont touch it please :D */

void execsh(const Arg *arg);
void quit(const Arg *arg);
void tfc(const Arg *arg);
void cww(const Arg *arg);
void mfa(const Arg *arg);
void mfr(const Arg *arg);
void sl(const Arg *arg);
void wg(const Arg *arg);
void nf(const Arg *arg);
void tc(const Arg *arg);