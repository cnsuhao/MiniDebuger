#include <Windows.h>
#include "Debug.h"
#include "AntiAsm.h"
#include "DealEvent.h"
#include <vector>

using std::vector;

extern _DEBUG_EVENT	DbgEvt;	
HANDLE g_hProc;//被调试者的句柄


//////////////////////////////////////////////////////////////////////////
//一组用于保存int3状态的向量
vector<BYTE>Int3Back;	//原值
vector<void*>Int3Addr;	//地址
vector<bool>Int3Value;  //该断点是否有效?
//////////////////////////////////////////////////////////////////////////


//进程创建处理.
bool Deal_CPEV()
{
	OutputDebug(L"进程创建事件");

	//OEP地址
	LPVOID	lpIntAddress	= DbgEvt.u.CreateProcessInfo.lpStartAddress;

	g_hProc   = OpenProcess(PROCESS_ALL_ACCESS,0,DbgEvt.dwProcessId);
	if (NULL==g_hProc)
	{
		OutputDebug(L"打开进程失败,进程为:%p,FILE: %s, LINE: %d…",DbgEvt.dwProcessId,__FILE__,__LINE__);
		return true;
	}

	//在OEP放一个软件断点.
	WriteInt3(lpIntAddress);

	return true;
}

//使用Int3断点无效
bool InvalidInt3(DWORD dwIndex)
{
	if ((dwIndex<Int3Addr.size())&&(!Int3Addr.empty()))
	{
		Int3Value[dwIndex]=0;
		if(false==ClearInt3(dwIndex))
		{
			return false;
		}
		return true;
	}
	return false;
}

//删除Int3断点
bool DeleteInt3(DWORD dwIndex)
{
	if ((dwIndex<Int3Addr.size())&&(!Int3Addr.empty()))
	{
		if (false==ClearInt3(dwIndex))
		{
			return false;
		}

		Int3Addr.erase(Int3Addr.begin()+dwIndex);
		Int3Back.erase(Int3Back.begin()+dwIndex);
		Int3Value.erase(Int3Value.begin()+dwIndex);

		return true;
	}

	return false;
}

//恢复Int3下断前的值,但是不操作vector
bool ClearInt3(DWORD dwIndex)
{ 
	if ((dwIndex<Int3Addr.size())&&(!Int3Addr.empty()))
	{
		DWORD	dwRet	= 0;	
		//写入int3断点
		if(!WriteProcessMemory(g_hProc,Int3Addr[dwIndex],&Int3Back,1,&dwRet))
		{	
			OutputDebug(L"写内存地址失败,地址为:%p,FILE: %s, LINE: %d…",Int3Addr[dwIndex],__FILE__,__LINE__);

			DWORD OldProtect	= 0;//重要的值,用来存放保护属性
			PDWORD lpOldProtect =&OldProtect;

			DWORD OldProtectB	= 0;//一个不会使用的值,用于存放保护属性
			PDWORD  lpOldProtectB=&OldProtectB;

			if(0!=VirtualProtectEx(g_hProc,Int3Addr[dwIndex],4*1024,PAGE_READWRITE,lpOldProtect))
			{
				if(!WriteProcessMemory(g_hProc,Int3Addr[dwIndex],&Int3Back,1,&dwRet))
				{
					OutputDebug(L"写内存地址失败,地址为:%p,FILE: %s, LINE: %d…",Int3Addr[dwIndex],__FILE__,__LINE__);
					if(0==VirtualProtectEx(g_hProc,Int3Addr[dwIndex],4*1024,*lpOldProtect,lpOldProtectB))
					{
						OutputDebug(L"更改内存地址的属性变回原属性失败,地址为:%p,FILE: %s, LINE: %d…,原属性为:%d,目的属性为:PAGE_READWRITE",Int3Addr[dwIndex],__FILE__,__LINE__,OldProtect);
						return false;
					}
					return false;
				}

				if(0==VirtualProtectEx(g_hProc,Int3Addr[dwIndex],4*1024,*lpOldProtect,lpOldProtectB))
				{
					OutputDebug(L"更改内存地址的属性变回原属性失败,地址为:%p,FILE: %s, LINE: %d…,原属性为:%d,目的属性为:PAGE_READWRITE",Int3Addr[dwIndex],__FILE__,__LINE__,OldProtect);
					return false;
				}

				return true;
			}
			else
			{
				OutputDebug(L"更改内存地址属性失败,地址为:%p,FILE: %s, LINE: %d…,旧属性为:%d,新属性为:PAGE_READWRITE",Int3Addr[dwIndex],__FILE__,__LINE__,OldProtect);
				return false;
			}
		}
		return true;
	}

	return false;
}


//搜索指定地址是否存在于已记录Int3列表,存在则返回下标.-1没找到.
int SerarchInt3(LPVOID lpAddress)
{
	vector<void*>::iterator it;

	int iIndex=0;//序号
	for (it=Int3Addr.begin();it<Int3Addr.end();++it,++iIndex)
	{
		if (*it==lpAddress)
		{
			return iIndex;
		}
	}

	return -1;//空,或者找不到

}


