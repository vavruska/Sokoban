
#include <types.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

#include "globals.h"
#include "externs.h"
#include "display.h"
#include "gte.h"

extern int abs(int);
/* forward declarations */
static bool RunTo(int, int);
static void PushMan(int, int);
static void FindTarget(int, int, int);
static void MoveMan(int, int);
static void DoMove(int);
static int TestMove(word);
static void MakeMove(word);
static void MultiPushPacket(int, int);
static bool ApplyMoves(int moveseqlen, char *moveseq);

/* defining the types of move */
#define MOVE            1
#define PUSH            2
#define SAVE            3
#define UNSAVE          4
#define STOREMOVE       5
#define STOREPUSH       6

/* This value merely needs to be greater than the longest possible path length
 * making it the number of squares in the array seems a good bet. 
 */
#define BADMOVE MAXROW*MAXCOL

/* some simple checking macros to make sure certain moves are legal when 
 * trying to do mouse based movement.
 */
#define ISPLAYER(x,y) ((map[x][y] == player) || (map[x][y] == playerstore))
#define ISCLEAR(x,y) ((map[x][y] == ground) || (map[x][y] == store))
#define ISPACKET(x,y) ((map[x][y] == packet) || (map[x][y] == save))

/* Some macros useful in MultiPushPacket */
#define D_RIGHT    0
#define D_UP       1
#define D_LEFT     2
#define D_DOWN     3

#define UNVISITED (BADMOVE+1)

#define SETPACKET(x, y) map[x][y] = (map[x][y] == ground) ? packet : save
#define CLEARPACKET(x, y) map[x][y] = (map[x][y] == packet) ? ground : store
#define CAN_GO(x, y, d) \
  (ISCLEAR(x+Dx[d], y+Dy[d]) && PossibleToReach(x-Dx[d], y-Dy[d]))
#define OPDIR(d) ((d+2) % 4)

static POS tpos1, tpos2, lastppos, lasttpos1, lasttpos2;
static char lppc, ltp1c, ltp2c;
static int action, lastaction;

static bool shift = false;
static bool cntrl = false;
static bool displaying = true;
static word oldmove;
static int findmap[MAXROW+1][MAXCOL+1];

#define MAXDELTAS 4

/** For stack saves. **/
struct map_delta {
    int x,y;
    char oldchar, newchar;
};
struct move_r {
    int px, py;
    int moves, pushes, saved, ndeltas;
    struct map_delta deltas[MAXDELTAS];
};
static struct move_r move_stack[STACKDEPTH];
static int move_stack_sp; /* points to last saved move. If no saved move, -1 */
static Map prev_map;
static void RecordChange(void);
static void UndoChange(void);
static void InitMoveStack(void);
static int tempsave;
/* The move index at which a temporary save request was made */


Map map;
extern int rows, cols, left, top, level, moves, pushes, savepack, packets;
extern POS ppos;

#define XK_Up 11
#define XK_Down 10
#define XK_Left 8
#define XK_Right 21
#define XK_h 'h'
#define XK_H 'H'
#define XK_j 'j'
#define XK_J 'J'
#define XK_k 'k'
#define XK_K 'K'
#define XK_l 'l'
#define XK_L 'L'

