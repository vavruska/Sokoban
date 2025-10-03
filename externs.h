/* stuff from play.c */

extern int Play(void);
/* Play a particular level. All we do here is wait for input, and
   dispatch to appropriate routines to deal with it nicely.
*/

extern bool Verify(int, char *);
/* Determine whether the move sequence solves
   the current level. Return "_true_" if so.  Set "moves" and "pushes"
   appropriately.

   "moveseqlen" must be the number of characters in "moveseq".

   The format of "moveseq" is as described in "ApplyMoves".
*/


/* stuff from screen.c */
int ReadScreen(void);

/* stuff from save.c */
int SaveGame(void);
int RestoreGame(void);

/* stuff from scoredisp.c */
//extern int DisplayScores_(Display *, Window, int *);
/* Display scores. Return E_ENDGAME if user wanted to quit from here.
   If user middle-clicked on a level, and "newlev" is non-zero, put
   the level clicked-on into "newlev".
   */

