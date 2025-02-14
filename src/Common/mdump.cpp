/*
* $Id: mdump.cpp 5561 2009-12-25 07:23:59Z wangmeng $
*
* this file is part of eMule
* Copyright (C)2002-2006 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include "mdump.h"
#include <dbghelp.h>
#include <ShellAPI.h>
#include <tchar.h>
#include <stdio.h>

#define ARRSIZE(x)	(sizeof(x)/sizeof(x[0]))


typedef BOOL(WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
	CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
	CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
	CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

CMiniDumper theCrashDumper;

#ifdef _MSC_VER
TCHAR CMiniDumper::m_szAppName[MAX_PATH] = { 0 };
TCHAR CMiniDumper::m_szDumpPath[MAX_PATH] = { 0 };
#else
std::string CMiniDumper::m_szAppName;
std::string CMiniDumper::m_szDumpPath;

// 信号处理函数，用于捕获程序崩溃时的各类信号
// sig: 触发的信号类型
// info: 信号的详细信息
// context: 信号发生时的上下文信息
void CMiniDumper::SignalHandler(int sig, siginfo_t* info, void* context) {
    char szDumpPath[1024] = { 0 };
    // 如果未指定dump路径，则使用可执行文件所在目录
    if(m_szDumpPath.empty()) {
        char exePath[1024] = { 0 };
        Dl_info dlInfo;
        dladdr((void*)SignalHandler, &dlInfo);
        strncpy(exePath, dlInfo.dli_fname, sizeof(exePath)-1);
        char* lastSlash = strrchr(exePath, '/');
        if(lastSlash) {
            *(lastSlash+1) = '\0';
            strncpy(szDumpPath, exePath, sizeof(szDumpPath)-1);
        }
    } else {
        strncpy(szDumpPath, m_szDumpPath.c_str(), sizeof(szDumpPath)-1);
    }

    // 生成包含时间戳的dump文件名
    time_t now = time(NULL);
    struct tm* timeinfo = localtime(&now);
    char timeStr[64];
    strftime(timeStr, sizeof(timeStr), "%Y%m%d%H%M%S", timeinfo);

    std::string dumpFile = std::string(szDumpPath) + m_szAppName + "_" + timeStr + ".dump";
    GenerateStackTrace(dumpFile.c_str());

    // 生成完堆栈信息后退出程序
    exit(1);
}

// 生成程序崩溃时的堆栈跟踪信息
// dumpPath: dump文件的保存路径
void CMiniDumper::GenerateStackTrace(const char* dumpPath) {
    // 获取堆栈信息
    void* array[50];
    int size = backtrace(array, 50);
    char** messages = backtrace_symbols(array, size);

    FILE* fp = fopen(dumpPath, "w");
    if(!fp) return;

    // 写入基本信息
    fprintf(fp, "Crash Report for %s\n", m_szAppName.c_str());
    fprintf(fp, "Stack trace:\n");

    // 遍历并写入每一层堆栈信息
    for(int i = 0; i < size; i++) {
        Dl_info info;
        if(dladdr(array[i], &info)) {
            int status;
            // 尝试对符号名进行解析
            char* demangled = abi::__cxa_demangle(info.dli_sname, NULL, 0, &status);
            if(status == 0) {
                fprintf(fp, "#%d: %s\n", i, demangled);
                free(demangled);
            } else {
                fprintf(fp, "#%d: %s\n", i, messages[i]);
            }
        } else {
            fprintf(fp, "#%d: %s\n", i, messages[i]);
        }
    }

    fclose(fp);
    free(messages);
}
#endif

#ifdef _MSC_VER
void CMiniDumper::Enable(LPCTSTR pszAppName, bool bShowErrors, LPCTSTR pszDumpPath/* = ""*/)
{
    // 初始化应用名称和dump文件路径
    _tcsncpy(m_szAppName, pszAppName, ARRSIZE(m_szAppName));
    _tcsncpy(m_szDumpPath, pszDumpPath, ARRSIZE(m_szDumpPath));

    MINIDUMPWRITEDUMP pfnMiniDumpWriteDump = NULL;
    HMODULE hDbgHelpDll = GetDebugHelperDll((FARPROC*)&pfnMiniDumpWriteDump, bShowErrors);
    if (hDbgHelpDll)
    {
        if (pfnMiniDumpWriteDump)
            SetUnhandledExceptionFilter(TopLevelFilter);
        FreeLibrary(hDbgHelpDll);
        hDbgHelpDll = NULL;
        pfnMiniDumpWriteDump = NULL;
    }
}
#else
// 启用崩溃转储功能
// pszAppName: 应用程序名称
// bShowErrors: 是否显示错误信息
// pszDumpPath: dump文件保存路径
void CMiniDumper::Enable(const char* pszAppName, bool bShowErrors, const char* pszDumpPath/* = ""*/)
{
    m_szAppName = pszAppName;
    m_szDumpPath = pszDumpPath;

    // 设置信号处理器
    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_sigaction = SignalHandler;
    sa.sa_flags = SA_SIGINFO;

    // 注册需要捕获的信号
    sigaction(SIGSEGV, &sa, NULL);  // 段错误
    sigaction(SIGABRT, &sa, NULL);  // 异常终止
    sigaction(SIGFPE, &sa, NULL);   // 浮点异常
    sigaction(SIGILL, &sa, NULL);   // 非法指令
    sigaction(SIGBUS, &sa, NULL);   // 总线错误
}
#endif