int Play(void) {
    int ret;
    char buf[1];
    word sym, controlMask;
    unsigned long *keyModReg = (unsigned long *) 0xE0C025;

    displaying = true;
    ClearScreen();
    ShowScreen();
    InitMoveStack();
    tempsave = moves;
    ret = 0;
    while (ret == 0) {
        controlMask = GTEReadControl();
        sym = controlMask & 0x007F;
        shift = (*keyModReg) & 1;
        cntrl = (*keyModReg) & 2;
        switch (sym) {
        case 'q':
            /* q is for quit */
            if (!cntrl) {
                ret = E_ENDGAME;
            }
            break;
        case 'S':
            if (shift || cntrl) {
                ret = DisplayScores(0);
                ClearScreen();
                RedisplayScreen();
            }
            break;
        case 's':
            /* save */
            if (!cntrl && !shift) {
                ret = SaveGame();
                if (ret == 0) {
                    ret = E_SAVED;
                }
            }
            break;
        case '?':
            /* help */
            if (!cntrl) {
                ShowHelp();
                RedisplayScreen();
            }
            break;
        case 18:
            /* ^R refreshes the screen */
            RedisplayScreen();
            break;
        case 'U':
            /* Do a full screen reset */
            tempsave = moves = pushes = 0;
            ret = ReadScreen();
            InitMoveStack();
            if (ret == 0) {
                ShowScreen();
            }
            break;
        case 'u':
            UndoChange();
            ShowScreen();
            break;
        case 'O':
        {
            bool ok;
            moves = pushes = 0;
            ret = ReadScreen();
            InitMoveStack();
            ok = ApplyMoves(tempsave, move_history);
            RecordChange();
            assert(ok);
            ShowScreen();
            assert(tempsave == moves);
            break;
        }
        case 'c':
            if (!cntrl) {
                if (moves < MOVE_HISTORY_SIZE) {
                    tempsave = moves;
                } else {
                    tempsave = MOVE_HISTORY_SIZE - 1;
                }
            }
            break;
        case XK_k:
        case XK_K:
        case XK_Up:
        case XK_j:
        case XK_J:
        case XK_Down:
        case XK_l:
        case XK_L:
        case XK_Right:
        case XK_h:
        case XK_H:
        case XK_Left:
            /* Ordinary move keys */
            MakeMove(sym);
            RecordChange();
            break;
        default:
            if (buf[0]) {
                HelpMessage();
            }
            break;
        }
        while (sym) {
            controlMask = GTEReadControl();
            sym = controlMask & 0x007F;
        }
        /* if we solved a level, set it up so we get some score! */
        if ((ret == 0) && (packets == savepack)) {
            scorelevel = level;
            scoremoves = moves;
            scorepushes = pushes;
            break;
        }
    }
    return ret;
}


/* Perform a user move based on the key in "sym". */
static void MakeMove(word sym) {
    do {
        action = TestMove(sym);
        if (action != 0) {
            lastaction = action;
            lastppos.x = ppos.x;
            lastppos.y = ppos.y;
            lppc = map[ppos.x][ppos.y];
            lasttpos1.x = tpos1.x;
            lasttpos1.y = tpos1.y;
            ltp1c = map[tpos1.x][tpos1.y];
            lasttpos2.x = tpos2.x;
            lasttpos2.y = tpos2.y;
            ltp2c = map[tpos2.x][tpos2.y];
            DoMove(lastaction);
            /* store the current word so we can do the repeat. */
            oldmove = sym;
        }
    } while ((action != 0) && (packets != savepack) && (shift || cntrl));
}

/* make sure a move is valid and if it is, return type of move */
static int TestMove(word action) {
    int ret;
    char tc;
    bool stop_at_object;

    stop_at_object = cntrl;

    if ((action == XK_Up) || (action == XK_k) || (action == XK_K)) {
        tpos1.x = ppos.x - 1;
        tpos2.x = ppos.x - 2;
        tpos1.y = tpos2.y = ppos.y;
        if (moves < MOVE_HISTORY_SIZE) {
            move_history[moves] = 'k';
        }
    }
    if ((action == XK_Down) || (action == XK_j) || (action == XK_J)) {
        tpos1.x = ppos.x + 1;
        tpos2.x = ppos.x + 2;
        tpos1.y = tpos2.y = ppos.y;
        if (moves < MOVE_HISTORY_SIZE) {
            move_history[moves] = 'j';
        }
    }
    if ((action == XK_Left) || (action == XK_h) || (action == XK_H)) {
        tpos1.y = ppos.y - 1;
        tpos2.y = ppos.y - 2;
        tpos1.x = tpos2.x = ppos.x;
        if (moves < MOVE_HISTORY_SIZE) {
            move_history[moves] = 'h';
        }
    }
    if ((action == XK_Right) || (action == XK_l) || (action == XK_L)) {
        tpos1.y = ppos.y + 1;
        tpos2.y = ppos.y + 2;
        tpos1.x = tpos2.x = ppos.x;
        if (moves < MOVE_HISTORY_SIZE) {
            move_history[moves] = 'l';
        }
    }
    tc = map[tpos1.x][tpos1.y];
    if ((tc == packet) || (tc == save)) {
        if (!stop_at_object) {
            if (map[tpos2.x][tpos2.y] == ground) {
                ret = (tc == save) ? UNSAVE : PUSH;
            } else if (map[tpos2.x][tpos2.y] == store) {
                ret = (tc == save) ? STOREPUSH : SAVE;
            } else {
                ret = 0;
            }
        } else {
            ret = 0;
        }
    } else if (tc == ground) {
        ret = MOVE;
    } else if (tc == store) {
        ret = STOREMOVE;
    } else {
        ret = 0;
    }
    return ret;
}

