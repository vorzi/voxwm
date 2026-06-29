#ifndef CONFIG_H
#define CONFIG_H

#include <stddef.h>
#include <X11/X.h>
#include <X11/keysym.h>
#include "main.h"
#include "wm.h"

#define MODKEY Mod1Mask 

float master_factor = 0.6;
float master_factor_max = 0.9;
float master_factor_min = 0.1;
int gap = 2;

static const Key keys[] = {
    /* Mod                 Tecla         Função      Argumento */

    { MODKEY,              XK_Return,       execsh,        {.s = "alacritty"}    },
    { MODKEY,              XK_n,            cww,           {0} },
    { MODKEY,              XK_space,        sl,            {0} },
    { MODKEY|ShiftMask,    XK_q,            quit,          {0} },
};

static const unsigned int nkeys =
    sizeof(keys) / sizeof(keys[0]);

#endif