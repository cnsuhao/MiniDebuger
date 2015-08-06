
#include "StartDebug.h"
#include <Windows.h>
DWORD	dwAttachPID			=	0;

bool	InputDebuggedPath	=	false;//是否以路径传入方式启动调试程序
extern WCHAR *wcInputDebuggedPath;

int main(int argc,char* argv[])
{
	char cPePath[MAX_PATH];

	if(NULL!=argv[1])
	{
		strcpy_s(cPePath,argv[1]);
		DWORD dwNum = MultiByteToWideChar (CP_ACP, 0, cPePath, -1, NULL, 0);
		wchar_t *pwText=new wchar_t[dwNum];
		if(!pwText)
		{
			delete []pwText;
		}
		else
		{

			MultiByteToWideChar (CP_ACP, 0, cPePath, -1, pwText, dwNum);

			StartDebug(true,pwText);

			delete []pwText;
		}
		//AfxGetApp()->m_lpCmdLine;

		while(true)
		{


			if (0!=dwAttachPID)//优先附加方式,
			{
				DWORD dwPIDBuffer=dwAttachPID;
				StartDebug(false,&dwPIDBuffer);

			}
			else if(true==InputDebuggedPath)

			{

				InputDebuggedPath	=	false;
				StartDebug(true,wcInputDebuggedPath);



			}

			Sleep(500);

		}

		return 0;
	}
}