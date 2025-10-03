#include <types.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "externs.h"
#include "globals.h"
#include "help.h"
#include "display.h"
#include "scores.h"

#include "gte.h"

static void MakeHelpWindows(void);
static int PickWall(int, int);
static void DisplayLevel(void);
static void DisplayPackets(void);
static void DisplayHelp(void);

#define STATUSLINE 0
#define HELPLINE 24

enum {
    goal = 17,
    floor = 18,
    man = 19,
    saveman = 20,
    treasure = 21,
    object = 22,
    blank = 23,
};

unsigned bit_width, bit_height;
static char buf[500];

/* returns and index into the fancy walls array. works by assigning a value
 * to each 'position'.. the type of fancy wall is computed based on how
 * many neighboring walls there are.
 */
int PickWall(int i, int j) {
    int ret = 0;

    if (i > 0 && map[i - 1][j] == wall) ret += 1;
    if (j < cols && map[i][j + 1] == wall) ret += 2;
    if (i < rows && map[i + 1][j] == wall) ret += 4;
    if (j > 0 && map[i][j - 1] == wall) ret += 8;
    return ret + 1;
}

int GetObjectTile(int i, int j, char c) {
    switch (c) {
    case player:
        return man;
    case playerstore:
        return saveman;
    case store:
        return goal;
    case save:
        return treasure;
    case packet:
        return object;
    case wall:
        return PickWall(i, j);
    case ground:
        return floor;
    default:
        return blank;
    }
}


void ClearScreen(void) {
    GTEFillTileStore(floor);
    GTERender(0);
}

void ShowScreen(void) {
    int i, j;

    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols && map[i][j] != 0; j++) {
            GTESetTile(left + j, top + i, GetObjectTile(i, j, map[i][j]));
        }
    }
    DisplayLevel();
    DisplayPackets();
    DisplaySave();
    DisplayMoves();
    DisplayPushes();
    DisplayHelp();
    GTERender(0);
}

void MapChar(char c, int i, int j) {
    int z = GetObjectTile(i, j, c);
    //GTESetTile(j, i, z);
    GTESetTile(left + j, top + i, z);
    GTERender(0);
}


void RedisplayScreen(void) {
    ShowScreen();
}


void DrawString(int x, int y, char *text) {
    char c;
    word tile;
    for (int i = 0; i < strlen(text); i++) {
        c = text[i];
        tile = 33 + c;
        GTESetTile(x + i, y, tile);
    }
}

/* The following routines display various 'statusline' stuff (ie moves, pushes,
 * etc) on the screen.  they are called as they are needed to be changed to
 * avoid unnecessary drawing */
void DisplayLevel(void) {
    sprintf(buf, "Level:%2d", level);
    DrawString(5, STATUSLINE, buf);
}

void DisplayPackets(void) {
    return;
    sprintf(buf, "Packets: %3d", packets);
    DrawString(12, STATUSLINE, buf);
}

void DisplaySave(void) {
    return;
    sprintf(buf, "Saved: %3d", savepack);
    DrawString(26, STATUSLINE, buf);
}

void DisplayMoves(void) {
    sprintf(buf, "Moves:%4d", moves);
    DrawString(15, STATUSLINE, buf);
}

void DisplayPushes(void) {
    sprintf(buf, "Pushes:%3d", pushes);
    DrawString(26, STATUSLINE, buf);
}

void DisplayHelp(void)
{
  DrawString(0, HELPLINE, "Press ? for help.");
}

bool WaitForEnter(void)
{
    word controlMask, sym;

    controlMask = GTEReadControl();
    sym = controlMask & 0x007F;

    return sym == 13 ? true : false;
}

void ShowHelp(void)
{
  int i = 0;
  bool done = FALSE;

  ClearScreen();
  while (help_pages[i].textline) {
      DrawString(help_pages[i].xpos, help_pages[i].ypos, help_pages[i].textline);
      i++;
  }
  GTERender(0);

  while(!done) {
    done = WaitForEnter();
  }
  GTEFillTileStore(floor);
  ShowScreen();
}

int DisplayScores(int *newlev) {
    int ret;
    ClearScreen();
    ret = DisplayScores_(newlev);
    return ret;
}

void HelpMessage(void) {
    RedisplayScreen();
}


