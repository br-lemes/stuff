/* winpopen.c
   Replace io.pclose, io.popen and os.execute by Breno Ramalho Lemes
   http://www.br-lemes.net/
*/

/* popen.c
  RunSilent() is by Steven Szelei,
  and pt_popen()/pt_pclose() is by Kurt Keller
  Modified and comments translated by Steve Donovan

  Please note an extension; if your commmand contains '2>&1'
  then any error output will be redirected as well to the pipe.

  Put this file in scite\lua\src\lib and add to your project

  to modify liolib.c in the same dir,
  replace conditional at line 47 with:

  #ifndef USE_POPEN
  #ifdef WIN32
  #define USE_POPEN 1
  FILE* pt_popen(const char *cmd, const char*mode);
  int pt_pclose(FILE *file);
  unsigned long RunSilent(const char* strCommand);
  #define popen pt_popen
  #define pclose pt_pclose
  #define system RunSilent
  #endif
  #else
  #define USE_POPEN	0
  #endif

*/

#include "lua.h"
#include "lauxlib.h"

#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <io.h>

DWORD RunSilent(const char* strCommand) {
	STARTUPINFO StartupInfo;
	PROCESS_INFORMATION ProcessInfo;
	char Args[4096];
	char *pEnvCMD = NULL;
	char *pDefaultCMD = "CMD.EXE";
	ULONG rc;

	memset(&StartupInfo, 0, sizeof(StartupInfo));
	StartupInfo.cb = sizeof(STARTUPINFO);
	StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
	StartupInfo.wShowWindow = SW_HIDE;

	Args[0] = 0;

	pEnvCMD = getenv("COMSPEC");

	if(pEnvCMD){		
		strcpy(Args, pEnvCMD);
	} else{
		strcpy(Args, pDefaultCMD);
	}

	/* "/c" option - Do the command then terminate the command window */
	strcat(Args, " /c "); 
	/*the application you would like to run from the command window */
	strcat(Args, strCommand);  

	if (!CreateProcess( NULL, Args, NULL, NULL, FALSE,
		CREATE_NEW_CONSOLE, 
		NULL, 
		NULL,
		&StartupInfo,
		&ProcessInfo))
	{
		return GetLastError();		
	}

	WaitForSingleObject(ProcessInfo.hProcess, INFINITE);
	if(!GetExitCodeProcess(ProcessInfo.hProcess, &rc))
		rc = 0;

	CloseHandle(ProcessInfo.hThread);
	CloseHandle(ProcessInfo.hProcess);

	return rc;
}

/*------------------------------------------------------------------------------
  Globals for the Routines pt_popen() / pt_pclose()
------------------------------------------------------------------------------*/
static HANDLE my_pipein[2], my_pipeout[2], my_pipeerr[2];
static char   my_popenmode = ' ';

static int
my_pipe(HANDLE *readwrite)
{
  SECURITY_ATTRIBUTES sa;

  sa.nLength = sizeof(sa);          /* Length in bytes */
  sa.bInheritHandle = 1;            /* the child must inherit these handles */
  sa.lpSecurityDescriptor = NULL;

  if (! CreatePipe (&readwrite[0],&readwrite[1],&sa,1 << 13))
  {
    errno = -1; /* EMFILE; que? */
    return -1;
  }

  return 0;
}

