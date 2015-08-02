#pragma once


bool Deal_CPEV();//进程创建事件
bool InvalidInt3(DWORD dwIndex);//使int3断点失效
bool DeleteInt3(DWORD dwIndex);
bool ClearInt3(DWORD dwIndex);
int SerarchInt3(LPVOID lpAddress);
bool WriteInt3(LPVOID	lpAddress);

DWORD OnExceptionDebugEvent(LPEXCEPTION_DEBUG_INFO pDbgInfo);