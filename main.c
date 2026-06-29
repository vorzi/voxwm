#include <X11/cursorfont.h>
#include <X11/keysym.h>

#include <X11/Xutil.h>
#include <X11/Xlib.h>
#include <unistd.h>

#include <stdio.h>

#include <math.h>
#include "config.h"
#include "wm.h"

int idx = -1;

Window active = None;
Window old_active = None;

int offset_x = 0;
int offset_y = 0;

int minr_x = 100;
int minr_y = 100;

int start_x = 0;
int start_y = 0;

int wi = 0;
int h = 0;

int xerror(Display *d, XErrorEvent * e) {
  (void)d;
  printf("Error code: %d\n", e -> error_code);
  printf("Resource: %lu\n", e -> resourceid);
  printf("Request: %d\n", e->request_code);
  printf("Minor: %d\n", e->minor_code);
  return 0;
}

/* Existent Clients */
int ec(void) {
  int count = 0;

  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (w[i].win != None)
      count++;
  }

  return count;
}

/* Stack Layout */
void sl(const Arg *arg) {
  (void)arg;
  int t = ec();

  if (t == 0)
    return;

  if (master_factor < 0.1) master_factor = 0.1;
  if (master_factor > 0.9) master_factor = 0.9;

  if (t == 1) {
    XMoveResizeWindow(
      d,
      w[0].win,
      0 + gap,
      0 + gap,
      wi - (gap * 2),
      h - (gap * 2)
    );
    return;
  }

  int master_w = wi * master_factor;
  int stack_w = wi - master_w;
  int stack_h = h / (t - 1);

  XMoveResizeWindow(
    d,
    w[0].win,
    0 + gap,
    0 + gap,
    master_w - (gap * 2),
    h - (gap * 2)
  );

  for (int i = 1; i < t; i++) {
    XMoveResizeWindow(
      d,
      w[i].win,
      master_w + gap,
      (i - 1) * stack_h + gap,
      stack_w - (gap * 2),
      stack_h - (gap * 2)
    );
  }
}

/* Windows Grid */
void wg(const Arg *arg) {
  (void)arg;
  int total = ec();

  if (total == 0)
    return;

  int cols = ceil(sqrt(total));
  int rows = ceil((double) total / cols);

  int cell_w = wi / cols;
  int cell_h = h / rows;

  int tile = 0;

  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (w[i].win == None)
      continue;

    int col = tile % cols;
    int row = tile / cols;

    int x = col * cell_w;
    int y = row * cell_h;

    int wi = (col == cols - 1) ? (wi - x) : cell_w;
    int h = (row == rows - 1) ? (h - y) : cell_h;

    XMoveResizeWindow(d, w[i].win, x + gap, y + gap, wi - (gap * 2), h - (gap * 2));

    tile++;
  }
}

/* Neartest Client */
int nc(int current) {
  for (int i = 1; i <= MAX_CLIENTS; i++) {
    int id = (current + i) % MAX_CLIENTS;

    if (w[id].win != None) {
      return id;
    }
  }

  return -1;
}

int pc(int current) {
  for (int i = 1; i <= MAX_CLIENTS; i++) {
    int id = (current - i + MAX_CLIENTS) % MAX_CLIENTS;

    if (w[id].win != None) {
      return id;
    }
  }

  return -1;
}

int fci(Window win) {
  /* Find Client Index */
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (w[i].win == win) {
      return i;
    }
  }
  return -1;
}

int mci(Window win) {
  Window current = win;

  while (current != None) {
    int index = fci(current);
    if (index != -1) {
      return index;
    }

    Window root_return;
    Window parent_return;
    Window *children = NULL;
    unsigned int nchildren = 0;

    if (!XQueryTree(d, current, &root_return, &parent_return, &children, &nchildren)) {
      return -1;
    }

    if (children != NULL) {
      XFree(children);
    }

    if (parent_return == current || parent_return == None) {
      break;
    }

    current = parent_return;
  }

  return -1;
}

int ffci(void) {
  /* Find Free Client Index */
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (w[i].win == None) {
      return i;
    }
  }
  return -1;
}

void grabbers(void) {
  unsigned int modifiers[] = {0, LockMask, Mod2Mask, LockMask | Mod2Mask};

  for (unsigned int i = 0; i < nkeys; i++) {
    KeyCode code = XKeysymToKeycode(d, keys[i].key);
    if (code == 0) continue;

    for (unsigned int j = 0; j < sizeof(modifiers) / sizeof(modifiers[0]); j++) {
      XGrabKey(
        d,
        code,
        keys[i].mod | modifiers[j],
        root,
        True,
        GrabModeAsync,
        GrabModeAsync
      );
    }
  }
}

