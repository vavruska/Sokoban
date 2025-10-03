#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>

#include <gsos.h>
#include <orca.h>

#include "externs.h"
#include "globals.h"
#include "scores.h"

#define SCORE_VERSION "xs02"
#define MAXPATHLEN 255

int scoreentries;
struct st_entry scoretable[MAXSCOREENTRIES];

/* Forward decls */
static int MakeScore(void);
/* Adds a new user score to the score table, if appropriate. Users' top
 * level scores, and the best scores for a particular level (in moves and
 * pushes, separately considered), are always preserved.
 */

static void FlushDeletedScores(bool delete[]);
/* Deletes entries from the score table for which the bool array
   contains true.
*/

static void CopyEntry(int i1, int i2);
/* Duplicate a score entry: overwrite entry i1 with contents of i2. */

static void DeleteLowRanks(void);
static void CleanupScoreTable(void);
/* Removes all score entries for a user who has multiple entries,
 * that are for a level below the user's top level, and that are not "best
 * solutions" as defined by "SolnRank". Also removes duplicate entries
 * for a level that is equal to the user's top level, but which are not
 * the user's best solution as defined by table position.
 *
 * The current implementation is O(n^2) in the number of actual score entries.
 * A hash table would fix this.
 */

static int FindPos(void);
/* Find the position for a new score in the score table */

static int ParseScoreText(char *text, bool all_users);
static int ParseScoreLine(int i, char **text /* in out */, bool all_users);


static void ShowScore(int level);
/* displays the score table to the user. If level == 0, show all
   levels. */

static void ShowScoreLine(int i);
/* Print out line "i" of the score file. */

static int ReadScore(void);
/* Read in an existing score file.  Uses the ntoh() and hton() functions
   so that the score files transfer across systems. Update "scoretable",
   "scoreentries", and "date_stamp" appropriately.
*/

static int FindUser(void);
/* Search the score table to find a specific player, returning the
   index of the highest level that user has played, or -1 if none. */

static int WriteScore(void);
/* Update the score file to contain a new score. See comments below. */

static FILE *scorefile;
static int sfdbn;


int OutputScore(int lev) {
    int ret;


    if ((ret = ReadScore()) == 0) ShowScore(lev);

    return ((ret == 0) ? E_ENDGAME : ret);
}

void DumpLinesWithHeader(int top, int bottom) {
    int i;
    printf("Entries: %d\n", scoreentries);
    printf("Line1: %d\n", scoreentries - 1 - bottom);
    printf("Line2: %d\n", scoreentries - top);
    printf("========================================"
           "==============================\n");
    for (i = bottom; i >= top; i--) ShowScoreLine(i);
}

int OutputScoreLines(int line1, int line2) {
    int ret;
    ret = ReadScore();

    if (ret == 0) {
        int i;
        int top, bottom;
        DeleteLowRanks();
        if (line1 > scoreentries) line1 = scoreentries;
        if (line2 > scoreentries) line2 = scoreentries;
        bottom = scoreentries - 1 - line1;
        top = scoreentries - line2;

        for (i = top - 1; i >= 0; i--) {
            if (scoretable[i].lv != scoretable[top].lv) break;
        }
        top = i + 1;
        for (i = bottom + 1; i < scoreentries; i++) {
            if (scoretable[i].lv != scoretable[bottom].lv) break;
        }
        bottom = i - 1;

        DumpLinesWithHeader(top, bottom);
    }
    return ((ret == 0) ? E_ENDGAME : ret);
}

