#pragma once


#include <Windows.h>

enum Printf2UICode{
	MINIF_DEBUG_STRING=1,
	MINIF_ANTIASM,
	MINIF_REGISTER,
	MINIF_MODULE,
	MINIF_INT3,
	MINIF_HARDWARE_BREAKPOINT,
	MINIF_SOFTWARE_BREAKPOINT

};



void CommandParsing(CString CommandText);
void AutoAnalysisCommandParsing();
bool Printf2UI(CString stPrint,DWORD dwCode);