void grabbuttons(Window win) {
  XGrabButton(
    d,
    Button1,
    MODKEY,
    win,
    False,
    ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
    GrabModeAsync,
    GrabModeAsync,
    None,
    None
  );

  XGrabButton(
    d,
    Button3,
    MODKEY,
    win,
    False,
    ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
    GrabModeAsync,
    GrabModeAsync,
    None,
    None
  );
}

/* Up is functions  */

void execsh(const Arg *arg)
{
  if (fork() == 0) {
    setsid();
    execl("/bin/sh", "sh", "-c", arg->s, NULL);
  }
};

void quit(const Arg *arg) /* Quit, Breaks a window */ 
{
  (void)arg;
  if (idx != -1 && idx < MAX_CLIENTS) {
      if (old_active == w[idx].win) old_active = None;
        XDestroyWindow(d, w[idx].win);
        w[idx].win = None;
        w[idx].mode = NONE;

              int c = MAX_CLIENTS;
              w[c - 1].win = None;

              int n = nc(c);
              if (n != -1) {
                active = w[n].win;
                idx = n;
              } else {
                active = None;
                idx = -1;
              };

              int near = nc(idx);
              Client wind = w[near];

              active = wind.win;

              w[c - 1].win = None;
              w[c - 1].mode = NONE;
  };
};

void tfc(const Arg *arg) /* Toggle FullScreen */
{
  (void)arg;
  XWindowAttributes attr;
  if (idx < 0 || idx >= MAX_CLIENTS) return;
  int active = w[idx].win;
  XGetWindowAttributes(d, active, & attr);
    if (!w[idx].fullscreen) {
        w[idx].old_x = attr.x;
        w[idx].old_y = attr.y;
        w[idx].old_w = attr.width;
        w[idx].old_h = attr.height;
        int sw = DisplayWidth(d, DefaultScreen(d));
        int sh = DisplayHeight(d, DefaultScreen(d));

              XMoveResizeWindow(
                d,
                active,
                0,
                0,
                sw,
                sh
              );
              w[idx].fullscreen = 1;
            } else {
              XMoveResizeWindow(
                d,
                active,
                w[idx].old_x,
                w[idx].old_y,
                w[idx].old_w,
                w[idx].old_h
              );
              w[idx].fullscreen = 0;
            };
}

void cww(const Arg *arg) /* Create White Window */
{
  (void)arg;
  int i = ffci();
        if (i != -1) {
          printf("I -> %d", i);

          w[i].win = XCreateSimpleWindow(
            d,
            root,
            100 + (30 * i),
            100 + (30 * i),
            800,
            600,
            3,
            0x000000,
            0xffffff
          );

          w[i].fullscreen = 0;
          w[i].old_x = 0;
          w[i].old_y = 0;
          w[i].old_w = 500;
          w[i].old_h = 200;
          w[i].mode = NONE;

          XSelectInput(d, w[i].win, EnterWindowMask | StructureNotifyMask | PropertyChangeMask);
          grabbuttons(w[i].win);

          XMapWindow(d, w[i].win);
          XFlush(d);

          active = w[i].win;
          idx = i;
        }
};

/* master factor add */
void mfa(const Arg *arg)
{
  (void)arg;
    if (master_factor < master_factor_max) {
      master_factor += master_factor_min;
    } else {
      master_factor = master_factor_max;
    }

    sl(NULL);
};

/* master factor remove */
void mfr(const Arg *arg)
{
  (void)arg;
  if (master_factor > master_factor_min) {
      master_factor -= master_factor_min;
    } else {
      master_factor = master_factor_min;
    };

    sl(NULL);
};

/* Next Focus */
void nf(const Arg *arg) 
{
  (void)arg;
  idx = fci(active);
    if (idx != -1) {
      int n = nc(idx);

      if (n == -1) return;

      idx = n;
      active = w[idx].win;

      XSetInputFocus(d, active, RevertToParent, CurrentTime);
      XRaiseWindow(d, active);
    }
};

/* To center (Move window to center) */
void tc(const Arg *arg) 
{
  (void)arg;
  if (w[idx].fullscreen) return;

  int sw, sh;
  sw = DisplayWidth(d, DefaultScreen(d));
  sh = DisplayHeight(d, DefaultScreen(d));

  XWindowAttributes attr;
  XGetWindowAttributes(d, active, & attr);

  int nx, ny;
  nx = (sw - attr.width) / 2;
  ny = (sh - attr.height) / 2;

  XMoveWindow(d, active, nx, ny);
};

