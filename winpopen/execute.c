
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

static int os_execute (lua_State *L) {
	const char *cmd = luaL_optstring(L, 1, NULL);
	int stat = RunSilent(cmd);
	if (cmd != NULL) {
		lua_pushinteger(L, stat);
		return 1;
	} else {
		lua_pushboolean(L, stat);  /* true if there is a shell */
		return 1;
	}
}

int LUA_API luaopen_execute (lua_State *L) {
	static const luaL_Reg os[] = {
		{"execute", os_execute},
		{NULL, NULL},
	};
	luaL_register(L, "os", os);
	return 1;
}
