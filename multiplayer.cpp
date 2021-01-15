#include "messages.h"

#include <thread>
#include <string>
#include <cstdio>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <WinUser.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

#define cchNameMax 32
#define PORT "27030"

HWND hwndMain;
char g_hostName[cchNameMax] = {};
SOCKET g_socket;

//SendMessageW(hwndMain, WM_STARTGAME, 1, 2);

extern "C"
{
#	include "pref.h"
#	include "rtns.h"
	extern PREF Preferences;

	struct buffer_t
	{
		int msgType;
		union
		{
			PREF pref;
			int args[sizeof(PREF) / sizeof(int)];

		};

		operator char* ()
		{
			return reinterpret_cast<char*>(this);
		}
	};

	int g_isConnected = 0;

	void Recieve();

	int CreateServer(HWND hwnd)
	{
		hwndMain = hwnd;

		WSADATA wsaData;
		int iResult;

		SOCKET ListenSocket = INVALID_SOCKET;

		struct addrinfo* result = NULL;
		struct addrinfo hints;

		int iSendResult;

		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0)
		{
			MessageBox(NULL, L"WSAStartup failed.", L"Error", MB_OK);
			return 0;
		}

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		// Resolve the server address and port
		iResult = getaddrinfo(NULL, PORT, &hints, &result);
		if (iResult != 0)
		{
			MessageBox(NULL, L"Failed to resolve the server address.", L"Error", MB_OK);
			WSACleanup();
			return 0;
		}

		// Create a SOCKET for connecting to server
		ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (ListenSocket == INVALID_SOCKET) {
			MessageBox(NULL, L"Failed to create a socket for connecting to server.", L"Error", MB_OK);
			freeaddrinfo(result);
			WSACleanup();
			return 0;
		}

		// Setup the TCP listening socket
		iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
		if (iResult == SOCKET_ERROR)
		{
			MessageBox(NULL, L"Failed to setup the TCP listening socket.", L"Error", MB_OK);
			freeaddrinfo(result);
			closesocket(ListenSocket);
			WSACleanup();
			return 0;
		}

		freeaddrinfo(result);

		iResult = listen(ListenSocket, SOMAXCONN);
		if (iResult == SOCKET_ERROR)
		{
			MessageBox(NULL, L"Failed to listen.", L"Error", MB_OK);
			closesocket(ListenSocket);
			WSACleanup();
			return 0;
		}

		// Accept a client socket
		g_socket = accept(ListenSocket, NULL, NULL);
		if (g_socket == INVALID_SOCKET)
		{
			MessageBox(NULL, L"Failed to accept a client socket.", L"Error", MB_OK);
			closesocket(ListenSocket);
			WSACleanup();
			return 0;
		}

		char buf[64];

		closesocket(ListenSocket);

		g_isConnected = 1;

		std::thread thread(Recieve);
		thread.detach();

		return 1;
	}

	int Connect(HWND hwnd, WCHAR wip[cchNameMax])
	{
		wcstombs_s(NULL, g_hostName, wip, cchNameMax);
		hwndMain = hwnd;

		WSADATA wsaData;
		struct addrinfo* result = NULL,
			* ptr = NULL,
			hints;

		int iResult;

		// Initialize Winsock
		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0)
		{
			MessageBox(NULL, L"WSAStartup failed.", L"Error", MB_OK);
			return 0;
		}

		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		// Resolve the server address and port
		iResult = getaddrinfo(g_hostName, PORT, &hints, &result);
		if (iResult != 0)
		{
			MessageBox(NULL, L"Failed to resolve the server address and port.", L"Error", MB_OK);
			WSACleanup();
			return 0;
		}

		// Attempt to connect to an address until one succeeds
		for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

			// Create a SOCKET for connecting to server
			g_socket = socket(ptr->ai_family, ptr->ai_socktype,
				ptr->ai_protocol);
			if (g_socket == INVALID_SOCKET)
			{
				MessageBox(NULL, L"Failed to create a socket for connecting to server.", L"Error", MB_OK);
				WSACleanup();
				return 0;
			}

			// Connect to server.
			iResult = connect(g_socket, ptr->ai_addr, (int)ptr->ai_addrlen);
			if (iResult == SOCKET_ERROR) {
				closesocket(g_socket);
				g_socket = INVALID_SOCKET;
				continue;
			}
			break;
		}

		freeaddrinfo(result);

		if (g_socket == INVALID_SOCKET) {
			MessageBox(NULL, L"Unable to connect to server.", L"Error", MB_OK);
			WSACleanup();
			return 0;
		}

		g_isConnected = 1;

		StartGame();

		std::thread thread(Recieve);
		thread.detach();

		return 1;
	}

	void SendPref()
	{
		buffer_t buffer;
		buffer.msgType = SEND_SETPREF;
		buffer.pref = Preferences;

		send(g_socket, buffer, sizeof(buffer_t), 0);
	}

	void SendGameStart()
	{
		buffer_t buffer;
		buffer.msgType = SEND_GAMESTART;

		send(g_socket, buffer, sizeof(buffer_t), 0);
	}

	void SendMakeGuess(int x, int y)
	{
		buffer_t buffer;
		buffer.msgType = SEND_MAKEGUESS;
		buffer.args[0] = x;
		buffer.args[1] = y;

		send(g_socket, buffer, sizeof(buffer_t), 0);
	}

	void SendDoButton(int block, int x, int y)
	{
		buffer_t buffer;
		buffer.msgType = SEND_DOBUTTON;
		buffer.args[0] = block;
		buffer.args[1] = x;
		buffer.args[2] = y;

		send(g_socket, buffer, sizeof(buffer_t), 0);
	}

	void SendSetBomb(int x, int y)
	{
		buffer_t buffer;
		buffer.msgType = SEND_SETBOMB;
		buffer.args[0] = x;
		buffer.args[1] = y;

		send(g_socket, buffer, sizeof(buffer_t), 0);
	}

	void SendTrackMouse(int block, int xOld, int yOld, int xNew, int yNew)
	{
		buffer_t buffer;
		buffer.msgType = SEND_TRACKMOUSE;
		buffer.args[0] = block;
		buffer.args[1] = xOld;
		buffer.args[2] = yOld;
		buffer.args[3] = xNew;
		buffer.args[4] = yNew;

		send(g_socket, buffer, sizeof(buffer_t), 0);
	}

	void Recieve()
	{
		buffer_t buffer;

		while (true)
		{
			recv(g_socket, buffer, sizeof(buffer), 0);

			switch (buffer.msgType)
			{
			case SEND_SETPREF:
			{
				auto x = Preferences.xWindow;
				auto y = Preferences.yWindow;
				Preferences = buffer.pref;
				Preferences.xWindow = x;
				Preferences.yWindow = y;
				break;
			}
			case SEND_GAMESTART:
			{
				RecieveStartGame();
				break;
			}
			case SEND_SETBOMB:
			{
				SetBomb(buffer.args[0], buffer.args[1]);
				break;
			}
			case SEND_MAKEGUESS:
			{
				MakeGuessRec(buffer.args[0], buffer.args[1]);
				break;
			}
			case SEND_DOBUTTON:
			{
				DoButton1UpRec(buffer.args[0], buffer.args[1], buffer.args[2]);
				break;
			}
			case SEND_TRACKMOUSE:
			{
				TrackMouseRec(buffer.args[0], buffer.args[1], buffer.args[2], buffer.args[3], buffer.args[4]);
				break;
			}
			default:
				break;
			}
		}

	}

	void Disconnect()
	{
		//closesocket(g_socket);
		WSACleanup();
	}
}