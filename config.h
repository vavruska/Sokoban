/***********************************************************************
   Configuration variables for xsokoban

   You may wish to alter the directory paths, particularly ROOTDIR.

   Note that the string concatenation performed below requires an
   ANSI-standard preprocessor. If you don't have one, you'll have to
   manually edit SCREENPATH, SAVEPATH, etc. to start with ROOTDIR.
***********************************************************************/
segment "AUTOSEG~~~";
#pragma memorymodel 1

/*
   ROOTDIR: The directory below which the sokoban data files, such as
	    the bitmaps, score file, and saved games, are stored.  I
	    suggest "/usr/local/share/xsokoban" as a value for this
	    variable in the installed version, but you know best...
*/
#ifndef ROOTDIR
#define ROOTDIR "9:data"
#endif

/*
   SCREENPATH: the name of the directory where the screen files are held
*/
#ifndef SCREENPATH
#define SCREENPATH ROOTDIR ":screens"
#endif

/*
   SAVEPATH: the name of the path where save files are held
             Attention: Be sure that there are no other files with
                        the name <username>.sav
*/
#ifndef SAVEPATH
#define SAVEPATH ROOTDIR
#endif


/*
   SCOREFILE: the full pathname of the score file
*/
#ifndef SCOREFILE
#define SCOREFILE ROOTDIR ":scores"
#endif

/*
   MAXUSERNAME: defines the maximum length of a system's user name
*/
#define MAXUSERNAME     32

/*
   MAXSCOREENTRIES: defines the maximum number of entries in the scoretable
*/
#define MAXSCOREENTRIES 1000

/*
   ANYLEVEL: Allow any user to play any level and enter a score for it
*/
#define ANYLEVEL 0

/*
   MAXSOLNRANK: The maximum solution rank for which an entry is retained
   in the score table.
*/
#define MAXSOLNRANK 10

/*
   STACKDEPTH: Number of previous positions remembered in the move stack
*/
#define STACKDEPTH 1000

/*
   SLEEPLEN: Amount of time to sleep for between moves of the man,
   in msec.
*/
#define SLEEPLEN 8

/*
   WWW: Use WWW to store the score file and screens for you.

   In "www.h", you will find definitions that will allow xsokoban to connect
   to an public xsokoban server maintained by Andrew Myers. The xsokoban
   home page is at
    
       http://xsokoban.lcs.mit.edu/xsokoban.html
    
   In order to create your own WWW xsokoban score server, a few small
   shell scripts must be used; they are not provided in this
   distribution, but can be obtained on request from andru@lcs.mit.edu.
*/

#ifndef WWW
#define WWW 1
#endif