int MakeNewScore(char *textfile) {
    int ret = 0;
    GSString255 path;
    FileInfoRecGS info = { 9, &path, 0 };

    if (textfile) {
        char *text, *pos, *end;
        int fd;

        strcpy(path.text, textfile);
        path.length = strlen(textfile);
        GetFileInfoGS(&info);
        if (toolerror()) {
            perror(textfile);
            return E_FOPENSCORE;
        }
        if (0 > (fd = open(textfile, O_RDONLY))) {
            perror(textfile);
            return E_FOPENSCORE;
        }
        pos = text = (char *) malloc((size_t) info.eof);
        end = text + info.eof;
        while (pos < end) {
            int n = read(fd, pos, end - pos);
            switch (n) {
            case -1:
                perror(textfile);
                return E_FOPENSCORE;
            case 0:
                fprintf(stderr, "Unexpected EOF\n");
                return E_FOPENSCORE;
            default:
                pos += n;
                break;

            }
        }
        (void) close(fd);
        if ((ret = ParseScoreText(text, true))) {
            return ret;
        }
        free(text);
    } else {
        scoreentries = 0;
    }

    if ((ret = WriteScore())) return ret;
    return ((ret == 0) ? E_ENDGAME : ret);
}

int GetUserLevel(int *lv) {
    int ret = 0;

    if ((scorefile = fopen(SCOREFILE, "r")) == NULL) {
        ret = E_FOPENSCORE;
    } else {
        int pos;
        if ((ret = ReadScore()) == 0) *lv = ((pos = FindUser()) > -1) ? scoretable[pos].lv + 1 : 1;
    }
    return (ret);
}


int Score(void) {
    int ret;
    bool made = false;

    do {
        if ((ret = ReadScore()) == 0) {
            if ((ret = MakeScore()) == 0) {
                ret = WriteScore();
            }
            break;
        } else {
            if (made) {
                ret = E_FOPENSCORE;
                break;
            }
            MakeNewScore(NULL);
            made = true;
        }
    } while (1);
    return ((ret == 0) ? E_ENDGAME : ret);
}

static int ReadOldScoreFile01(void);
static int ReadScore(void) {
    int ret = 0;
    long tmp;

    sfdbn = open(SCOREFILE, O_RDONLY);
    if (0 > sfdbn) ret = E_FOPENSCORE;
    else {
        char magic[5];
        if (read(sfdbn, &magic[0], 4) != 4) ret = E_READSCORE;
        magic[4] = 0;
        if (0 == strcmp(magic, SCORE_VERSION)) {
            /* we have the right version */
        } else {
            if (0 == strcmp(magic, "xs01")) {
                fprintf(stderr, "Warning: old-style score file\n");
                return ReadOldScoreFile01();
            } else {
                fprintf(stderr, "Error: unrecognized score file format. May be"
                        "  obsolete, or else maybe this program is.\n");
                ret = E_READSCORE;
                (void) close(ret);
                return ret;
            }
        }
        if (read(sfdbn, &scoreentries, 2) != 2) ret = E_READSCORE;
        else {
            tmp = scoreentries * sizeof(scoretable[0]);
            if (read(sfdbn, &(scoretable[0]), tmp) != tmp) ret = E_READSCORE;

        }
        (void) close(sfdbn);
    }
    return ret;
}

int ReadOldScoreFile01(void) {
    int ret = 0;
    time_t now = time(0);
    if (read(sfdbn, &scoreentries, 2) != 2) ret = E_READSCORE;
    else {
        int tmp;
        struct old_st_entry *t = (struct old_st_entry *) malloc(scoreentries *
                                                                sizeof(struct old_st_entry));
        tmp = scoreentries * sizeof(t[0]);
        if (read(sfdbn, &t[0], tmp) != tmp) ret = E_READSCORE;

        /* swap up for little-endian machines */
        for (tmp = 0; tmp < scoreentries; tmp++) {
            scoretable[tmp].lv = t[tmp].lv;
            scoretable[tmp].mv = t[tmp].mv;
            scoretable[tmp].ps = t[tmp].ps;
            strncpy(scoretable[tmp].user, t[tmp].user, MAXUSERNAME);
            scoretable[tmp].date = now;
        }
    }
    (void) close(sfdbn);
    return ret;
}