int main() {
  d = XOpenDisplay(NULL);
  XSetErrorHandler(xerror);
  KeySym key;
  for (int k = 0; k < MAX_CLIENTS; k++) {
    w[k].win = None;
    w[k].mode = NONE;
    w[k].fullscreen = 0;
  }

  if (!d) {
    puts("Error, X server Running?");
    return 1;
  };

  root = DefaultRootWindow(d);

  Cursor cursor = XCreateFontCursor(d, XC_left_ptr);
  XDefineCursor(d, root, cursor);

  wi = DisplayWidth(d, DefaultScreen(d));
  h = DisplayHeight(d, DefaultScreen(d));

  XSelectInput(d, root, KeyPressMask | SubstructureRedirectMask | SubstructureNotifyMask);
  grabbers(); /* Grab the keys, just that! */
  XEvent e;

  while (1) {
    XNextEvent(d, & e);

    if (e.type == MapRequest) {
      Window win = e.xmaprequest.window;
      printf("MapRequest");

      int i = ffci();

      if (i == -1) continue;

      w[i].win = win;
      w[i].mode = NONE;
      w[i].fullscreen = 0;

      XSelectInput(
        d,
        win,
        EnterWindowMask |
        StructureNotifyMask |
        PropertyChangeMask
      );
      grabbuttons(win);

      XMapWindow(d, win);
      XSetInputFocus(d, win, RevertToPointerRoot, CurrentTime);

      active = win;
      idx = i;
    };

    if (e.type == ConfigureRequest) {
    XConfigureRequestEvent *ev = &e.xconfigurerequest;

      int i = fci(ev->window);

      if (i != -1) continue;
    XWindowChanges wc;

    wc.x = ev->x;
    wc.y = ev->y;
    wc.width = ev->width;
    wc.height = ev->height;
    wc.border_width = ev->border_width;
    wc.sibling = ev->above;
    wc.stack_mode = ev->detail;

    XConfigureWindow(
        d,
        ev->window,
        ev->value_mask,
        &wc
    );
};

    if (e.type == DestroyNotify) {
      Window win = e.xdestroywindow.window;

      int i = fci(win);

      if (i != -1) {
        w[i].win = None;
        w[i].mode = NONE;
      }
    }

    if (e.type == ButtonPress) {
      active = e.xbutton.window;
      if (active != old_active) {
        if (old_active != None) XSetWindowBorder(d, old_active, 0x000000);
        if (active != None) XSetWindowBorder(d, active, 0xff0000);
        old_active = active;
      }
      idx = mci(active);
      if (idx != -1) {

        active = w[idx].win;

        XSetInputFocus(d, active, RevertToPointerRoot, CurrentTime);
        XRaiseWindow(d, active);

        if ((e.xbutton.state & MODKEY) && e.xbutton.button == Button1) {

            w[idx].mode = MOVE;

            offset_x = e.xbutton.x;
            offset_y = e.xbutton.y;


            XGrabPointer(
                d,
                root,
                False,
                PointerMotionMask |
                ButtonReleaseMask,
                GrabModeAsync,
                GrabModeAsync,
                None,
                None,
                CurrentTime
            );

        } else if ((e.xbutton.state & MODKEY) && e.xbutton.button == Button3) {
          w[idx].mode = RESIZE;
          start_y = e.xbutton.y_root;
          start_x = e.xbutton.x_root;

          XGrabPointer(
        d,
        root,
        False,
        PointerMotionMask | ButtonReleaseMask,
        GrabModeAsync,
        GrabModeAsync,
        None,
        None,
        CurrentTime
    );
        };
      }
    };

    if (e.type == EnterNotify) {
      idx = mci(e.xcrossing.window);
      if (idx != -1) {
        active = w[idx].win;
        if (active != old_active) {
          if (old_active != None) XSetWindowBorder(d, old_active, 0x000000);
          XSetWindowBorder(d, active, 0xff0000);
          old_active = active;
        }
        XSetInputFocus(d, active, RevertToPointerRoot, CurrentTime);
      }
    }

    if (e.type == KeyPress) {
      key = XLookupKeysym( &
        e.xkey,
        0
      );

      for (unsigned int i = 0; i < nkeys; i++) {
        if (keys[i].key == key &&
            (e.xkey.state & keys[i].mod)) {

            keys[i].func(&keys[i].arg);
        }
      }
    };

    if (e.type == MotionNotify && active != None && idx != -1) {
      if (w[idx].mode == RESIZE) {
        XWindowAttributes attr;
        XGetWindowAttributes(d, active, & attr);

        int dx = e.xmotion.x_root - start_x;
        int dy = e.xmotion.y_root - start_y;

        int new_w = attr.width + dx;
        int new_h = attr.height + dy;

        if (new_w < minr_x) new_w = minr_x;
        if (new_h < minr_y) new_h = minr_y;

        XResizeWindow(d, active, new_w, new_h);

        start_x = e.xmotion.x_root;
        start_y = e.xmotion.y_root;
      } else if (w[idx].mode == MOVE) {
        XMoveWindow(
          d,
          active,
          e.xmotion.x_root - offset_x,
          e.xmotion.y_root - offset_y
        );
      }
      XFlush(d);
    };

    if (e.type == ButtonRelease) {
      if (idx != -1) w[idx].mode = NONE;

      XUngrabPointer(d, CurrentTime);
    }
  };

  XCloseDisplay(d);
  return 0;
}
