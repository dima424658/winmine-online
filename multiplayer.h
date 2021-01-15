#ifndef cchNameMax
#define cchNameMax 32
#endif

extern int g_isConnected;

int CreateServer(HWND hwnd);
int Connect(HWND, WCHAR[cchNameMax]);

void SendPref();
void SendGameStart();
void SendMakeGuess(int, int);
void SendDoButton(int, int, int);
void SendSetBomb(int x, int y);
void SendTrackMouse(int, int, int, int, int);