int SolnRank(int j, bool *ignore) {
    int i, rank = 1;
    unsigned int level = scoretable[j].lv;
    for (i = 0; i < j; i++) {
        if (VALID_ENTRY(i) &&
            !(ignore && ignore[i]) && scoretable[i].lv == level) {
            if (scoretable[i].mv == scoretable[j].mv &&
                scoretable[i].ps == scoretable[j].ps &&
                0 == strcmp(scoretable[i].user, scoretable[j].user)) {
                rank = BADSOLN;
            }
            if ((scoretable[i].mv < scoretable[j].mv &&
                 scoretable[i].ps <= scoretable[j].ps) ||
                (scoretable[i].mv <= scoretable[j].mv &&
                 scoretable[i].ps < scoretable[j].ps)) {
                if (0 == strcmp(scoretable[i].user, scoretable[j].user)) rank = BADSOLN;
                else rank++;
            }
        }
    }
    return rank;
}

static void DeleteLowRanks(void) {
    int i;
    bool deletable[MAXSCOREENTRIES];
    for (i = 0; i < scoreentries; i++) {
        deletable[i] = false;
        if (SolnRank(i, deletable) > MAXSOLNRANK &&
            0 != strcmp(scoretable[i].user, localuser)) {
            deletable[i] = true;
        }
    }
    FlushDeletedScores(deletable);
}

static void CleanupScoreTable(void) {
    int i;
    bool deletable[MAXSCOREENTRIES];
    for (i = 0; i < scoreentries; i++) {
        deletable[i] = false;
        if (SolnRank(i, deletable) > MAXSOLNRANK) {
            char *user = scoretable[i].user;
            int j;
            for (j = 0; j < i; j++) {
                if (0 == strcmp(scoretable[j].user, user)) deletable[i] = true;
            }
        }
    }
    FlushDeletedScores(deletable);
}

static void FlushDeletedScores(bool delete[]){
    int i, k = 0;
    for (i = 0; i < scoreentries; i++) {
        if (i != k) CopyEntry(k, i);
        if (!delete[i]) k++;
    }
    scoreentries = k;
}

static int MakeScore(void) {
    int pos, i;

    pos = FindPos();      /* find the new score position */
    if (pos > -1){       /* score table not empty */
        for (i = scoreentries; i > pos; i--)
        CopyEntry(i, i - 1);
    } else {
        pos = scoreentries;
    }

    strcpy(scoretable[pos].user, localuser);
    scoretable[pos].lv = scorelevel;
    scoretable[pos].mv = scoremoves;
    scoretable[pos].ps = scorepushes;
    scoretable[pos].date = time(0);
    scoreentries++;

    CleanupScoreTable();
    if (scoreentries == MAXSCOREENTRIES)
    return E_TOMUCHSE;
    else
    return 0;
}

static int FindUser(void) {
    int i;
    bool found = false;
    extern char *localuser;

    for (i = 0; (i < scoreentries) && (!found); i++) {
        found = (strcmp(scoretable[i].user, localuser) == 0);
    }
    return ((found) ? i - 1 : -1);
}

static int FindPos(void) {
    int i;
    bool found = false;

    for (i = 0; (i < scoreentries) && (!found); i++) {
        found = ((scorelevel > scoretable[i].lv) ||
                 ((scorelevel == scoretable[i].lv) &&
                  (scoremoves < scoretable[i].mv)) ||
                 ((scorelevel == scoretable[i].lv) &&
                  (scoremoves == scoretable[i].mv) &&
                  (scorepushes < scoretable[i].ps)));
    }
    return ((found) ? i - 1 : -1);
}

