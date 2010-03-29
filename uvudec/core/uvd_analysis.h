/*
UVNet Universal Decompiler (uvudec)
Copyright 2008 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the BSD license.  See LICENSE for details.
*/

#ifndef UVD_ANALYSIS
#define UVD_ANALYSIS

/*
Some data may appear after logical execution of function leading to the next.
Mostly commonly done to preserve alignment.
Code will be marked as unused if it occurs before next function entry and after one of:
-Return (most common)
-Jump

*/
extern int g_filterPostRet;

/*
Look for jumps to other functions
Uusually indicates there is likely something wrong with the analysis
*/
extern int g_analyzeOtherFunctionJump;

/*
For a specific object under analysis
Stores analysis data specific to a job in progress
Original reason for this was so FLIRT engine can run with core UVD stuff without needing assembly engine instances
*/
class UVDAnalysis
{
};

#endif
