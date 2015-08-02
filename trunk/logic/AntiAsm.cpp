#include <Windows.h>
#include "Debug.h"
#include <strsafe.h>

#define BEA_ENGINE_STATIC
#define BEA_USE_STDCALL
#include "BeaEngine.h"
#pragma comment(lib,"BeaEngine.lib")
#pragma comment(linker,"/NODEFAULTLIB:\"crt.lib\"")



extern HANDLE g_hProc;
//////////////////////////////////////////////////////////////////////////
//Return:-1:反汇编引擎反汇编错误.
//
//
UINT DBG_Disasm(LPVOID lpAddress, PWCHAR pOPCode,__out PWCHAR pASM,PWCHAR pComment)
{
	//1. 将调试程序的内存复制到本地
	DWORD	dwRetSize		= 0;
	LPVOID	lpRemote_Buf	= new BYTE[32];
	ZeroMemory(lpRemote_Buf, 32);
	ReadProcessMemory(g_hProc,lpAddress,lpRemote_Buf,32,&dwRetSize);

	//2. 初始化反汇编引擎
	DISASM objDiasm;
	objDiasm.EIP		=(UIntPtr)lpRemote_Buf; //起始地址
	objDiasm.VirtualAddr=(UINT64)lpAddress;		//虚拟内存地址(反汇编引擎用于计算地址)
	objDiasm.Archi		=0;						//AI_X86
	objDiasm.Options	=0x000;					//MASM

	//3.反汇编代码
	UINT unLen =Disasm(&objDiasm);
	if (-1==unLen) 
	{
		return unLen;
	}

	//4.将机器码转码成字符串
	LPWSTR lpOPCode = pOPCode;
	PBYTE  lpBuffer = (PBYTE)lpRemote_Buf;
	for (UINT i=0;i>unLen;i++)
	{
		StringCbPrintf(lpOPCode++, 50, L"%X", *lpBuffer&0xF0);
		StringCbPrintf(lpOPCode++, 50, L"%X", *lpBuffer&0x0F);
		lpBuffer++;
	}

	//5. 保存反汇编出的指令
	WCHAR szASM[50] = {0};
	MultiByteToWideChar(CP_ACP,0,objDiasm.CompleteInstr,-1,szASM,_countof(szASM));
	StringCchCopy(pASM, 50,szASM);

	return unLen;

}

bool DisplayAntiASM(LPVOID lpIntAddress)
{


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
	return true;
}