#include "rtns.hpp"

static constexpr char MaskBomb = 0x80;
static constexpr char MaskVisit = 0x40;
static constexpr char MaskFlags = 0xE0;
static constexpr char MaskData = 0x1F;

static constexpr char NOTMaskBomb = 0x7F;

GameLogic::GameLogic(Gtk::Window *mainWindow, int x, int y, int bombCount)
    : m_width{x}, m_height{y}, m_bombStart{bombCount}, r_mainWindow{mainWindow}
{
    ClearField();
}


void GameLogic::StartGame(int x, int y, int bombCount)
{
    m_width = x;
    m_height = y;
    m_bombStart = bombCount;

    StartGame();
}

void GameLogic::StartGame()
{
    m_timer = false;

    ClearField();
    m_button = Button::Happy;

    m_bombLeft = 0;

    std::random_device device;
    std::default_random_engine engine{device()};
    std::uniform_int_distribution<size_t> dist{0, static_cast<size_t>(m_width * m_height - 1)};

    while (m_bombLeft != m_bombStart)
    {
        auto &blk = m_field.at(dist(engine));
        if (!IsBomb(blk))
        {
            SetBomb(blk);
            ++m_bombLeft;
        }
    }

    m_seconds = 0;
    m_blockVisit = 0;
    m_blockVisitMax = (m_width * m_height) - m_bombLeft;
    m_status = Status::Play;

    UpdateBombCount(0);
}

bool GameLogic::DoTimer()
{
    if (m_timer && (m_seconds < 999))
        ++m_seconds;

    r_mainWindow->get_action_group("game")->activate_action("update_graphics");

    return m_timer;
}

void GameLogic::TrackMouse(int xNew, int yNew, bool block)
{
    if ((xNew == m_xCurrent) && (yNew == m_yCurrent))
        return;

    {
        int xOld = m_xCurrent;
        int yOld = m_yCurrent;

        m_xCurrent = xNew;
        m_yCurrent = yNew;

        if (block)
        {
            for (int x = xOld - 1; x <= xOld + 1; x++)
                for (int y = yOld - 1; y <= yOld + 1; y++)
                    if (isInRange(x, y) && !isVisit(x, y))
                        PopBoxUp(x, y);

            for (int x = xNew - 1; x <= xNew + 1; x++)
                for (int y = yNew - 1; y <= yNew + 1; y++)
                    if (isInRange(x, y) && !isVisit(x, y))
                        PushBoxDown(x, y);
        }
        else
        {
            if (isInRange(xOld, yOld) && !isVisit(xOld, yOld))
                PopBoxUp(xOld, yOld);

            if (isInRange(xNew, yNew) && !(isVisit(xNew, yNew) || GuessBomb(xNew, yNew)))
                PushBoxDown(xNew, yNew);
        }
    }
}

void GameLogic::MakeGuess(int x, int y)
{
    if (!isInRange(x, y))
        return;

    auto &iBlk = Block(x, y);

    if (!isVisit(x, y))
    {
        if (GuessBomb(x, y))
        {
            //if (Preferences.fMark)
            //    iBlk = iBlkGuessUp;
            //else

            SetBlk(iBlk, Block::BlankUp);
            UpdateBombCount(+1);
        }
        else if (GuessMark(x, y))
        {
            SetBlk(iBlk, Block::BlankUp);
        }
        else
        {
            SetBlk(iBlk, Block::BombUp);
            UpdateBombCount(-1);
        }

        if (GuessBomb(x, y) && m_blockVisit == m_blockVisitMax)
            GameOver(true);
    }
}

void GameLogic::DoButton1Up(bool block)
{
    if (isInRange(m_xCurrent, m_yCurrent))
    {

        if ((m_blockVisit == 0) && (m_seconds == 0))
        {
            m_timer = true;
            m_seconds = 0;

            Glib::signal_timeout().connect(sigc::mem_fun(*this, &GameLogic::DoTimer), 1000);
        }

        if (m_status != Status::Play)
            m_xCurrent = m_yCurrent = -2;
        else
        {
            if (block)
                StepBlock(m_xCurrent, m_yCurrent, block);
            else if (!(isVisit(m_xCurrent, m_yCurrent) || GuessBomb(m_xCurrent, m_yCurrent)))
                StepSquare(m_xCurrent, m_yCurrent);
        }
    }
}