HMODULE CMiniDumper::GetDebugHelperDll(FARPROC* ppfnMiniDumpWriteDump, bool bShowErrors)
{
	*ppfnMiniDumpWriteDump = NULL;
	HMODULE hDll = LoadLibrary(_T("DBGHELP.DLL"));
	if (hDll == NULL)
	{
		if (bShowErrors) {
			// Do *NOT* localize that string (in fact, do not use MFC to load it)!
			MessageBox(NULL, _T("DBGHELP.DLL not found. Please install a DBGHELP.DLL."), m_szAppName, MB_ICONSTOP | MB_OK);
		}
	}
	else
	{
		*ppfnMiniDumpWriteDump = GetProcAddress(hDll, "MiniDumpWriteDump");
		if (*ppfnMiniDumpWriteDump == NULL)
		{
			if (bShowErrors) {
				// Do *NOT* localize that string (in fact, do not use MFC to load it)!
				MessageBox(NULL, _T("DBGHELP.DLL found is too old. Please upgrade to a newer version of DBGHELP.DLL."), m_szAppName, MB_ICONSTOP | MB_OK);
			}
		}
	}
	return hDll;
}

LONG CMiniDumper::TopLevelFilter(struct _EXCEPTION_POINTERS* pExceptionInfo)
{
	LONG lRetValue = EXCEPTION_CONTINUE_SEARCH;
	TCHAR szResult[_MAX_PATH + 1024] = { 0 };
	MINIDUMPWRITEDUMP pfnMiniDumpWriteDump = NULL;
	HMODULE hDll = GetDebugHelperDll((FARPROC*)&pfnMiniDumpWriteDump, true);
	HINSTANCE	hInstCrashReporter = NULL;	//ADDED by fengwen on 2006/11/15 : 使用新的发送错误报告机制。

	if (hDll)
	{
		if (pfnMiniDumpWriteDump)
		{
			//MessageBox(NULL,"test","test",MB_OK);
			// Ask user if they want to save a dump file
			// Do *NOT* localize that string (in fact, do not use MFC to load it)!
			//COMMENTED by fengwen on 2006/11/15	<begin> : 使用新的发送错误报告机制。
			//if (MessageBox(NULL, _T("eMule crashed :-(\r\n\r\nA diagnostic file can be created which will help the author to resolve this problem. This file will be saved on your Disk (and not sent).\r\n\r\nDo you want to create this file now?"), m_szAppName, MB_ICONSTOP | MB_YESNO) == IDYES)
			//COMMENTED by fengwen on 2006/11/15	<end> : 使用新的发送错误报告机制。
			{
				// Create full path for DUMP file
				TCHAR szDumpPath[_MAX_PATH] = { 0 };
				if(_tcsclen(m_szDumpPath) == 0)
				{
					GetModuleFileName(NULL, szDumpPath, ARRSIZE(szDumpPath));
					LPTSTR pszFileName = _tcsrchr(szDumpPath, _T('\\'));
					if (pszFileName) {
						pszFileName++;
						*pszFileName = _T('\0');
					}
				}
				else
				{
					_tcsncpy(szDumpPath, m_szDumpPath, _tcsclen(m_szDumpPath));
					szDumpPath[_tcsclen(m_szDumpPath)] = _T('\0');
				}

				// Replace spaces and dots in file name.
				TCHAR szBaseName[_MAX_PATH] = { 0 };
				_tcsncat(szBaseName, m_szAppName, ARRSIZE(szBaseName) - 1);
				LPTSTR psz = szBaseName;
				while (*psz != _T('\0')) {
					if (*psz == _T('.'))
						*psz = _T('-');
					else if (*psz == _T(' '))
						*psz = _T('_');
					psz++;
				}
				_tcsncat(szDumpPath, szBaseName, ARRSIZE(szDumpPath) - 1);
				SYSTEMTIME curTime;
				GetLocalTime(&curTime);
				char buf[64];
				sprintf(buf, "%4.4d%2.2d%2.2d%2.2d%2.2d%2.2d", curTime.wYear, curTime.wMonth, curTime.wDay, curTime.wHour, curTime.wMinute, curTime.wSecond);
				strcat(szDumpPath, buf);

				_tcsncat(szDumpPath, _T(".dmp"), ARRSIZE(szDumpPath) - 1);

				HANDLE hFile = CreateFile(szDumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if (hFile != INVALID_HANDLE_VALUE)
				{
					_MINIDUMP_EXCEPTION_INFORMATION ExInfo = { 0 };
					ExInfo.ThreadId = GetCurrentThreadId();
					ExInfo.ExceptionPointers = pExceptionInfo;
					ExInfo.ClientPointers = NULL;

					BOOL bOK = (*pfnMiniDumpWriteDump)(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL);
					if (bOK)
					{
						// Do *NOT* localize that string (in fact, do not use MFC to load it)!
						_sntprintf(szResult, ARRSIZE(szResult), _T("Saved dump file to \"%s\".\r\n\r\nPlease send this file together with a detailed bug report to bastet.wang@gmail.com !\r\n\r\nThank you for helping to improve Tsts."), szDumpPath);
						lRetValue = EXCEPTION_EXECUTE_HANDLER;

						//ADDED by fengwen on 2006/11/15	<begin> : 使用新的发送错误报告机制。
						hInstCrashReporter = ShellExecute(NULL, _T("open"), _T("CrashReporter.exe"), szDumpPath, NULL, SW_SHOW);
						if (hInstCrashReporter <= (HINSTANCE)32)
							lRetValue = EXCEPTION_CONTINUE_SEARCH;
						//ADDED by fengwen on 2006/11/15	<end> : 使用新的发送错误报告机制。
					}
					else
					{
						// Do *NOT* localize that string (in fact, do not use MFC to load it)!
						_sntprintf(szResult, ARRSIZE(szResult), _T("Failed to save dump file to \"%s\".\r\n\r\nError: %u"), szDumpPath, GetLastError());
					}
					CloseHandle(hFile);
				}
				else
				{
					// Do *NOT* localize that string (in fact, do not use MFC to load it)!
					_sntprintf(szResult, ARRSIZE(szResult), _T("Failed to create dump file \"%s\".\r\n\r\nError: %u"), szDumpPath, GetLastError());
				}
			}
		}
		FreeLibrary(hDll);
		hDll = NULL;
		pfnMiniDumpWriteDump = NULL;
	}

	//COMMENTED by fengwen on 2006/11/15	<begin> : 使用新的发送错误报告机制。
	//if (szResult[0] != _T('\0'))
	//	MessageBox(NULL, szResult, m_szAppName, MB_ICONINFORMATION | MB_OK);
	//COMMENTED by fengwen on 2006/11/15	<end> : 使用新的发送错误报告机制。

#ifndef _DEBUG
	if (EXCEPTION_EXECUTE_HANDLER == lRetValue)		//ADDED by fengwen on 2006/11/15 : 由此filter处理了异常,才去中止进程。
	{
		// Exit the process only in release builds, so that in debug builds the exceptio is passed to a possible
		// installed debugger
		ExitProcess(0);
	}
	else
		return lRetValue;

#else

	return lRetValue;
#endif
}