/*  WriteScore() writes out the score table.  It uses ntoh() and hton()
    functions to make the scorefile compatible across systems. It and
    LockScore() try to avoid trashing the score file, even across NFS.
    However, they are not perfect.

     The vulnerability here is that if we take more than 10 seconds to
     finish Score(), AND someone else decides to break the lock, AND
     they pick the same temporary name, they may write on top of the
     same file. Then we could scramble the score file by moving it with
     alacrity to SCOREFILE before they finish their update. This is
     quite unlikely, but possible.

     We could limit the damage by writing just the one score we're
     adding to a temporary file *when we can't acquire the lock*. Then,
     the next time someone comes by and gets the lock, they integrate
     all the temporary files. Since the score change would be smaller
     than one block, duplicate temporary file names means only that a
     score change can be lost. This approach would not require a TIMEOUT.

     The problem with that scheme is that if someone dies holding the
     lock, the temporary files just pile up without getting applied.
     Also, user intervention is required to blow away the lock; and
     blowing away the lock can get us in the same trouble that happens
     here.
*/

char const *tempnm = SCOREFILE "000";

static int WriteScore(void) {
    int ret = 0;
    int tmp;
    char tempfile[MAXPATHLEN];

    strcpy(tempfile, tempnm);

    scorefile = fopen(tempfile, "w");
    if (!scorefile) {
        return E_FOPENSCORE;
    }

    if (fwrite(SCORE_VERSION, 4, 1, scorefile) != 1) {
        ret = E_WRITESCORE;
    } else if (fwrite(&scoreentries, 2, 1, scorefile) != 1) {
        ret = E_WRITESCORE;
    } else {

        tmp = scoreentries;
        while (tmp > 0) {
            int n = fwrite(&(scoretable[scoreentries - tmp]),
                           sizeof(struct st_entry),
                           tmp,
                           scorefile);
            if (n <= 0) {
                perror(tempfile);
                ret = E_WRITESCORE;
                break;
            }
            tmp -= n;
        }
    }
    if (EOF == fflush(scorefile)) {
        ret = E_WRITESCORE;
        perror(tempfile);
    }
    if (EOF == fclose(scorefile)) {
        ret = E_WRITESCORE;
        perror(tempfile);
    }
    if (ret == 0) {
        ret = remove(SCOREFILE);
        ret = 0;
    }
    if (ret == 0) {
        ret = rename(tempfile, SCOREFILE);
    }
    if (ret != 0) {
        ret = remove(tempfile);
    }
    return ret;
}

