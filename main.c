#include <types.h>
#include <locator.h>
#include <memory.h>
#include <misctool.h>
#include <orca.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#include "gte.h"
#include "gteHelp.h"

#include "globals.h"
#include "errors.h"
#include "externs.h"
#include "scores.h"
#include "display.h"

static int GameLoop(void);
static void Error(int);

extern Byte tiles[];
extern Word tilesPalette[16];
/* exported globals */
bool scoring = true;
int level, packets, savepack, moves, pushes, rows, cols, left, top;
unsigned int scorelevel, scoremoves, scorepushes;
static int userlevel = 0;

char gsuser[] = "GSUSER";
char *localuser = gsuser;
POS ppos;

static int GameLoop(void) {
    int ret = 0;

    ret = GetUserLevel(&userlevel);
    if (ret) {
        userlevel = 1;
    }
    ret = RestoreGame();
    if (ret) {
        /* get where we are starting from */
        level = userlevel;
        ret = ReadScreen();
    }

    /* until we quit or get an error, just keep on going. */
    while (ret == 0) {
        ret = Play();
        if ((scorelevel > 0) && scoring) {
            int ret2;
            ret2 = Score();
            Error(ret2);
            scorelevel = 0;
        }
        if (ret == 0 || ret == E_ABORTLEVEL) {
            int newlev = 0;
            int ret2;
            ret2 = DisplayScores(&newlev);
            if (ret2 == 0) {
                if (newlev > 0 &&
#if !ANYLEVEL
                    newlev <= userlevel &&
#endif
                    1) {
                    level = newlev;
                } else {
                    if (ret == 0) {
                        level++;
                    }
                }
                ret = 0;
            } else {
                ret = ret2;
            }

        }
        if (ret == 0) {
            moves = pushes = packets = savepack = 0;
            ret = ReadScreen();
        }
    }
    return ret;
}


/* display the correct error message based on the error number given us. 
 * There are 2 special cases, E_ENDGAME (in which case we don't WANT a 
 * silly error message cause it's not really an error, and E_USAGE, in which
 * case we want to give a really nice list of all the legal options.
 */
static void Error(int err) {
    switch (err) {
    case E_FOPENSCREEN:
    case E_PLAYPOS1:
    case E_ILLCHAR:
    case E_PLAYPOS2:
    case E_TOMUCHROWS:
    case E_TOMUCHCOLS:
    case E_FOPENSAVE:
    case E_WRITESAVE:
    case E_STATSAVE:
    case E_READSAVE:
    case E_ALTERSAVE:
    case E_SAVED:
    case E_TOMUCHSE:
    case E_FOPENSCORE:
    case E_READSCORE:
    case E_WRITESCORE:
    case E_NOSAVEFILE:
        SysFailMgr(err, errmess[err]);
        break;
    default:
        if (err != E_ENDGAME && err != E_ABORTLEVEL) {
            SysFailMgr(err, errmess[0]);
        }
        break;
    }
}

void displayTitle(word userId);
void main(void) {
    Word userId;

    TLStartUp();
    TOOLFAIL("Unable to start tool locator");

    userId = MMStartUp();
    TOOLFAIL("Unable to start memory manager");

    MTStartUp();
    TOOLFAIL("Unable to start misc tools");

    startupGTE(userId);

    displayTitle(userId);

    GTESetPalette(0, (Pointer)tilesPalette);
    GTELoadTileSet(0, 160, tiles);   /* Load in the tiles */

    GameLoop();

    shutdownGTE();
    MTShutDown();
    MMShutDown(userId);
    TLShutDown();
}
