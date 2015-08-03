#include <Windows.h>
#include "Debug.h"
#include "DealEvent.h"
#include "..\Interface.h"

_DEBUG_EVENT	DbgEvt	= {0};		
DWORD	dwRet			= DBG_EXCEPTION_NOT_HANDLED;//未处理!!!!!!!!!!!dwRet有错误使用的代码似乎
STARTUPINFO			stcStartupInfo	= {sizeof(STARTUPINFO)};
PROCESS_INFORMATION stcProcInfo		= {0};//进程信息
extern HANDLE g_hProc;//被调试者的句柄

int StartDebug(bool bNewProcess,void *Infor);
bool GetEvent();
//标记是否正在调试状态
bool Debuging=false;



//////////////////////////////////////////////////////////////////////////
//bool bNewProcess	:以调试的方式启动一个进程(1),附加到一个进程(0)
//void *Infor		:指向一个wchar_t字符串或者DWORD值,这取决于bNewProcess.
int StartDebug(bool bNewProcess,void *Infor)
{
	//判断当前调试状态
	if (true==Debuging)
	{
		return -1;
	}

	if (bNewProcess)
	{
		if(!CreateProcess((LPCWSTR)Infor,
			NULL,				//命令行
			NULL,				//安全描述符
			NULL,				//线程属性是否可继承
			FALSE,				//是否从调用进程出继承了句柄
			DEBUG_PROCESS,		//如果以DEBUG_ONLY_THIS_PROCESS,只会调试这一个进程而不是它的子进程一起
			NULL,				//新进程的环境块
			NULL,				//新进程的当前工作路径(当前目录)
			&stcStartupInfo,	//指定新进程的主窗口特性
			&stcProcInfo		//接收新进程的识别信息
			))
		{
			OutputDebug(L"创建调试进程失败");
			return 1;
		}
			
			
			
			;
		//DEBUG_PROCESS
		//DEBUG_ONLY_THIS_PROCESS
	}
	else
	{
		DebugActiveProcess(*((DWORD*)Infor));//传入DWORD值,而不是传入DWORD指针.
	}

	Debuging=true;

	GetEvent();

	return 1;
}

bool GetEvent()
{
	while(1==WaitForDebugEvent(&DbgEvt,INFINITE))
	{
		//检测是否存在需要调用的函数,它们由命令行提供
		AutoAnalysisCommandParsing();

		switch (DbgEvt.dwDebugEventCode)
		{		
			//异常调试
		case EXCEPTION_DEBUG_EVENT:
			{
				dwRet= OnExceptionDebugEvent(&DbgEvt.u.Exception);
				break;
			}	
			//进程创建
		case CREATE_PROCESS_DEBUG_EVENT:
			{
				Deal_CPEV();
				break;
			}
			//进程退出
		case EXIT_PROCESS_DEBUG_EVENT:
			{
				break;
			}
			//线程创建
		case CREATE_THREAD_DEBUG_EVENT:
			{
				break;
			}
			//线程退出
		case EXIT_THREAD_DEBUG_EVENT:
			{
				break;
			}
			//映射DLL
		case LOAD_DLL_DEBUG_EVENT:
			{
				break;
			}
			//卸载DLL
		case UNLOAD_DLL_DEBUG_EVENT:
			{
				break;
			}
			//调试字符串输出
		case OUTPUT_DEBUG_STRING_EVENT:
			{
				break;
			}
			//RIP(内部错误)
		case RIP_EVENT:
			{
				break;
			}

		default:
			{
				break;
			}
		}
		ContinueDebugEvent(
			DbgEvt.dwProcessId,
			DbgEvt.dwThreadId,
			dwRet);
		//dwRet的值:
		//DBG_CONTINUE:				异常已经处理
		//DBG_EXCEPTION_NOT_HANDLED:异常需要系统继续处理(我们的默认)
	}
	return false;
}


bool Detach()
{
	if (0!=DbgEvt.dwProcessId)
	{
		DebugActiveProcessStop(DbgEvt.dwProcessId);
		Debuging=false;
		return true;
	}
	
	return false;
	
}


//bLove==true   调试者崩溃,被调试者不会退出
//bLove==false  相反.默认情况.
//返回:false表示失败.
bool LoveIt(bool bLove)
{
	if (0==DebugSetProcessKillOnExit(!bLove))
	{
		return false;
	}

	return true;
}


