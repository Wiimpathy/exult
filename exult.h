/*
 *  Copyright (C) 2001-2013  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef EXULT_H
#define EXULT_H

#include "mouse.h"

class Actor;
class Cheat;
class Configuration;
class Game_window;
class KeyBinder;
class Tile_coord;
class Paintable;
class GameManager;

/*
 *  Get a click, or, optionally, a keyboard char.
 *
 *  Output: 0 if user hit ESC.
 *      Chr gets keyboard char., or 0 if it's was a mouse click.
 */
extern bool g_waiting_for_click;

extern int Get_click(
    int &x, int &y,         // Location returned (if not ESC).
    Mouse::Mouse_shapes shape,  // Mouse shape to use.
    char *chr = 0,          // Char. returned if not null.
    bool drag_ok = false,       // Can drag while here.
    Paintable *paint = 0,       // Paint over everything else.
    bool rotate_colors = false      // If the palette colors should rotate.
);

/*
 *  Make a screenshot of the current screen display
 */
extern void make_screenshot(
    bool silent = false     // If false, will display a success/failure message
);

/*
 *  Wait for someone to stop walking.  If a timeout is given, at least
 *  one animation cycle will still always occur.
 */

extern void Wait_for_arrival(
    Actor *actor,           // Whom to wait for.
    Tile_coord dest,        // Where he's going.
    long maxticks = 0       // Max. # msecs. to wait, or 0.
);

extern void change_gamma(bool down);
extern void increase_scaleval();
extern void decrease_scaleval();
extern void setup_video(bool fullscreen, int setup_video_type,
                        int resx = 0, int resy = 0, int gw = 0,
                        int gh = 0, int scaleval = 0, int scaler = 0,
                        Image_window::FillMode fillmode = Image_window::Fill,
                        int fill_scaler = 0);

enum setup_video_type {
    VIDEO_INIT = 0,         // read and write initial settings and create gwin
    TOGGLE_FULLSCREEN = 1,  // toggle fullscreen using proper settings
    MENU_INIT = 2,          // read and set initial settings for video gump menu
    SET_CONFIG = 3          // sets the config settings (doesn't write)
};

typedef enum {
    QUIT_TIME_NO = 0,
    QUIT_TIME_YES = 1,
    QUIT_TIME_RESTART = 2
} quitting_time_enum;

extern KeyBinder *keybinder;
extern Configuration *config;
extern GameManager *gamemanager;

extern quitting_time_enum quitting_time;

#ifdef UNDER_CE
extern std::string WINCE_exepath;
extern bool minimized;
extern class Keyboard_gump *gkeyboard;
extern class clsTouchscreen *Touchscreen;
#endif

extern class ShortcutBar_gump *g_shortcutBar;
#ifdef __IPHONEOS__
extern class KeyboardButton_gump *gkeybb;
#endif

#ifdef GEKKO
#include <SDL.h>
extern void push_keyboard(SDLKey key, bool pressed);
extern void push_mousemotion(Sint16 *mousecoords);
extern int Wii_handle_event(SDL_Event *event, Sint16 *mousecoords);
extern void get_memory();

#define WIIMOTE    0
#define GCPAD      1

#define WII_BUTTON_A             (0)
#define WII_BUTTON_B             (1)
#define WII_BUTTON_1             (2)
#define WII_BUTTON_2             (3)
#define WII_BUTTON_MINUS         (4)
#define WII_BUTTON_PLUS          (5)
#define WII_BUTTON_HOME          (6)
#define NUNCHUCK_BUTTON_C        (7)
#define NUNCHUCK_BUTTON_Z        (8)
#define GC_BUTTON_A              (0)
#define GC_BUTTON_B              (1)
#define GC_BUTTON_X              (2)
#define GC_BUTTON_Y              (3)
#define GC_BUTTON_Z              (4)
#define GC_BUTTON_R              (5)
#define GC_BUTTON_L              (6)
#define GC_BUTTON_START          (7)
#define CLASSIC_BUTTON_A         (9)
#define CLASSIC_BUTTON_B         (10)
#define CLASSIC_BUTTON_X         (11)
#define CLASSIC_BUTTON_Y         (12)
#define CLASSIC_BUTTON_L         (13)
#define CLASSIC_BUTTON_R         (14)
#define CLASSIC_BUTTON_ZL        (15)
#define CLASSIC_BUTTON_ZR        (16)
#define CLASSIC_BUTTON_MINUS     (17)
#define CLASSIC_BUTTON_PLUS      (18)
#define CLASSIC_BUTTON_HOME      (19)
#endif

#endif
