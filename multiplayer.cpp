#include "multiplayer.hpp"

Connection::Connection(Gtk::Window *mainWindow, int x, int y, int bombCount)
    : GameLogic(mainWindow, x, y, bombCount), m_socket{-1} {}

void Connection::StartGame(int x, int y, int bombCount)
{
    std::scoped_lock lock(m_mutex);

    m_width = x;
    m_height = y;
    m_bombStart = bombCount;

    StartGame();
}

void Connection::StartGame()
{
    if (IsConnected())
    {
        SendPref();
        SendGameStart();
    }

    std::scoped_lock lock(m_mutex);

    m_timer = false;

    ClearField();
    m_button = Button::Happy;

    m_bombLeft = 0;

    std::random_device device;
    std::default_random_engine engine{device()};
    std::uniform_int_distribution<size_t> distWidth{0, static_cast<size_t>(m_width - 1)};
    std::uniform_int_distribution<size_t> distHeight{0, static_cast<size_t>(m_height - 1)};

    while (m_bombLeft != m_bombStart)
    {
        auto x = distWidth(engine);
        auto y = distHeight(engine);

        auto &blk = Block(x, y);
        if (!IsBomb(blk))
        {
            SetBomb(blk);

            if (IsConnected())
                SendSetBomb(x, y);

            ++m_bombLeft;
        }
    }

    m_seconds = 0;
    m_blockVisit = 0;
    m_blockVisitMax = (m_width * m_height) - m_bombLeft;
    m_status = Status::Play;

    UpdateBombCount(0);
}

void Connection::RecieveStartGame()
{
    std::scoped_lock lock(m_mutex);

    m_timer = false;

    ClearField();
    m_button = Button::Happy;

    m_bombLeft = m_bombStart;

    m_seconds = 0;
    m_blockVisit = 0;
    m_blockVisitMax = (m_width * m_height) - m_bombLeft;
    m_status = Status::Play;

    UpdateBombCount(0);
}

bool Connection::DoTimer()
{
    std::scoped_lock lock(m_mutex);

    if (m_timer && (m_seconds < 999))
        ++m_seconds;

    r_mainWindow->get_action_group("game")->activate_action("update_graphics");

    return m_timer;
}