/*------------------------------------------------------------------------------
  Replacement for 'popen()' under WIN32.
  NOTE: if cmd contains '2>&1', we connect the standard error file handle
    to the standard output file handle.
------------------------------------------------------------------------------*/
FILE *
pt_popen(const char *cmd, const char *mode)
{
  FILE *fptr = (FILE *)0;
  PROCESS_INFORMATION piProcInfo;
  STARTUPINFO siStartInfo;
  int success, redirect_error = 0;
  char cmd_buff[2048];
  char *err2out;

  const char *shell_cmd = getenv("COMSPEC");
  if (! shell_cmd) shell_cmd = "cmd";
  strcpy(cmd_buff,shell_cmd);
  strcat(cmd_buff," /c ");
  strcat(cmd_buff,cmd);

  my_pipein[0]   = INVALID_HANDLE_VALUE;
  my_pipein[1]   = INVALID_HANDLE_VALUE;
  my_pipeout[0]  = INVALID_HANDLE_VALUE;
  my_pipeout[1]  = INVALID_HANDLE_VALUE;
  my_pipeerr[0]  = INVALID_HANDLE_VALUE;
  my_pipeerr[1]  = INVALID_HANDLE_VALUE;

  if (!mode || !*mode)
    goto finito;

  my_popenmode = *mode;
  if (my_popenmode != 'r' && my_popenmode != 'w')
    goto finito;

  /*
   * Shall we redirect stderr to stdout ? */
  if ((err2out = strstr("2>&1",cmd)) != NULL) {
     /* this option doesn't apply to win32 shells, so we clear it out! */
     strncpy(err2out,"    ",4);
     redirect_error = 1;
  }

  /*
   * Create the Pipes... */
  if (my_pipe(my_pipein)  == -1 ||
      my_pipe(my_pipeout) == -1)
    goto finito;
  if (!redirect_error && my_pipe(my_pipeerr) == -1)
    goto finito;

  /*
   * Now create the child process */
  ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
  siStartInfo.cb           = sizeof(STARTUPINFO);
  siStartInfo.hStdInput    = my_pipein[0];
  siStartInfo.hStdOutput   = my_pipeout[1];
  if (redirect_error)
    siStartInfo.hStdError  = my_pipeout[1];
  else
    siStartInfo.hStdError  = my_pipeerr[1];
  siStartInfo.dwFlags    = STARTF_USESTDHANDLES;

  success = CreateProcess(NULL,
     (LPTSTR)cmd_buff,  // command line 
     NULL,              // process security attributes 
     NULL,              // primary thread security attributes 
     TRUE,              // handles are inherited 
     DETACHED_PROCESS,  // creation flags: without window (?)
     NULL,              // use parent's environment 
     NULL,              // use parent's current directory 
     &siStartInfo,      // STARTUPINFO pointer 
     &piProcInfo);      // receives PROCESS_INFORMATION 

  if (!success)
    goto finito;

  /*
   * These handles listen to the Child process */
  CloseHandle(my_pipein[0]);  my_pipein[0]  = INVALID_HANDLE_VALUE;
  CloseHandle(my_pipeout[1]); my_pipeout[1] = INVALID_HANDLE_VALUE;
  CloseHandle(my_pipeerr[1]); my_pipeerr[1] = INVALID_HANDLE_VALUE;

  if (my_popenmode == 'r')
    fptr = _fdopen(_open_osfhandle((long)my_pipeout[0],_O_BINARY),"r");
  else
    fptr = _fdopen(_open_osfhandle((long)my_pipein[1],_O_BINARY),"w");

finito:
  if (!fptr)
  {
    if (my_pipein[0]  != INVALID_HANDLE_VALUE)
      CloseHandle(my_pipein[0]);
    if (my_pipein[1]  != INVALID_HANDLE_VALUE)
      CloseHandle(my_pipein[1]);
    if (my_pipeout[0] != INVALID_HANDLE_VALUE)
      CloseHandle(my_pipeout[0]);
    if (my_pipeout[1] != INVALID_HANDLE_VALUE)
      CloseHandle(my_pipeout[1]);
    if (my_pipeerr[0] != INVALID_HANDLE_VALUE)
      CloseHandle(my_pipeerr[0]);
    if (my_pipeerr[1] != INVALID_HANDLE_VALUE)
      CloseHandle(my_pipeerr[1]);
  }
  return fptr;

}

/*------------------------------------------------------------------------------
  Replacement for 'pclose()' under WIN32
------------------------------------------------------------------------------*/
int
pt_pclose(FILE *fle)
{
  if (fle)
  {
    (void)fclose(fle);

    CloseHandle(my_pipeerr[0]);
    if (my_popenmode == 'r')
      CloseHandle(my_pipein[1]);
    else
     CloseHandle(my_pipeout[0]);
    return 0;
  }
  return -1;
}

typedef luaL_Stream LStream;
#define tolstream(L)	((LStream *)luaL_checkudata(L, 1, LUA_FILEHANDLE))

static LStream *newprefile (lua_State *L) {
  LStream *p = (LStream *)lua_newuserdata(L, sizeof(LStream));
  p->closef = NULL;  /* mark file handle as 'closed' */
  luaL_setmetatable(L, LUA_FILEHANDLE);
  return p;
}

static int io_pclose (lua_State *L) {
  LStream *p = tolstream(L);
  return luaL_execresult(L, pt_pclose(p->f));
}

static int io_popen (lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  const char *mode = luaL_optstring(L, 2, "r");
  LStream *p = newprefile(L);
  p->f = pt_popen(filename, mode);
  p->closef = &io_pclose;
  return (p->f == NULL) ? luaL_fileresult(L, 0, filename) : 1;
}

static int os_execute (lua_State *L) {
  const char *cmd = luaL_optstring(L, 1, NULL);
  int stat = RunSilent(cmd);
  if (cmd != NULL)
    return luaL_execresult(L, stat);
  else {
    lua_pushboolean(L, stat);  /* true if there is a shell */
    return 1;
  }
}

int LUA_API luaopen_winpopen (lua_State *L) {
	static const luaL_Reg io[] = {
		{"pclose", io_pclose},
		{"popen", io_popen},
		{NULL, NULL},
	};
	static const luaL_Reg os[] = {
		{"execute", os_execute},
		{NULL, NULL},
	};
	lua_getglobal(L, "io");
	luaL_setfuncs (L, io, 0);
	lua_pop(L, 1);
	lua_getglobal(L, "os");
	luaL_setfuncs (L, os, 0);
	lua_pop(L, 1);
	return 1;
}