//写入int3断点,如果备份则使其有效.传入需要写断点的地址
//返回值:false不代表完全的失败.还得看具体情况,例如其它工作已经完成.只是原来的内存页属性没办法写回.
bool WriteInt3(LPVOID	lpAddress)
{
	BYTE	byNowValue	= 0;//盛放目标内存的探测值
	DWORD	dwRetN		= 0;
	BYTE	byInt3		= 0xCC;

	int iIndex=SerarchInt3(lpAddress);


	if (-1!=iIndex)
	{
		if(false==Int3Value[iIndex])
		{
			//判断目标位置是否还是0xCC.不是则修改.

			if(!ReadProcessMemory(g_hProc,lpAddress,&byNowValue,1,&dwRetN))
			{
				DWORD OldProtect	= 0;//重要的值,用来存放保护属性
				PDWORD lpOldProtect =&OldProtect;

				DWORD OldProtectB	= 0;//一个不会使用的值,用于存放保护属性
				PDWORD  lpOldProtectB=&OldProtectB;

				OutputDebug(L"读内存地址失败,地址为:%p,FILE: %s, LINE: %d…",lpAddress,__FILE__,__LINE__);
				if(0!=VirtualProtectEx(g_hProc,lpAddress,4*1024,PAGE_READWRITE,lpOldProtect))
				{

					if(!ReadProcessMemory(g_hProc,lpAddress,&byNowValue,1,&dwRetN))
					{
						OutputDebug(L"改内存地址属性成功后,读内存地址竟然!!失败,地址为:%p,FILE: %s, LINE: %d…",lpAddress,__FILE__,__LINE__);
						//恢复原属性.
						if(0==VirtualProtectEx(g_hProc,lpAddress,4*1024,*lpOldProtect,lpOldProtectB))
						{
							OutputDebug(L"更改内存地址的属性变回原属性失败,地址为:%p,FILE: %s, LINE: %d…,原属性为:%d,目的属性为:PAGE_READWRITE",lpAddress,__FILE__,__LINE__,OldProtect);
							return false;
						}
						return false;
					}
					else
					{
						if (byInt3!=byNowValue)
						{
							//!!这里将检测到的非0xCC的值写入到了vector中,而没有考虑此值和vector原来的值的关系.
							if(!WriteProcessMemory(g_hProc,lpAddress,&byInt3,1,(SIZE_T*)&Int3Back[iIndex]))
							{	
								OutputDebug(L"写内存地址失败,地址为:%p,FILE: %s, LINE: %d…",lpAddress,__FILE__,__LINE__);
								if(0==VirtualProtectEx(g_hProc,lpAddress,4*1024,*lpOldProtect,lpOldProtectB))
								{
									OutputDebug(L"更改内存地址的属性变回原属性失败,地址为:%p,FILE: %s, LINE: %d…,原属性为:%d,目的属性为:PAGE_READWRITE",lpAddress,__FILE__,__LINE__,OldProtect);
									return false;
								}
								return false;
							}
							if(0==VirtualProtectEx(g_hProc,lpAddress,4*1024,*lpOldProtect,lpOldProtectB))
							{
								OutputDebug(L"更改内存地址的属性变回原属性失败,地址为:%p,FILE: %s, LINE: %d…,原属性为:%d,目的属性为:PAGE_READWRITE",lpAddress,__FILE__,__LINE__,OldProtect);
								return false;
							}
							return true;
						}				
						Int3Value[iIndex]=true;
						if(0==VirtualProtectEx(g_hProc,lpAddress,4*1024,*lpOldProtect,lpOldProtectB))
						{
							OutputDebug(L"更改内存地址的属性变回原属性失败,地址为:%p,FILE: %s, LINE: %d…,原属性为:%d,目的属性为:PAGE_READWRITE",lpAddress,__FILE__,__LINE__,OldProtect);
							return false;
						}
						return true;
					}

				}
				else
				{
					OutputDebug(L"更改内存地址属性失败,地址为:%p,FILE: %s, LINE: %d…,旧属性为:%d,新属性为:PAGE_READWRITE",lpAddress,__FILE__,__LINE__,OldProtect);
					return false;
				}

			}
			else
			{
				if(byInt3!=byNowValue)
				{
					if(!WriteProcessMemory(g_hProc,lpAddress,&byInt3,1,(SIZE_T*)&Int3Back[iIndex]))
					{	
						OutputDebug(L"写内存地址失败,地址为:%p,FILE: %s, LINE: %d…",lpAddress,__FILE__,__LINE__);
						return false;
					}

				}
				//读内存成功则修改Int3Value,否则false
				Int3Value[iIndex]=true;
				return  true;
			}
		}
		else
		{
			return true;//该断点现在已经有效.
		}
		return true;
	}
	//这个断点未记录过.进入记录环节:


	DWORD	dwRet	= 0;
	BYTE	byBack	= 0;

	//备份原指令
	if(!ReadProcessMemory(g_hProc,lpAddress,&byBack,1,&dwRet))
	{
		DWORD OldProtect	= 0;//重要的值,用来存放保护属性
		PDWORD lpOldProtect =&OldProtect;

		DWORD OldProtectB	= 0;//一个不会使用的值,用于存放保护属性
		PDWORD  lpOldProtectB=&OldProtectB;

		OutputDebug(L"读内存地址失败,地址为:%p,FILE: %s, LINE: %d…",lpAddress,__FILE__,__LINE__);
		if(0!=VirtualProtectEx(g_hProc,lpAddress,4*1024,PAGE_READWRITE,lpOldProtect))
		{

			if(!ReadProcessMemory(g_hProc,lpAddress,&byBack,1,&dwRet))
			{
				OutputDebug(L"改内存地址属性成功后,读内存地址竟然!!失败,地址为:%p,FILE: %s, LINE: %d…",lpAddress,__FILE__,__LINE__);
				//恢复原属性.
				if(0==VirtualProtectEx(g_hProc,lpAddress,4*1024,*lpOldProtect,lpOldProtectB))
				{
					OutputDebug(L"更改内存地址的属性变回原属性失败,地址为:%p,FILE: %s, LINE: %d…,原属性为:%d,目的属性为:PAGE_READWRITE",lpAddress,__FILE__,__LINE__,OldProtect);
					return false;
				}

				return false;
			}
			else
			{
				if(!WriteProcessMemory(g_hProc,lpAddress,&byInt3,1,&dwRet))
				{	
					OutputDebug(L"写内存地址失败,地址为:%p,FILE: %s, LINE: %d…",lpAddress,__FILE__,__LINE__);
					if(0==VirtualProtectEx(g_hProc,lpAddress,4*1024,*lpOldProtect,lpOldProtectB))
					{
						OutputDebug(L"更改内存地址的属性变回原属性失败,地址为:%p,FILE: %s, LINE: %d…,原属性为:%d,目的属性为:PAGE_READWRITE",lpAddress,__FILE__,__LINE__,OldProtect);
						return false;
					}
					return false;
				}

				Int3Back.push_back(byBack);
				Int3Addr.push_back(lpAddress);
				Int3Value.push_back(true);

				if(0==VirtualProtectEx(g_hProc,lpAddress,4*1024,*lpOldProtect,lpOldProtectB))
				{
					OutputDebug(L"更改内存地址的属性变回原属性失败,地址为:%p,FILE: %s, LINE: %d…,原属性为:%d,目的属性为:PAGE_READWRITE",lpAddress,__FILE__,__LINE__,OldProtect);
					return false;
				}

				return true;
			}
			OutputDebug(L"更改内存地址属性失败,地址为:%p,FILE: %s, LINE: %d…,旧属性为:%d,新属性为:PAGE_READWRITE",lpAddress,__FILE__,__LINE__,OldProtect);
			return false;
		}
	}
	//写入int3断点
	if(!WriteProcessMemory(g_hProc,lpAddress,&byInt3,1,&dwRet))
	{	
		OutputDebug(L"写内存地址失败,地址为:%p,FILE: %s, LINE: %d…",lpAddress,__FILE__,__LINE__);
		DWORD OldProtect	= 0;//重要的值,用来存放保护属性
		PDWORD lpOldProtect =&OldProtect;

		DWORD OldProtectB	= 0;//一个不会使用的值,用于存放保护属性
		PDWORD  lpOldProtectB=&OldProtectB;

		if(0!=VirtualProtectEx(g_hProc,lpAddress,4*1024,PAGE_READWRITE,lpOldProtect))
		{
			if(!WriteProcessMemory(g_hProc,lpAddress,&byInt3,1,&dwRet))
			{
				OutputDebug(L"写内存地址失败,地址为:%p,FILE: %s, LINE: %d…",lpAddress,__FILE__,__LINE__);
				if(0==VirtualProtectEx(g_hProc,lpAddress,4*1024,*lpOldProtect,lpOldProtectB))
				{
					OutputDebug(L"更改内存地址的属性变回原属性失败,地址为:%p,FILE: %s, LINE: %d…,原属性为:%d,目的属性为:PAGE_READWRITE",lpAddress,__FILE__,__LINE__,OldProtect);
					return false;
				}
				return false;
			}
			Int3Back.push_back(byBack);
			Int3Addr.push_back(lpAddress);
			Int3Value.push_back(true);

			if(0==VirtualProtectEx(g_hProc,lpAddress,4*1024,*lpOldProtect,lpOldProtectB))
			{
				OutputDebug(L"更改内存地址的属性变回原属性失败,地址为:%p,FILE: %s, LINE: %d…,原属性为:%d,目的属性为:PAGE_READWRITE",lpAddress,__FILE__,__LINE__,OldProtect);
				return false;
			}

			return true;
		}
		else
		{
			OutputDebug(L"更改内存地址属性失败,地址为:%p,FILE: %s, LINE: %d…,旧属性为:%d,新属性为:PAGE_READWRITE",lpAddress,__FILE__,__LINE__,OldProtect);
			return false;
		}

	}
	Int3Back.push_back(byBack);
	Int3Addr.push_back(lpAddress);
	Int3Value.push_back(true);

	return true;
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