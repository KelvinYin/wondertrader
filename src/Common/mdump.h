/*
* $Id: mdump.h 5561 2009-12-25 07:23:59Z wangmeng $
*
* this file is part of easyMule
* Copyright (C)2002-2008 VeryCD Dev Team ( strEmail.Format("%s@%s", "emuledev", "verycd.com") / http: * www.easymule.org )
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
#pragma once

#ifdef _MSC_VER
#include <Windows.h>
struct _EXCEPTION_POINTERS;
#else
#include <string>
#include <signal.h>
#endif

// CMiniDumper类用于处理程序崩溃时的转储功能
// 在Windows平台使用MiniDumpWriteDump生成dump文件
// 在Unix平台使用信号处理和堆栈跟踪生成dump文件
class CMiniDumper
{
public:
#ifdef _MSC_VER
    // 启用崩溃转储功能（Windows平台）
    // pszAppName: 应用程序名称
    // bShowErrors: 是否显示错误信息
    // pszDumpPath: dump文件保存路径，默认为空
    static void Enable(LPCTSTR pszAppName, bool bShowErrors, LPCTSTR pszDumpPath = "");
#else
    // 启用崩溃转储功能（Unix平台）
    // pszAppName: 应用程序名称
    // bShowErrors: 是否显示错误信息
    // pszDumpPath: dump文件保存路径，默认为空
    static void Enable(const char* pszAppName, bool bShowErrors, const char* pszDumpPath = "");
#endif

private:
#ifdef _MSC_VER
    static TCHAR m_szAppName[MAX_PATH];    // 应用程序名称
    static TCHAR m_szDumpPath[MAX_PATH];   // dump文件保存路径
    // 获取调试帮助DLL
    static HMODULE GetDebugHelperDll(FARPROC* ppfnMiniDumpWriteDump, bool bShowErrors);
    // 顶层异常过滤器
    static LONG WINAPI TopLevelFilter(struct _EXCEPTION_POINTERS* pExceptionInfo);
#else
    static std::string m_szAppName;        // 应用程序名称
    static std::string m_szDumpPath;       // dump文件保存路径
    // 信号处理函数
    static void SignalHandler(int sig, siginfo_t* info, void* context);
    // 生成堆栈跟踪信息
    static void GenerateStackTrace(const char* dumpPath);
#endif
};

extern CMiniDumper theCrashDumper;