char *mos[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
#define MONTH(x) mos[x]

char date_buf[10];

char *DateToASCII(time_t date) {
    struct tm then, now;
    time_t dnow = time(0);
    now = *localtime(&dnow);
    then = *localtime(&date);
    if (then.tm_year != now.tm_year) {
        sprintf(date_buf, "%s %d", MONTH(then.tm_mon), then.tm_year % 100);
    } else if (then.tm_mon != now.tm_mon ||
               then.tm_mday != now.tm_mday) {
        sprintf(date_buf, "%d %s", then.tm_mday, MONTH(then.tm_mon));
    } else {
        int hour = then.tm_hour % 12;
        if (hour == 0) hour = 12;
        sprintf(date_buf, "Today, %d:%.2d%s", hour, then.tm_min,
                (then.tm_hour < 12) ? "am" : "pm");
    }
    return date_buf;
}


#define TRY(name,expr) do { if (0>(expr)) { perror(name); }} while (0)

static void ShowScoreLine(int i) {
    int rank = SolnRank(i, 0);
    if (rank <= MAXSOLNRANK) TRY("printf", printf("%4d", rank));
    else TRY("printf", printf("    "));
    TRY("printf",
        fprintf(stdout, " %32s %4d     %4d     %4d   %s\n", scoretable[i].user,
                scoretable[i].lv, scoretable[i].mv, scoretable[i].ps,
                DateToASCII((time_t) scoretable[i].date)));
}


static void ShowScore(int level) {
    int i;
    int top = -1, bottom = -1;
    DeleteLowRanks();
    if (level == 0) {
        top = 0;
        bottom = scoreentries - 1;
    } else {
        for (i = 0; i < scoreentries; i++) {
            if (scoretable[i].lv == level) {
                if (top == -1) top = i;
                bottom = i;
            }
        }
    }
    DumpLinesWithHeader(top, bottom);
}

static void CopyEntry(int i1, int i2) {
    strcpy(scoretable[i1].user, scoretable[i2].user);
    scoretable[i1].lv = scoretable[i2].lv;
    scoretable[i1].mv = scoretable[i2].mv;
    scoretable[i1].ps = scoretable[i2].ps;
    scoretable[i1].date = scoretable[i2].date;
}

/* Extract one line from "text".  Return 0 if there is no line to extract. */
char *getline(char *text, char *linebuf, int bufsiz) {
    if (*text == 0) {
        *linebuf = 0;
        return 0;
    }
    bufsiz--; /* for trailing null */
    while (*text != '\n' && *text != '\r' && *text && bufsiz != 0) {
        *linebuf++ = *text++;
        bufsiz--;
    }
    if (text[0] == '\r' && text[1] == '\n') text++; /* skip over CRLF */
    *linebuf = 0;
    return (*text) ? text + 1 : text;
    /* point to next line or final null */
}

static int ParseScoreText(char *text, bool allusers) {
    char line[256];
    do {
        text = getline(text, line, sizeof(line));
        if (!text) return E_READSCORE;
    } while (line[0] != '=');
    scoreentries = 0;
    while (text) {
        ParseScoreLine(scoreentries, &text, allusers);
        if (VALID_ENTRY(scoreentries)) scoreentries++;
    }
    return 0;
}

static int ParseScoreLine(int i, char **text /* in out */, bool all_users) {
    char *user, *date_str;
    char *ws = " \t\r\n";
    int level, moves, pushes;
    int date = 0; /* time_t */
    bool baddate = false;
    int rank;
    char rank_s[4];
    char line[256];
    *text = getline(*text, line, sizeof(line));
    if (!*text) {
        return 0;
    }
    strncpy(rank_s, line, 4);
    rank = atoi(rank_s);
    user = strtok(line + 4, ws);
    if (!user) {
        *text = 0;
        return 0;
    }
    if (all_users || rank != 0 || 0 == strcmp(user, localuser)) {
        level = atoi(strtok(0, ws));
        if (!level) {
            return E_READSCORE;
        }
        moves = atoi(strtok(0, ws));
        if (!moves) {
            return E_READSCORE;
        }
        pushes = atoi(strtok(0, ws));
        if (!pushes) {
            return E_READSCORE;
        }
        date_str = strtok(0, ws);
        if (date_str) {
            date = (time_t) atoi(date_str);
        }
        if (!date) {
            date = time(0);
            if (!baddate) {
                baddate = true;
                fprintf(stderr,
                        "Warning: Bad or missing date in ASCII scores\n");
            }
        }
        strncpy(scoretable[i].user, user, MAXUSERNAME);
        scoretable[i].lv = (unsigned int) level;
        scoretable[i].mv = (unsigned int) moves;
        scoretable[i].ps = (unsigned int) pushes;
        scoretable[i].date = date;
    } else {
        scoretable[i].user[0] = 0;
    }
    return 0;
}

int FindCurrent(void)
/*
    Return the scoretable index pointing to level "level", or as
    close as possible. If the scoretable is empty, return -1.
*/
{
    int i;
    for (i = 0; i < scoreentries; i++) {
        if (0 == strcmp(scoretable[i].user, localuser) &&
            (unsigned int) level == scoretable[i].lv) {
            return i;
        }
    }
/* Find largest of smaller-numbered levels */
    for (i = 0; i < scoreentries; i++) {
        if (scoretable[i].user[0] &&
            (unsigned int) level >= scoretable[i].lv) {
            return i;
        }
    }
/* Find smallest of larger-numbered levels */
    for (i = scoreentries - 1; i >= 0; i--) {
        if (scoretable[i].user[0] &&
            (unsigned int) level < scoretable[i].lv) {
            return i;
        }
    }
    return -1; /* Couldn't find it at all */
}