void GameLogic::PauseGame()
{
    if (!(m_status == Status::Pause))
        m_OldTimerStatus = m_timer;
    if (m_status == Status::Pause)
        m_timer = false;

    m_status = Status::Pause;
}

void GameLogic::ResumeGame()
{
    if (m_status == Status::Play)
        m_timer = true; //fOldTimerStatus;

    m_status = Status::Play; // TODO
}

void GameLogic::UpdateBombCount(int BombAdjust)
{
    m_bombLeft += BombAdjust;
}

void GameLogic::SetButton(Button btn)
{
    m_button = btn;
}

Button GameLogic::GetButton()
{
    return m_button;
}

int GameLogic::GetTime()
{
    return m_seconds;
}

int GameLogic::GetBomb()
{
    return m_bombLeft;
}

size_t GameLogic::GetWidth()
{
    return m_width;
}

size_t GameLogic::GetHeight()
{
    return m_height;
}

Status GameLogic::GetStatus()
{
    return m_status;
}

char &GameLogic::Block(int x, int y)
{
    if(x >= m_width || y >= m_height || m_field.size() <= m_width * y + x)
        throw;
    return m_field.at(m_width * y + x);
}

// protected

int GameLogic::CountBombs(int xCenter, int yCenter)
{
    int result = 0;

    for (int y = yCenter - 1; y <= yCenter + 1; y++)
        for (int x = xCenter - 1; x <= xCenter + 1; x++)
            if (isInRange(x, y) && IsBomb(x, y))
                ++result;

    return result;
}

void GameLogic::ShowBombs(int iBlk) // char
{
    std::for_each(m_field.begin(), m_field.end(), [iBlk](auto &blk) {
        if (!isVisit(blk))
        {
            if (IsBomb(blk))
            {
                if (!GuessBomb(blk))
                    SetBlk(blk, iBlk);
            }
            else if (GuessBomb(blk))
            {
                SetBlk(blk, Block::Wrong);
            }
        }
    });
}

void GameLogic::GameOver(bool fWinLose)
{
    m_timer = false;
    m_button = fWinLose ? Button::Win : Button::Lose;

    ShowBombs(fWinLose ? Block::BombUp : Block::Bomb);

    if (fWinLose && m_bombLeft)
        UpdateBombCount(-m_bombLeft);

    // PlayTune(fWinLose ? TUNE_WINGAME : TUNE_LOSEGAME);
    m_status = Status::Demo;

    /*
        if (fWinLose && (Preferences.wGameType != wGameOther)
                && (cSec < Preferences.rgTime[Preferences.wGameType]))
                {
                Preferences.rgTime[Preferences.wGameType] = cSec;
                DoEnterName();
                DoDisplayBest();
                }
    */
}

void GameLogic::StepXY(int x, int y)
{
    if (!isInRange(x, y))
        return;

    int cBombs;
    auto &blk = Block(x, y);

    if ((blk & MaskVisit) || ((blk & MaskData) == Block::BombUp))
        return;

    ++m_blockVisit;

    blk = MaskVisit | (cBombs = CountBombs(x, y));

    if (cBombs != 0)
        return;

    rgStepX[iStepMac] = x;
    rgStepY[iStepMac] = y;

    if (++iStepMac == iStepMax)
        iStepMac = 0;
}

void GameLogic::StepBox(int x, int y)
{
    int iStepCur = 0;

    iStepMac = 1;

    StepXY(x, y);

    if (++iStepCur != iStepMac)

        while (iStepCur != iStepMac)
        {
            x = rgStepX[iStepCur];
            y = rgStepY[iStepCur];

            StepXY(x - 1, --y);
            StepXY(x, y);
            StepXY(x + 1, y);

            StepXY(x - 1, ++y);
            StepXY(x + 1, y);

            StepXY(x - 1, ++y);
            StepXY(x, y);
            StepXY(x + 1, y);

            if (++iStepCur == iStepMax)
                iStepCur = 0;
        }
}

