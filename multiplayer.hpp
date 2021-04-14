#include <sys/socket.h>
#include <sys/types.h>

#include <netdb.h>
#include <netinet/ip.h>
#include <unistd.h>

#include <future>
#include <mutex>
#include <stdexcept>
#include <thread>

#include "rtns.hpp"

enum class message_t
{
    GameStart = 0x0001,
    SetPref = 0x0002,
    SetBomb = 0x0004,
    MakeGuess = 0x0008,
    DoButton = 0x0016,
    TrackMouse = 0x0032
};

constexpr size_t cchNameMax{32};

typedef struct
{
    short wGameType;
    int Mines;
    int Height;
    int Width;
    int xWindow;
    int yWindow;
    int fSound;
    int fMark;
    int fTick;
    int fMenu;
    int fColor;
    int rgTime[3];
    char szBegin[cchNameMax];
    char szInter[cchNameMax];
    char szExpert[cchNameMax];
} PREF;

struct buffer_t
{
    message_t msgType;
    union
    {
        PREF pref;
        int args[sizeof(PREF) / sizeof(int)];
    };

    operator char *()
    {
        return reinterpret_cast<char *>(this);
    }
};

class Connection : public GameLogic
{
public:
    Connection(Gtk::Window *mainWindow, int x = 9, int y = 9, int bombCount = 10);

    void Disconnect()
    {
        if (m_socket >= 0)
            close(m_socket);
    }

    void RecieveStartGame();
    void StartGame();
    void StartGame(int x, int y, int bombCount);

    bool DoTimer();

    void TrackMouse(int xNew, int yNew, bool block);
    void TrackMouseRec(int xOld, int yOld, int xNew, int yNew, int block);
    
    void MakeGuess(int x, int y);
    void MakeGuessRec(int x, int y);
    
    void DoButton1Up(bool block);
    void DoButton1UpRec(int block, int x, int y);

    void PauseGame();
    void ResumeGame();

    void UpdateBombCount(int BombAdjust);

    void SetButton(Button);
    Button GetButton();
    int GetTime();
    int GetBomb();

    size_t GetWidth();
    size_t GetHeight();
    
    Status GetStatus();

    char &Block(int x, int y);

    void SendPref();
    void SendGameStart();
    void SendMakeGuess(int, int);
    void SendDoButton(int, int, int);
    void SendSetBomb(int x, int y);
    void SendTrackMouse(int, int, int, int, int);

    void Recieve();

    std::future<std::pair<bool, std::string>> CreateServer(unsigned short port);
    std::future<std::pair<bool, std::string>> Connect(std::string addr, unsigned short port);

    bool IsConnected() const noexcept
    {
        return m_socket > 0;
    }

protected:
    std::recursive_mutex m_mutex;
    int m_socket;
};