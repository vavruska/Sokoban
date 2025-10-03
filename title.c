#include <types.h>
#include <stdio.h>
#include <string.h>
#include <gsos.h>
#include <memory.h>
#include <misctool.h>
#include <orca.h>
#include <resources.h>

#include "gte.h"

#define TITLE_PATH "9:data:sokotitle24.SHR"
#define SCREEN_LOC 0xE12000

word imagePallete[16] = {
    0x0743,0x0532,0x0c87,0x0000,0x0200,0x0741,0x0520,0x0b72,0x0a51,0x0dd8,0x0555,0x0225,0x0557,0x064c,0x0fff,0x088c
};

void displayTitle(word userId) {
    GSString255 path;
    OpenRecGS openRec = { 13, 0, &path, 0 };
    IORecGS readRec = { 4 };
    RefNumRecGS closeRec = { 1 };
    Handle picHandle;
    char *pic;
    char *loc = (char *) SCREEN_LOC;

    strcpy(path.text, TITLE_PATH);
    path.length = strlen(TITLE_PATH);

    OpenGS(&openRec);
    if (toolerror()) {
        SysFailMgr(toolerror(), "\pTitle Open Failed");
    }

    picHandle = NewHandle(openRec.eof, userId, attrLocked, 0);
    pic = *picHandle;

    readRec.refNum = openRec.refNum;
    readRec.requestCount = openRec.eof;
    readRec.dataBuffer = pic;
    ReadGS(&readRec);
    if (toolerror()) {
        SysFailMgr(toolerror(), "\pTitle Read Failed");
    }

    closeRec.refNum = openRec.refNum;
    CloseGS(&closeRec); 

    GTESetPalette(0, (Pointer)(pic+32256L));

    // Copy image data to SHR screen memory
    for (int i = 0; i < 32000; i++) {
        *(loc++) = *(pic++);
    }

    //wait for a keypress
    while (!GTEReadControl() & 0x007F);
    //wait for it to be released. 
    while (GTEReadControl() & 0x007F);

}

