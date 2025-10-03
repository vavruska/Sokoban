#include <types.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "externs.h"
#include "globals.h"
#include "display.h"
#include "scores.h"

#include "gte.h"

#define DEBUG_FETCH 0

/* local functions */
static void ComputeRanks(void);
static void DrawScores(void);

static word rank[MAXSCOREENTRIES];

extern int DisplayScores_(int *newlev) {
    int ret = 0;

    ComputeRanks();
    DrawScores();

    for (;;) {
        word controlMask, sym;

        controlMask = GTEReadControl();
        sym = controlMask & 0x007F;
        switch (sym) {
        case 13:
            goto done;
        case 'q':
            ret = E_ENDGAME;
            goto done;
        default:
            break;
        }

    }
    done:
    return ret;
}

static void DrawScores(void) {
    int first_index = 0;
    int last_index = 23;
    int i;
    char *header = "Level    Moves   Pushes   Date";
    DrawString(0, 0, header);
    char *msg = "Press <Return> to continue, \"q\" to quit the game.";
    DrawString(0, 24, msg);


    for (i = first_index; i <= last_index && i < scoreentries; i++) {
        char buf[500];
        word y = i + 1;
        sprintf(buf, " %4d     %4d     %4d   %s",
                scoretable[i].lv, scoretable[i].mv, scoretable[i].ps,
                DateToASCII((time_t) scoretable[i].date));
        DrawString(0, y, buf);
    }
    GTERender(0);
}

static void ComputeRanks(void) {
    int i;
    for (i = 0; i < scoreentries; i++) {
        rank[i] = SolnRank(i, 0);
    }
}



