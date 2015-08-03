//这是逻辑层与UI的接口.
#include <afx.h>
#include <Windows.h>
#include "Interface.h"

CString csCallFunctionName = "";
CString csCallArgv	= "";

//UI to Logic

/*
命令行解析
*/
void CommandParsing(CString CommandText)
{
	int iBlackIndex=0;
	CString m_functionName="";
	CString m_argv="";

	iBlackIndex=CommandText.Find(L" ");
	if (-1==iBlackIndex)
	{
		m_functionName=CommandText;
	}
	else
	{
		m_functionName=CommandText.Left(iBlackIndex);
		m_argv=CommandText.Mid(iBlackIndex+1);
	}
	csCallFunctionName=m_functionName;
	csCallArgv=m_argv;

	return;

}

//检测需要执行的函数
void AutoAnalysisCommandParsing()
{

	//比对函数表,如果执行错误,设置错误标记.用于UI端检测.

	if (L"bp"==csCallFunctionName)
	{
		//int3断点,参数不能为空
		if(""==csCallArgv)
		{
			return;
		}
		//先断下来再



	}
	else
	{

	}













	//清理函数和参数
	csCallFunctionName	= "";
	csCallArgv			= "";

	return;
}





//Logic to UI


/*
对传入的字符串进行格式化处理,转输出到UI窗口
DWORD dwCode:标识打印格式及需要附加的来源等
*/
bool Printf2UI(CString stPrint,DWORD dwCode)
{
	switch (dwCode)
	{
	case MINIF_DEBUG_STRING:
		{

		}




	default:
		break;
	};

	return true;
}









