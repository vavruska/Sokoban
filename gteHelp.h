#ifndef __GTE_HELP_H__
#define __GTE_HELP_H__

/* toolbox fail handler */
#define TOOLFAIL(string) if (toolerror()) SysFailMgr(toolerror(), "\p" string "\n\r    Error Code -> $");

void LoadGTEToolSet(Word userId);
void UnloadGTEToolSet(void);
void startupGTE(word userId);
void shutdownGTE(void);
#endif
