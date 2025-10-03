#include <stdbool.h>
#include "config.h"
#ifndef __GLOBAL_H__
#define __GLOBAL_H__
/*****************************************************************************\
 *  Stuff in this file shouldn't ever need to be changed.                    *
\*****************************************************************************/

#define BUFSIZE 256

/* internal object representation */
#define   player	'@'
#define   playerstore	'+'
#define   store		'.'
#define   packet	'$'
#define   save		'*'
#define   ground	' '
#define   wall		'#'

/* maximum possible size of a board */
#define MAXROW		25
#define MAXCOL		40

/* player position for movement */
typedef struct {
   int x, y;
} POS;

/* list of possible errors */
#define E_FOPENSCREEN	1
#define E_PLAYPOS1	2
#define E_ILLCHAR	3
#define E_PLAYPOS2	4
#define E_TOMUCHROWS	5
#define E_TOMUCHCOLS	6
#define E_ENDGAME	7
#define E_FOPENSAVE	9
#define E_WRITESAVE	10
#define E_STATSAVE	11
#define E_READSAVE	12
#define E_ALTERSAVE	13
#define E_SAVED		14
#define E_TOMUCHSE	15
#define E_FOPENSCORE	16
#define E_READSCORE	17
#define E_WRITESCORE	18
#define E_NOSAVEFILE	23
#define E_ABORTLEVEL    29


#define MOVE_HISTORY_SIZE 4096
/* The number of moves that are remembered for temp saves and
   verifies. */

/*** Global state ***/
typedef char Map[MAXROW+1][MAXCOL+1];

extern Map map;

extern int rows, cols, left, top, level, moves, pushes, savepack, packets;
extern unsigned int scorelevel, scoremoves, scorepushes;
extern POS ppos;
extern bool datemode, headermode;
extern char *localuser;

extern char move_history[MOVE_HISTORY_SIZE];
/* The characters "move_history[0..moves-1]" describe the entire
   sequence of moves applied to this level so far, in a format
   compatible with "Verify".  */

#endif