/* actually update the internal map with the results of the move */
static void DoMove(int moveaction) {
    map[ppos.x][ppos.y] = (map[ppos.x][ppos.y] == player) ? ground : store;
    switch (moveaction) {
    case MOVE:
        map[tpos1.x][tpos1.y] = player;
        break;
    case STOREMOVE:
        map[tpos1.x][tpos1.y] = playerstore;
        break;
    case PUSH:
        map[tpos2.x][tpos2.y] = map[tpos1.x][tpos1.y];
        map[tpos1.x][tpos1.y] = player;
        pushes++;
        break;
    case UNSAVE:
        map[tpos2.x][tpos2.y] = packet;
        map[tpos1.x][tpos1.y] = playerstore;
        pushes++;
        savepack--;
        break;
    case SAVE:
        map[tpos2.x][tpos2.y] = save;
        map[tpos1.x][tpos1.y] = player;
        savepack++;
        pushes++;
        break;
    case STOREPUSH:
        map[tpos2.x][tpos2.y] = save;
        map[tpos1.x][tpos1.y] = playerstore;
        pushes++;
        break;
    }
    moves++;
    if (displaying) {
        DisplayMoves();
        DisplayPushes();
        DisplaySave();
        MapChar(map[ppos.x][ppos.y], ppos.x, ppos.y);
        MapChar(map[tpos1.x][tpos1.y], tpos1.x, tpos1.y);
        MapChar(map[tpos2.x][tpos2.y], tpos2.x, tpos2.y);
        //SyncScreen();
#ifdef HAS_USLEEP
        usleep(SLEEPLEN * 1000);
#endif
    }
    ppos.x = tpos1.x;
    ppos.y = tpos1.y;
}

/* find the intest path to the target via a fill search algorithm */
static void FindTarget(int px, int py, int pathlen) {
    if (!(ISCLEAR(px, py) || ISPLAYER(px, py))) {
        return;
    }
    if (findmap[px][py] <= pathlen) {
        return;
    }

    findmap[px][py] = pathlen++;

    if ((px == ppos.x) && (py == ppos.y)) {
        return;
    }

    FindTarget(px - 1, py, pathlen);
    FindTarget(px + 1, py, pathlen);
    FindTarget(px, py - 1, pathlen);
    FindTarget(px, py + 1, pathlen);
}

static void InitMoveStack(void) {
    move_stack_sp = -1;
    move_stack[0].moves = moves;
    move_stack[0].pushes = pushes;
    move_stack[0].saved = savepack;
    memcpy(prev_map, map, sizeof(map));
}

