#include <Windows.h>
#include "Debug.h"
#include "AntiAsm.h"

_DEBUG_EVENT	DbgEvt	= {0};		
DWORD	dwRet			= DBG_EXCEPTION_NOT_HANDLED;//未处理!!!!!!!!!!!dwRet有错误使用的代码似乎
STARTUPINFO			stcStartupInfo	= {sizeof(STARTUPINFO)};
PROCESS_INFORMATION stcProcInfo		= {0};//进程信息
HANDLE g_hProc;//被调试者的句柄

int StartDebug(bool bNewProcess,void *Infor);
DWORD OnExceptionDebugEvent(LPEXCEPTION_DEBUG_INFO pDbgInfo);
bool GetEvent();




//////////////////////////////////////////////////////////////////////////
//bool bNewProcess	:以调试的方式启动一个进程(1),附加到一个进程(0)
//void *Infor		:指向一个wchar_t字符串或者DWORD值,这取决于bNewProcess.
int StartDebug(bool bNewProcess,void *Infor)
{
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
			return 0;
		}
			
			
			
			;
		//DEBUG_PROCESS
		//DEBUG_ONLY_THIS_PROCESS
	}
	else
	{
		DebugActiveProcess(*((DWORD*)Infor));//传入DWORD值,而不是传入DWORD指针.
	}

	GetEvent();

	return 1;
}

bool GetEvent()
{
	while(1==WaitForDebugEvent(&DbgEvt,INFINITE))
	{
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

				OutputDebug(L"进程创建事件");

				LPVOID	lpIntAddress	= DbgEvt.u.CreateProcessInfo.lpStartAddress;
				DWORD	dwRet	= 0;
				BYTE	byBack	= 0;
				BYTE	byInt3  = 0xCC;
						g_hProc   = OpenProcess(PROCESS_ALL_ACCESS,0,DbgEvt.dwProcessId);
				if (NULL==g_hProc)
				{
					OutputDebug(L"打开进程失败,进程为:%p,FILE: %s, LINE: %d…",DbgEvt.dwProcessId,__FILE__,__LINE__);
					break;
				}
				//备份原指令
				if(!ReadProcessMemory(g_hProc,lpIntAddress,&byBack,1,&dwRet))
				{
					OutputDebug(L"读内存地址失败,地址为:%p,FILE: %s, LINE: %d…",lpIntAddress,__FILE__,__LINE__);
					break;
				}
				//写入int3断点
				if(!WriteProcessMemory(g_hProc,lpIntAddress,&byInt3,1,&dwRet))
				{	
					OutputDebug(L"写内存地址失败,地址为:%p,FILE: %s, LINE: %d…",lpIntAddress,__FILE__,__LINE__);
					break;
				}

				wchar_t szOPCode[64]	= {0};
				wchar_t szASM[64]		= {0};
				wchar_t szComment[64]	= {0};
				DWORD	dwTempAddr		= (DWORD)lpIntAddress;//起始反汇编地址
				DWORD   dwOPCodeLen		= 0;

				for (;dwOPCodeLen!=-1;)
				{
				dwOPCodeLen = DBG_Disasm((LPVOID)dwTempAddr,szOPCode,szASM,szComment);
				dwTempAddr+=dwOPCodeLen;
				OutputDebug(L"Addr:0x%p %-12s %-16s %s\n",dwTempAddr,szOPCode,szASM,szComment);
				}

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
		//DBG_CONTINUE//异常已经处理
		//DBG_EXCEPTION_NOT_HANDLED//需要系统继续处理
	}
	return 1;
}


DWORD OnExceptionDebugEvent(LPEXCEPTION_DEBUG_INFO pDbgInfo)
{
	EXCEPTION_RECORD* pstcExp = &pDbgInfo->ExceptionRecord;

	DWORD	dwExpCode	= pstcExp->ExceptionCode;	//异常代码
	PVOID	lpExpAddr	= pstcExp->ExceptionAddress;//异常地址

	switch (dwExpCode)
	{
		//非法访问异常
	case EXCEPTION_ACCESS_VIOLATION:
		{
			break;
		}
		//断点异常
	case EXCEPTION_BREAKPOINT:
		{
			OutputDebug(L"Addr:0x%p",lpExpAddr);
			break;
		}
		//内存对齐异常
	case EXCEPTION_DATATYPE_MISALIGNMENT:
		{
			break;
		}
		//无效指令
	case EXCEPTION_ILLEGAL_INSTRUCTION:
		{
			break;
		}
		//除零错误
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
		{
			break;
		}
		//指令不支持当前模式
	case EXCEPTION_PRIV_INSTRUCTION:
		{
			break;
		}
		//单步或硬件断点异常
	case EXCEPTION_SINGLE_STEP:
		{
			break;
		}

	default:
		break;
	}
	return DBG_EXCEPTION_NOT_HANDLED;////DBG_CONTINUE//异常已经处理

}