void Connection::TrackMouse(int xNew, int yNew, bool block)
{
    if (IsConnected())
        SendTrackMouse(block, m_xCurrent, m_yCurrent, xNew, yNew);

    std::scoped_lock lock(m_mutex);

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

void Connection::TrackMouseRec(int xOld, int yOld, int xNew, int yNew, int block)
{
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

void Connection::MakeGuess(int x, int y)
{
    std::scoped_lock lock(m_mutex);
    if (IsConnected())
        SendMakeGuess(x, y);

    MakeGuessRec(x, y);
}

void Connection::MakeGuessRec(int x, int y)
{
    std::scoped_lock lock(m_mutex);

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

void Connection::DoButton1Up(bool block)
{
    if (IsConnected())
        SendDoButton(block, m_xCurrent, m_yCurrent);

    std::scoped_lock lock(m_mutex);

    if (isInRange(m_xCurrent, m_yCurrent))
    {

        if ((m_blockVisit == 0) && (m_seconds == 0))
        {
            m_timer = true;
            m_seconds = 0;

            Glib::signal_timeout().connect(sigc::mem_fun(*this, &Connection::DoTimer), 1000);
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

void Connection::DoButton1UpRec(int block, int x, int y)
{
    if (isInRange(x, y))
    {

        if ((m_blockVisit == 0) && (m_seconds == 0))
        {
            m_timer = true;
            m_seconds = 0;

            Glib::signal_timeout().connect(sigc::mem_fun(*this, &Connection::DoTimer), 1000);
        }

        if (m_status != Status::Play)
            x = y = -2;
        else
        {
            if (block)
                StepBlock(x, y, block);
            else if (!(isVisit(x, y) || GuessBomb(x, y)))
                StepSquare(x, y);
        }
    }
}

void Connection::PauseGame()
{
    std::scoped_lock lock(m_mutex);

    if (!(m_status == Status::Pause))
        m_OldTimerStatus = m_timer;
    if (m_status == Status::Pause)
        m_timer = false;

    m_status = Status::Pause;
}

void Connection::ResumeGame()
{
    std::scoped_lock lock(m_mutex);

    if (m_status == Status::Play)
        m_timer = true; //fOldTimerStatus;

    m_status = Status::Play; // TODO
}

void Connection::UpdateBombCount(int BombAdjust)
{
    std::scoped_lock lock(m_mutex);

    m_bombLeft += BombAdjust;
}

void Connection::SetButton(Button btn)
{
    std::scoped_lock lock(m_mutex);

    m_button = btn;
}

Button Connection::GetButton()
{
    return m_button;
}

int Connection::GetTime()
{
    return m_seconds;
}

int Connection::GetBomb()
{
    return m_bombLeft;
}

size_t Connection::GetWidth()
{
    return m_width;
}

size_t Connection::GetHeight()
{
    return m_height;
}

Status Connection::GetStatus()
{
    return m_status;
}

char &Connection::Block(int x, int y)
{
    return m_field.at(m_width * y + x);
}

void Connection::SendPref()
{
    buffer_t buffer;
    buffer.msgType = message_t::SetPref;
    buffer.pref = {
        .wGameType = 0,
        .Mines = m_bombStart,
        .Height = m_height,
        .Width = m_width};

    send(m_socket, buffer, sizeof(buffer_t), 0);
}

void Connection::SendGameStart()
{
    buffer_t buffer;
    buffer.msgType = message_t::GameStart;

    send(m_socket, buffer, sizeof(buffer_t), 0);
}

void Connection::SendMakeGuess(int x, int y)
{
    buffer_t buffer;
    buffer.msgType = message_t::MakeGuess;
    buffer.args[0] = x;
    buffer.args[1] = y;

    send(m_socket, buffer, sizeof(buffer_t), 0);
}

void Connection::SendDoButton(int block, int x, int y)
{
    buffer_t buffer;
    buffer.msgType = message_t::DoButton;
    buffer.args[0] = block;
    buffer.args[1] = x;
    buffer.args[2] = y;

    send(m_socket, buffer, sizeof(buffer_t), 0);
}

void Connection::SendSetBomb(int x, int y)
{
    buffer_t buffer;
    buffer.msgType = message_t::SetBomb;
    buffer.args[0] = x;
    buffer.args[1] = y;

    send(m_socket, buffer, sizeof(buffer_t), 0);
}

void Connection::SendTrackMouse(int block, int xOld, int yOld, int xNew, int yNew)
{
    buffer_t buffer;
    buffer.msgType = message_t::TrackMouse;
    buffer.args[0] = block;
    buffer.args[1] = xOld;
    buffer.args[2] = yOld;
    buffer.args[3] = xNew;
    buffer.args[4] = yNew;

    send(m_socket, buffer, sizeof(buffer_t), 0);
}

void Connection::Recieve()
{
    buffer_t buffer;

    while (true)
    {
        recv(m_socket, buffer, sizeof(buffer), 0);
        std::scoped_lock lock(m_mutex);

        switch (buffer.msgType)
        {
        case message_t::SetPref:
        {
            m_bombStart = buffer.pref.Mines;
            m_height = buffer.pref.Height;
            m_width = buffer.pref.Width;
            //auto y = Preferences.yWindow;
            //Preferences = buffer.pref;
            //Preferences.xWindow = x;
            //Preferences.yWindow = y;
            break;
        }
        case message_t::GameStart:
        {
            RecieveStartGame();
            break;
        }
        case message_t::SetBomb:
        {
            SetBomb(Block(buffer.args[0], buffer.args[1]));
            break;
        }
        case message_t::MakeGuess:
        {
            MakeGuessRec(buffer.args[0], buffer.args[1]);
            break;
        }
        case message_t::DoButton:
        {
            DoButton1UpRec(buffer.args[0], buffer.args[1], buffer.args[2]);
            break;
        }
        case message_t::TrackMouse:
        {
            TrackMouseRec(buffer.args[0], buffer.args[1], buffer.args[2], buffer.args[3], buffer.args[4]);
            break;
        }
        default:
            break;
        }
    }
}

std::future<std::pair<bool, std::string>> Connection::CreateServer(unsigned short port)
{
    Disconnect();
    auto promise = std::make_shared<std::promise<std::pair<bool, std::string>>>();

    std::thread([this, port, promise, &client = m_socket]() {
        int listener;
        addrinfo *result = nullptr;
        addrinfo hints{
            .ai_flags = AI_PASSIVE,
            .ai_family = AF_INET,
            .ai_socktype = SOCK_STREAM,
            .ai_protocol = IPPROTO_TCP};

        if (getaddrinfo(nullptr, std::to_string(port).c_str(), &hints, &result) != 0)
        {
            promise->set_value(std::make_pair(false, "Failed to get address"));
            return;
        }

        listener = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (listener < 0)
        {
            promise->set_value(std::make_pair(false, "Failed to create socket."));
            return;
        }

        if (bind(listener, result->ai_addr, result->ai_addrlen) < 0)
        {
            promise->set_value(std::make_pair(false, "Failed to bind socket."));
            return;
        }

        freeaddrinfo(result);

        listen(listener, SOMAXCONN);

        client = accept(listener, nullptr, nullptr);
        if (client < 0)
        {
            promise->set_value(std::make_pair(false, "Failed to accept socket connection."));
            return;
        }

        if (close(listener) < 0)
        {
            promise->set_value(std::make_pair(false, "Failed to close listen connection."));
            return;
        }

        std::thread thread([this]() {
            this->Recieve();
        });
        thread.detach();

        promise->set_value(std::make_pair(true, "Connected successfully."));
    }).detach();

    return promise->get_future();
}

std::future<std::pair<bool, std::string>> Connection::Connect(std::string addr, unsigned short port)
{
    Disconnect();
    auto promise = std::make_shared<std::promise<std::pair<bool, std::string>>>();

    std::thread([this, &addr, port, promise, &client = m_socket]() {
        addrinfo *result = nullptr;
        addrinfo hints{
            .ai_flags = AI_PASSIVE,
            .ai_family = AF_INET,
            .ai_socktype = SOCK_STREAM,
            .ai_protocol = IPPROTO_TCP};

        if (getaddrinfo(addr.c_str(), std::to_string(port).c_str(), &hints, &result) < 0)
        {
            promise->set_value(std::make_pair(false, "Failed to get address"));
            return;
        }

        for (addrinfo *ptr = result; ptr != nullptr; ptr = ptr->ai_next)
        {
            client = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
            if (client < 0)
            {
                promise->set_value(std::make_pair(false, "Failed to create socket."));
                return;
            }

            if (connect(client, ptr->ai_addr, (int)ptr->ai_addrlen) < 0)
            {
                close(client);
                client = -1;
                continue;
            }
            break;
        }

        freeaddrinfo(result);

        if (client < 0)
            promise->set_value(std::make_pair(false, "Failed to connect."));
        else
        {
            std::thread thread([this]() {
                this->Recieve();
            });
            thread.detach();

            promise->set_value(std::make_pair(true, "Connected successfully."));
        }
    }).detach();

    return promise->get_future();
}