/* Add a record to the move stack that records the changes since the
   last map state (which is stored in "prev_map"). Update "prev_map"
   to contain the current map so the next call to "RecordChange()"
   will perform correctly.
   
   If the stack runs out of space, dump the oldest half of the
   saved moves and continue. Undoing past that point will jump
   back to the beginning of the level. If the user is using the
   mouse or any skill, should never happen.
*/
static void RecordChange(void) {
    struct move_r *r = &move_stack[++move_stack_sp];
    int x, y, ndeltas = 0;
    assert(move_stack_sp < STACKDEPTH);
    if (move_stack_sp == STACKDEPTH - 1) {
        int shift = STACKDEPTH / 2;
        memcpy(&move_stack[0], &move_stack[shift],
               sizeof(struct move_r) * (STACKDEPTH - shift));
        move_stack_sp -= shift;
        r -= shift;
    }
    r[1].moves = moves;
    r[1].pushes = pushes;
    r[1].saved = savepack;
    r[1].px = ppos.x;
    r[1].py = ppos.y;
    for (x = 0; x <= MAXROW; x++) {
        for (y = 0; y <= MAXROW; y++) {
            if (map[x][y] != prev_map[x][y]) {
                assert(ndeltas < MAXDELTAS);
                r->deltas[ndeltas].x = x;
                r->deltas[ndeltas].y = y;
                r->deltas[ndeltas].newchar = map[x][y];
                r->deltas[ndeltas].oldchar = prev_map[x][y];
                ndeltas++;
            }
        }
    }
    r->ndeltas = ndeltas;
    if (ndeltas == 0) {
        move_stack_sp--; /* Why push an identical entry? */
    }
    memcpy(prev_map, map, sizeof(map));
}

static void UndoChange(void) {
    if (move_stack_sp <= 0) {
        int ret;
        moves = pushes = 0;
        ret = ReadScreen();
        InitMoveStack();
        if (ret) {
            fprintf(stderr, "Can't read screen file\n");
            exit(-1);
        }
    } else {
        struct move_r *r = &move_stack[move_stack_sp];
        int i;
        moves = r->moves;
        pushes = r->pushes;
        savepack = r->saved;
        ppos.x = r->px;
        ppos.y = r->py;
        for (i = 0; i < r->ndeltas; i++) {
            map[r->deltas[i].x][r->deltas[i].y] = r->deltas[i].oldchar;
        }
        move_stack_sp--;
        memcpy(prev_map, map, sizeof(map));
    }
}

char move_history[MOVE_HISTORY_SIZE];

/* ApplyMoves:

    Receive a move sequence, and apply it to the current level as if
    the player had made the moves, but without doing any screen updates.
    Return true if the move sequence is well-formed; false if not.

    "moveseqlen" must be the length in characters of "moveseq".

    A well-formed move string "moveseq" is a sequence of the characters
    "h,j,k,l" and "1-9".

    [hjkl]: move the man by one in the appropriate direction
    [HJKL]: move the man by two in the appropriate direction
    [1-9]: provide a count of how many times the next move
        should be executed, divided by two. Thus, "1" means
        repeat twice, "9" means repeat 18 times.  The next
        character must be one of [hjklHJKL].
*/
static bool SingleMove(char c) {
    switch (c) {
    case 'h':
        MakeMove(XK_h); 
        break;
    case 'H':
        MakeMove(XK_h); 
        MakeMove(XK_h);
        break;
    case 'j':
        MakeMove(XK_j); 
        break;
    case 'J':
        MakeMove(XK_j); 
        MakeMove(XK_j); 
        break;
    case 'k':
        MakeMove(XK_k); 
        break;
    case 'K':
        MakeMove(XK_k); 
        MakeMove(XK_k); 
        break;
    case 'l':
        MakeMove(XK_l); 
        break;
    case 'L':
        MakeMove(XK_l); 
        MakeMove(XK_l); 
        break;
    default:
        return false;
    }
    return true;
}

static bool ApplyMoves(int moveseqlen, char *moveseq) {
    int i = 0;
    char lastc = 0;
    bool olddisp = displaying;

    displaying = false;
    shift = false;
    cntrl = false;

    while (i < moveseqlen) {
        char c = moveseq[i++];
        if (lastc && c != lastc) RecordChange();
        lastc = c;
        switch (c) {
        case 'h':
        case 'j':
        case 'k':
        case 'l':
        case 'H':
        case 'J':
        case 'K':
        case 'L':
            SingleMove(c);
            break;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        {
            int reps = c - '0';
            if (i == moveseqlen) {
                displaying = olddisp;
                return false;
            }
            c = moveseq[i++];
            lastc = tolower(c);
            while (reps--) {
                if (!SingleMove(c)) {
                    displaying = olddisp;
                    return false;
                }
            }
            break;
        }
        }
    }
    RecordChange();
    displaying = olddisp;
    return true;
}
