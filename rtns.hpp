#pragma once

#include <algorithm>
#include <utility>
#include <vector>

#include <chrono>
#include <random>

#include <gtkmm.h>

enum class Status
{
    Play = 0b00001,
    Pause = 0b00010,
    Panic = 0b00100,
    Icon = 0b01000,
    Demo = 0b10000,
};

bool operator==(Status lhs, Status rhs);

enum class Button
{
    Down = 0,
    Win = 1,
    Lose = 2,
    Caution = 3,
    Happy = 4
};

enum Block
{
    Blank = 0,
    One = 1,
    Two = 2,
    Three = 3,
    Four = 4,
    Five = 5,
    Six = 6,
    Seven = 7,
    Eight = 8,
    Guess = 9,
    Bomb = 10,
    Wrong = 11,
    Explode = 12,
    GuessUp = 13,
    BombUp = 14,
    BlankUp = 15
};

class GameLogic
{
public:
    GameLogic(Gtk::Window *mainWindow, int x = 9, int y = 9, int bombCount = 10);

    void StartGame();
    void StartGame(int x, int y, int bombCount);

    bool DoTimer();

    void TrackMouse(int xNew, int yNew, bool block);
    void MakeGuess(int x, int y);
    void DoButton1Up(bool block);

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

protected:
    bool isInRange(int x, int y);

    void ClearBomb(int x, int y);

    void SetBomb(char &blk);
    static bool IsBomb(char iBlk);
    bool IsBomb(int x, int y);

    void SetVisit(int x, int y);
    static bool isVisit(char iBlk);
    bool isVisit(int x, int y);

    static bool GuessBomb(char iBlk);
    bool GuessBomb(int x, int y);
    bool GuessMark(int x, int y);

    static void SetBlk(char &blk, char iBlk);
    void SetBlk(int x, int y, char iBlk);
    void ClearField();

    void ChangeBlk(int x, int y, char iBlk);

    void ShowBombs(int iBlk);
    int CountBombs(int xCenter, int yCenter);

    void GameOver(bool fWinLose);

    void StepXY(int x, int y);
    void StepBox(int x, int y);
    void StepSquare(int x, int y);
    void StepBlock(int xCenter, int yCenter, bool block);

    void PushBoxDown(int x, int y);
    void PopBoxUp(int x, int y);

    int CountMarks(int xCenter, int yCenter);

    int m_width;
    int m_height;

    std::vector<char> m_field;

    int m_bombStart;
    int m_bombLeft;
    int m_blockVisit;
    int m_blockVisitMax;

    int m_seconds;
    bool m_timer = false;
    bool m_OldTimerStatus = false;

    int m_xCurrent = -1;
    int m_yCurrent = -1;

#define iStepMax 100

    int rgStepX[iStepMax];
    int rgStepY[iStepMax];

    int iStepMac;

    Status m_status;
    Button m_button;

    Gtk::Window* r_mainWindow;
};