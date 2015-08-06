#pragma once


bool Deal_CPEV();//进程创建事件
bool Deal_EPDE();//被调试这退出
bool Deal_ODSE();//调试信息输出

bool ListInt3();
bool InvalidInt3(DWORD dwIndex);//使int3断点失效
bool ValidInt3(DWORD dwIndex);//使有效
bool DeleteInt3(DWORD dwIndex);
bool WriteInt3(LPVOID  lpAddress);




bool ClearInt3(DWORD dwIndex);//!!外部勿使用,它没有清理Vector中的东东.会导致不可预料的结果.
int SerarchInt3(LPVOID lpAddress);


DWORD OnExceptionDebugEvent(LPEXCEPTION_DEBUG_INFO pDbgInfo);
