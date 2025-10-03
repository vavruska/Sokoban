#include <types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>

#include "externs.h"
#include "globals.h"

static FILE *fd;
static char *sfname;

/* save out a game in a standard format.  Uses the ntoh functions and hton
 * functions so that the same save file will work on an MSB or LSB system
 * other than that, it is a standard sokoban score file.
 */
int SaveGame(void) {
    int ret = 0;

    sfname = malloc(strlen(SAVEPATH) + 10);
    sprintf(sfname, "%s:Savefile", SAVEPATH);

    if ((fd = fopen(sfname, "w")) == NULL) {
        ret = E_FOPENSAVE;
    } else {
        if (fwrite(&(map[0][0]), 1, MAXROW * MAXCOL, fd) != MAXROW * MAXCOL) {
            ret = E_WRITESAVE;
        } else if (fwrite(&ppos, 1, sizeof(POS), fd) != sizeof(POS)) {
            ret = E_WRITESAVE;
        } else if (fwrite(&level, 1, 2, fd) != 2) {
            ret = E_WRITESAVE;
        } else if (fwrite(&moves, 1, 2, fd) != 2) {
            ret = E_WRITESAVE;
        } else if (fwrite(&pushes, 1, 2, fd) != 2) {
            ret = E_WRITESAVE;
        } else if (fwrite( &packets, 1, 2, fd) != 2) {
            ret = E_WRITESAVE;
        } else if (fwrite(&savepack, 1, 2, fd) != 2) {
            ret = E_WRITESAVE;
        } else if (fwrite(&rows, 1, 2, fd) != 2) {
            ret = E_WRITESAVE;
        } else if (fwrite(&cols, 1, 2, fd) != 2) {
            ret = E_WRITESAVE;
        } else if (fwrite( &left, 1, 2, fd) != 2) {
            ret = E_WRITESAVE;
        } else if (fwrite(&top, 1, 2, fd) != 2) {
            ret = E_WRITESAVE;
        } else {
            fclose(fd);
        }
    }

    if ((ret == E_WRITESAVE) || (ret == E_STATSAVE)) {
        remove(sfname);
    }

    free(sfname);
    return ret;
}

/* loads in a previously saved game */
int RestoreGame(void) {
    int ret = 0;

    sfname = malloc(strlen(SAVEPATH) + 10);
    sprintf(sfname, "%s:Savefile", SAVEPATH);

    if ((fd = fopen(sfname, "r")) == NULL) {
        ret = E_FOPENSAVE;
    } else {
        if (fread(&(map[0][0]), 1, MAXROW * MAXCOL, fd) != MAXROW * MAXCOL) {
            ret = E_READSAVE;
        } else if (fread(&ppos, 1, sizeof(POS), fd) != sizeof(POS)) {
            ret = E_READSAVE;
        } else if (fread( &level, 1, 2, fd) != 2) {
            ret = E_READSAVE;
        } else if (fread(&moves, 1, 2, fd) != 2) {
            ret = E_READSAVE;
        } else if (fread(&pushes, 1, 2, fd) != 2) {
            ret = E_READSAVE;
        } else if (fread(&packets, 1, 2, fd) != 2) {
            ret = E_READSAVE;
        } else if (fread(&savepack, 1, 2, fd) != 2) {
            ret = E_READSAVE;
        } else if (fread(&rows, 1, 2, fd) != 2) {
            ret = E_READSAVE;
        } else if (fread(&cols, 1, 2, fd) != 2) {
            ret = E_READSAVE;
        } else if (fread(&left, 1, 2, fd) != 2) {
            ret = E_READSAVE;
        } else if (fread(&top, 1, 2, fd) != 2) {
            ret = E_READSAVE;
        }
        fclose(fd);
    }

    remove(sfname);
    free(sfname);
    return ret;
}