void GameLogic::StepSquare(int x, int y)
{
    if (IsBomb(x, y))
    {
        if (m_blockVisit == 0)
        {
            auto block = std::find_if(m_field.begin(), m_field.end(), [](auto block) {
                return !IsBomb(block);
            });

            if (block != m_field.end())
            {
                Block(x, y) = Block::BlankUp; /* Move bomb out of way */
                SetBomb(*block);
                StepBox(x, y);
            }
        }
        else
        {
            Block(x, y) = MaskVisit | Block::Explode;
            GameOver(false);
        }
    }
    else
    {
        StepBox(x, y);

        if (m_blockVisit == m_blockVisitMax)
            GameOver(true);
    }
}

int GameLogic::CountMarks(int xCenter, int yCenter)
{
    int result = 0;

    for (int y = yCenter - 1; y <= yCenter + 1; y++)
        for (int x = xCenter - 1; x <= xCenter + 1; x++)
            if (isInRange(x, y) && GuessBomb(x, y))
                ++result;

    return result;
}

void GameLogic::StepBlock(int xCenter, int yCenter, bool block)
{
    bool fGameOver = false;

    if (!isInRange(xCenter, yCenter))
        return;

    if ((!isVisit(xCenter, yCenter)) || ((Block(xCenter, yCenter) & MaskData) != CountMarks(xCenter, yCenter)))
    {
        /* not a safe thing to do */
        TrackMouse(-2, -2, block); /* pop up the blocks */
        return;
    }

    for (int y = yCenter - 1; y <= yCenter + 1; y++)
        for (int x = xCenter - 1; x <= xCenter + 1; x++)
        {
            if (isInRange(x, y))
            {
                if (!GuessBomb(x, y) && IsBomb(x, y))
                {
                    fGameOver = true;
                    Block(x, y) = MaskVisit | Block::Explode;
                }
                else
                    StepBox(x, y);
            }
        }

    if (fGameOver)
        GameOver(false);
    else if (m_blockVisit == m_blockVisitMax)
        GameOver(true);
}

void GameLogic::PushBoxDown(int x, int y)
{
    auto iBlk = Block(x, y) & MaskData;

    if (iBlk == Block::GuessUp)
        iBlk = Block::Guess;
    else if (iBlk == Block::BlankUp)
        iBlk = Block::Blank;

    SetBlk(x, y, iBlk); // TODO
}

void GameLogic::PopBoxUp(int x, int y)
{
    auto iBlk = Block(x, y) & MaskData;

    if (iBlk == Block::Guess)
        iBlk = Block::GuessUp;
    else if (iBlk == Block::Blank)
        iBlk = Block::BlankUp;

    SetBlk(x, y, iBlk);
}

bool GameLogic::isInRange(int x, int y)
{
    return x >= 0 && y >= 0 && x < m_width && y < m_height;
}

void GameLogic::SetBomb(char &blk)
{
    blk |= MaskBomb;
}

void GameLogic::ClearBomb(int x, int y)
{
    Block(x, y) &= NOTMaskBomb;
}

bool GameLogic::IsBomb(char iBlk)
{
    return (iBlk & MaskBomb) != 0;
}

bool GameLogic::IsBomb(int x, int y)
{
    return (Block(x, y) & MaskBomb) != 0;
}

void GameLogic::SetVisit(int x, int y)
{
    Block(x, y) |= MaskVisit;
}

bool GameLogic::isVisit(char iBlk)
{
    return (iBlk & MaskVisit) != 0;
}

bool GameLogic::isVisit(int x, int y)
{
    return (Block(x, y) & MaskVisit) != 0;
}

bool GameLogic::GuessBomb(char iBlk)
{
    return (iBlk & MaskData) == Block::BombUp;
}

bool GameLogic::GuessBomb(int x, int y)
{
    return (Block(x, y) & MaskData) == Block::BombUp;
}

bool GameLogic::GuessMark(int x, int y)
{
    return (Block(x, y) & MaskData) == Block::GuessUp;
}

void GameLogic::SetBlk(char &blk, char iBlk)
{
    blk = (blk & MaskFlags) | iBlk;
}

void GameLogic::SetBlk(int x, int y, char iBlk)
{
    auto &blk = Block(x, y);
    blk = (blk & MaskFlags) | iBlk;
}

void GameLogic::ClearField()
{
    m_field.clear();
    m_field.resize(m_width * m_height, (char)Block::BlankUp);
}

bool operator==(Status lhs, Status rhs)
{
    return static_cast<unsigned>(lhs) == static_cast<unsigned>(rhs);
}