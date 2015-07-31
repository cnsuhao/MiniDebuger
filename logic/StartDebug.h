#pragma once


//bool bNewProcess	:以调试的方式启动一个进程(1),附加到一个进程(0)
//void *Infor		:指向一个字符串或者DWORD值,这取决于bNewProcess.
int StartDebug(bool bNewProcess,void *Infor);