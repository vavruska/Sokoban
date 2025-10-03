#ifndef _DISPLAY_H
#define _DISPLAY_H

void DrawString(int x, int y, char *text);

extern void ClearScreen(void);
/* Clear the main window */

extern void RedisplayScreen(void);
/* 
   Redisplay the main window. Has to handle the help screens if one
   is currently active.
*/

extern void ShowScreen(void);
/*
    Draw all the neat little pictures and text onto the working pixmap
    so that RedisplayScreen is happy.
*/

extern void MapChar(char, int, int);
/* 
   Draw a single pixmap, translating from the character map to the pixmap
   rendition. If "copy_area", also push the change through to the actual window.
*/

extern void DisplaySave(void);
extern void DisplayMoves(void);
extern void DisplayPushes(void);
/*
    Display these three attributes of the current game.
*/

extern int DisplayScores(int *);
/*
   Display scores. Return E_ENDGAME if user wanted to quit from here.
   If user middle-clicked on a level, and "newlev" is non-zero, put
   the level clicked-on into "newlev".
*/

extern void ShowHelp(void);
/* 
   Display the first help page, and flip help pages (one per key press)
   until a return is pressed.
*/

extern void HelpMessage(void);
/*
    Remind the user how to get help. Currently just beeps because the
    help message is always displayed.
*/

extern bool WaitForEnter(void);
/*
    Wait for the enter key to be pressed.
*/

#endif /* _DISPLAY_H */

