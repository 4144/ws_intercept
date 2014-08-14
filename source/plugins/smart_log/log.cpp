#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include "log.h"

static DWORD WINAPI setup_console(LPVOID param);
static DWORD WINAPI initialize(LPVOID param);

void WINAPI log_ws(SOCKET *s, const char *buf, int *len, int *flags);

static inline void help_text();
static inline void send_toggle();
static inline void recv_toggle();

static DWORD threadIDConsole = 0;

static DWORD plugin_id_send = 0;
static DWORD plugin_id_recv = 0;

static volatile BOOL setup_flag = 0;

BOOL APIENTRY DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
	switch(reason)
	{
		case DLL_PROCESS_ATTACH:
			CreateThread(NULL,0,setup_console,NULL,0,&threadIDConsole);
			CreateThread(NULL,0,initialize,NULL,0,NULL);
			break;
		case DLL_PROCESS_DETACH:
			unregister_handler(plugin_id_send, WS_HANDLER_SEND);
			unregister_handler(plugin_id_recv, WS_HANDLER_RECV);
			if (threadIDConsole)
				PostThreadMessage(threadIDConsole, WM_QUIT, 0, 0);
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
	}
	return TRUE;
}

static DWORD WINAPI initialize(LPVOID params)
{
	while(!setup_flag);
	char command;
	help_text();
	LOG("Initialized. Type h for list of commands");
	do
	{
		LOGn("\n> ");
		scanf("%c",&command);
		switch(command)
		{
			case 's': send_toggle(); break;
			case 'r': recv_toggle(); break;
			case 'h': help_text(); break;
			default: break;
		}
	} while(command != 'q');
	return 0;	
}

static inline void help_text()
{
	LOG("Commands");
	LOG("h - Display this list");
	LOG("s - Toggle Send logging");
	LOG("r - Toggle Recv logging");
}

static inline void recv_toggle()
{
	if(!plugin_id_recv)
		plugin_id_recv = register_handler(log_ws, WS_HANDLER_RECV, (char*)"");
	else
	{
		unregister_handler(plugin_id_recv, WS_HANDLER_RECV);
		plugin_id_recv = 0;
	}
}

static inline void send_toggle()
{
	if(!plugin_id_send)
		plugin_id_send = register_handler(log_ws, WS_HANDLER_SEND, (char*)"");
	else
	{
		unregister_handler(plugin_id_send, WS_HANDLER_SEND);
		plugin_id_send = 0;
	}
}

void WINAPI log_ws(SOCKET *s, const char *buf, int *len, int *flags) //Note that you're given pointers to everything! (buf was already a pointer though)
{
	struct sockaddr_in info;
	int infolen;
	getpeername(*s,(struct sockaddr*)(&info),&infolen);
	const int port = ntohs(info.sin_port);
	
	LOG("%s:%u, Len %d, Flags %d, socket %u",inet_ntoa(info.sin_addr), port, *len, *flags, *s);
	LOGn("Data: ");  
	for(int i = 0; i < *len; i++) 
		LOGn("%02X ",(unsigned char)buf[i]);
	LOGn("\n");
	return;
}

static DWORD WINAPI setup_console(LPVOID param)
{
	AllocConsole();
	freopen("CONOUT$","w",stdout);
	freopen("CONIN$","r",stdin);
	setup_flag = 1;
	while(1)
	{
		MSG msg;
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
		{
			switch (msg.message) 
			{
				case WM_QUIT:
					FreeConsole();		
					return msg.wParam;
			}
		}
	}
	return 0;
}
