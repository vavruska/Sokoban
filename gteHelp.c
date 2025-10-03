#include <types.h>
#include <loader.h>
#include <locator.h>
#include <misctool.h>
#include <memory.h>
#include <orca.h>

#include "globals.h"
#include "gte.h"

static Handle dpHandle;


#ifdef GTE_IS_SYSTEM_TOOLS_INSTALL
#define ENGINE_STARTUP_MODE 0x0000
#else
#define ENGINE_STARTUP_MODE ENGINE_MODE_USER_TOOL
#endif

/* toolbox fail handler */
#define TOOLFAIL(string) if (toolerror()) SysFailMgr(toolerror(), "\p" string "\n\r    Error Code -> $");

/* path to the local GTE toolset */
Str32 toolPath = {9, "1/Tool160" };

/* Helper function to load GTE as a user tool or system tool */
#ifdef GTE_IS_SYSTEM_TOOLS_INSTALL
void LoadGTEToolSet(Word unused) {
  LoadOneTool(160, 0);
  TOOLFAIL("Unable to load GTE toolset");
}
#else
void LoadGTEToolSet(Word userId) {
  InitialLoadOutputRec loadRec;

  // Load the tool from the local directory
  loadRec = InitialLoad(userId, (Pointer) (&toolPath), 1);
  TOOLFAIL("Unable to load Tool160 from local path");

  // Install the tool using the user tool vector
  SetTSPtr(0x8000, 160, loadRec.startAddr);
  TOOLFAIL("Could not install tool");
}
#endif // GTE_IS_SYSTEM_TOOLS_INSTALL

#ifdef GTE_IS_SYSTEM_TOOLS_INSTALL
void UnloadGTEToolSet(void) {
  UnloadOneTool(160);
  TOOLFAIL("Unable to unload GTE toolset");
}
#else
void UnloadGTEToolSet(void) {
}
#endif // GTE_IS_SYSTEM_TOOLS_INSTALL

void startupGTE(word userId) {
    Word dpAddr;
    LoadGTEToolSet(userId);

    dpHandle = NewHandle(0x200L, userId, attrBank + attrPage + attrFixed + attrLocked + attrNoCross, 0);
    TOOLFAIL("Could not allocate direct page memory for GTE");
    dpAddr = (Word) (*dpHandle);

    GTEStartUp(dpAddr, (Word) ENGINE_STARTUP_MODE, userId);
}

void shutdownGTE(void) {

    GTEShutDown();
    UnloadGTEToolSet();

    DisposeHandle(dpHandle);
}
