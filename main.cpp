#define UNICODE
#pragma comment(linker,"/opt:nowin98")
#pragma comment(lib,"crypt32")
#pragma comment(lib,"ws2_32")
#include<winsock2.h>
#include<windows.h>

TCHAR szClassName[]=TEXT("Window");

LRESULT CALLBACK WndProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	static HWND hEdit1,hEdit2;
	switch(msg)
	{
	case WM_CREATE:
		CreateWindow(TEXT("STATIC"),TEXT("&LONG URL:"),WS_VISIBLE|WS_CHILD|SS_RIGHT|SS_CENTERIMAGE,10,10,128,28,hWnd,0,((LPCREATESTRUCT)lParam)->hInstance,0);
		hEdit1=CreateWindowEx(WS_EX_CLIENTEDGE,TEXT("EDIT"),0,WS_VISIBLE|WS_CHILD|WS_TABSTOP|ES_AUTOHSCROLL,148,10,256,28,hWnd,0,((LPCREATESTRUCT)lParam)->hInstance,0);
		CreateWindow(TEXT("STATIC"),TEXT("&SHORT URL:"),WS_VISIBLE|WS_CHILD|SS_RIGHT|SS_CENTERIMAGE,10,50,128,28,hWnd,0,((LPCREATESTRUCT)lParam)->hInstance,0);
		hEdit2=CreateWindowEx(WS_EX_CLIENTEDGE,TEXT("EDIT"),0,WS_VISIBLE|WS_CHILD|WS_TABSTOP|ES_READONLY|ES_AUTOHSCROLL,10+128+10,50,256,28,hWnd,0,((LPCREATESTRUCT)lParam)->hInstance,0);
		CreateWindow(TEXT("BUTTON"),TEXT("取得"),WS_VISIBLE|WS_CHILD|WS_TABSTOP|BS_DEFPUSHBUTTON,10,90,256,28,hWnd,(HMENU)IDOK,((LPCREATESTRUCT)lParam)->hInstance,0);
		SetFocus(hEdit1);
		break;
	case WM_COMMAND:
		if(LOWORD(wParam)==IDOK)
		{	
			DWORD dwURLLength;
			dwURLLength=GetWindowTextLengthA(hEdit1);
			if(dwURLLength==0){SetFocus(hEdit1);break;}
			SetWindowText(hEdit2,0);

			WSADATA wsa;
			WSAStartup(MAKEWORD(1,1),&wsa);
			SOCKET s=socket(AF_INET,SOCK_STREAM,0);
			struct sockaddr_in addr;
			struct hostent *host;
			addr.sin_family=AF_INET;
			addr.sin_port=htons(80);
			host=gethostbyname("api.bit.ly");
			addr.sin_addr=*((struct in_addr*)*host->h_addr_list);
			connect(s,(struct sockaddr*)&addr,sizeof(addr));
			
			LPSTR szURL=0,szRequest=0;
			szURL=(LPSTR)GlobalAlloc(GMEM_FIXED,dwURLLength+1);
			szRequest=(LPSTR)GlobalAlloc(GMEM_FIXED,dwURLLength+123+1);
			//if(!szURL||!szRequest)
			//{
			//	GlobalFree(szURL);
			//	GlobalFree(szRequest);
			//}
			GetWindowTextA(hEdit1,szURL,dwURLLength+1);
			lstrcpyA(szRequest,"GET http://api.bit.ly/shorten?version=2.0.1&login=kenjinote&apiKey=R_cb6e35bb73d7644fbabbd57df5b85d94&longUrl=");
			lstrcatA(szRequest,szURL);
			GlobalFree(szURL);
			lstrcatA(szRequest," HTTP/1.0\r\n\r\n");
			//リクエストを送信
			int iResult;
			iResult=send( s,szRequest,(int)lstrlenA(szRequest),0 );
			GlobalFree(szRequest);
			if (iResult == SOCKET_ERROR){
				closesocket(s);
				WSACleanup();
				MessageBox(hWnd,TEXT("エラー"),0,0);
				return 1;
			}
			//レスポンスを受信
			CHAR szBuffer[2048];
			do {
				iResult = recv(s,szBuffer,sizeof(szBuffer)-1,0);
				if ( iResult > 0 )
				{
					szBuffer[iResult]=0;
				}
			} while( iResult > 0 );
			shutdown(s,SD_BOTH);
			closesocket(s);
			WSACleanup();
			//レスポンス文字列を解析
			CHAR*p;
			p=strstr(szBuffer,"\"shortUrl\"");
			if(p)
			{
				p+=13;
				CHAR*q=p;
				while(*q!='"'&&*q!=0)q++;
				*q=0;
				SetWindowTextA(hEdit2,p);
				SetFocus(hEdit2);
				SendMessage(hEdit2,EM_SETSEL,0,-1);
			}
			else
			{
				SetWindowTextA(hEdit2,"取得できませんでした。");
				SetFocus(hEdit1);
				SendMessage(hEdit1,EM_SETSEL,0,-1);
			}
			return TRUE;
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return(DefDlgProc(hWnd,msg,wParam,lParam));
	}
	return 0;
}

EXTERN_C void __cdecl WinMainCRTStartup()
{
	MSG msg;
	HINSTANCE hInstance=GetModuleHandle(0);
	WNDCLASS wndclass={0,WndProc,0,DLGWINDOWEXTRA,hInstance,0,LoadCursor(0,IDC_ARROW),0,0,szClassName};
	RegisterClass(&wndclass);
	HWND hWnd=CreateWindow(szClassName,TEXT("Window"),WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,0,CW_USEDEFAULT,0,0,0,hInstance,0);
	ShowWindow(hWnd,SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while(GetMessage(&msg,0,0,0))
	{
		if(!IsDialogMessage(hWnd,&msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	ExitProcess(msg.wParam);
}

#if _DEBUG
void main(){}
#endif