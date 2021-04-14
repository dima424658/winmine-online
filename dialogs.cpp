#include "dialogs.hpp"

CustomGtk::CustomGtk(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &builder, Connection &gameLogic)
    : Gtk::Window{cobject}, m_builder{builder}, m_gameLogic{gameLogic}
{
    Glib::RefPtr<Gio::SimpleActionGroup> gameActions = Gio::SimpleActionGroup::create();
    gameActions->add_action("custom.ok", sigc::mem_fun(*this, &CustomGtk::CustomDialogOk));
    gameActions->add_action("custom.cancel", sigc::mem_fun(*this, &CustomGtk::hide));
    insert_action_group("game", gameActions);
}

void CustomGtk::CustomDialogOk()
{
    try
    {
        Gtk::Entry *pHeight, *pWidth, *pMines;
        m_builder->get_widget("game_custom_height", pHeight);
        m_builder->get_widget("game_custom_width", pWidth);
        m_builder->get_widget("game_custom_mines", pMines);

        auto height = std::stoi(pHeight->get_text());
        auto width = std::stoi(pWidth->get_text());
        auto mines = std::stoi(pMines->get_text());

        m_gameLogic.StartGame(width, height, mines);

        hide();
    }
    catch (const std::exception &e)
    {
        return;
    }
}

MineGtk::MineGtk(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &builder)
    : Gtk::Window(cobject), m_builder(builder), m_gameLogic{this}
{
    m_builder->get_widget_derived("game_draw", m_drawingArea, m_gameLogic);
    m_builder->get_widget_derived("game_custom_dialog", m_customDialog, m_gameLogic);
    m_builder->get_widget_derived("multiplayer_client", m_multiplayerClientDialog, m_gameLogic);
    m_builder->get_widget_derived("multiplayer_server", m_multiplayerServerDialog, m_gameLogic);

    {
        Glib::RefPtr<Gio::SimpleActionGroup> windowActions = Gio::SimpleActionGroup::create();
        windowActions->add_action("quit", sigc::mem_fun(*this, &MineGtk::OnQuit));
        windowActions->add_action("about", sigc::mem_fun(*this, &MineGtk::OnAbout));
        insert_action_group("win", windowActions);
    }
    {
        Glib::RefPtr<Gio::SimpleActionGroup> gameActions = Gio::SimpleActionGroup::create();
        gameActions->add_action("times", sigc::mem_fun(*this, &MineGtk::OnBestTimes));
        gameActions->add_action("new", sigc::mem_fun0(m_gameLogic, &Connection::StartGame));
        gameActions->add_action("update_graphics", sigc::mem_fun(*m_drawingArea, &Graphics::DrawingArea::queue_draw));
        gameActions->add_action("server_connect", sigc::mem_fun(*m_multiplayerClientDialog, &MultiplayerGtk::show_all));
        gameActions->add_action("server_create", sigc::mem_fun(*m_multiplayerServerDialog, &MultiplayerGtk::show_all));

        // gameActions->add_action("update_times", sigc::mem_fun(*this, &MineGtk::UpdateTimes));

        insert_action_group("game", gameActions);
    }
    {
        Gtk::CheckMenuItem *pDifficulty = nullptr;

        m_builder->get_widget("difficulty_beginner", pDifficulty);
        pDifficulty->signal_toggled().connect(sigc::mem_fun(*this, &MineGtk::ChangeDifficulty));
        pDifficulty->set_active();

        m_builder->get_widget("difficulty_intermediate", pDifficulty);
        pDifficulty->signal_toggled().connect(sigc::mem_fun(*this, &MineGtk::ChangeDifficulty));

        m_builder->get_widget("difficulty_expert", pDifficulty);
        pDifficulty->signal_toggled().connect(sigc::mem_fun(*this, &MineGtk::ChangeDifficulty));

        m_builder->get_widget("difficulty_custom", pDifficulty);
        pDifficulty->signal_toggled().connect(sigc::mem_fun(*this, &MineGtk::ChangeDifficulty));
    }

    show_all();
}

MineGtk::~MineGtk()
{
}

void MineGtk::OnQuit()
{
    hide();
}

void MineGtk::OnAbout()
{
    Gtk::AboutDialog *pDialog = nullptr;
    m_builder->get_widget("game_about", pDialog);
    if (pDialog)
    {
        pDialog->set_transient_for(*this);
        pDialog->run();
        pDialog->hide();
    }
}

void MineGtk::ChangeDifficulty()
{
    Gtk::CheckMenuItem *pDifficulty = nullptr;

    m_builder->get_widget("difficulty_beginner", pDifficulty);
    if (pDifficulty->get_active())
        m_gameLogic.StartGame(9, 9, 10);

    m_builder->get_widget("difficulty_intermediate", pDifficulty);
    if (pDifficulty->get_active())
        m_gameLogic.StartGame(16, 16, 40);

    m_builder->get_widget("difficulty_expert", pDifficulty);
    if (pDifficulty->get_active())
        m_gameLogic.StartGame(30, 16, 99);

    m_builder->get_widget("difficulty_custom", pDifficulty);
    if (pDifficulty->get_active())
        m_customDialog->show_all();

    m_drawingArea->queue_draw();
}

void MineGtk::OnBestTimes()
{
    Gtk::Dialog *pDialog = nullptr;
    Gtk::Button *pCloseBtn = nullptr;

    m_builder->get_widget("game_times_dialog", pDialog);
    m_builder->get_widget("game_times_dialog.ok", pCloseBtn);

    if (pCloseBtn && pDialog)
        pCloseBtn->signal_clicked().connect([&pDialog]() {
            pDialog->hide();
        });

    if (pDialog)
    {
        pDialog->set_transient_for(*this);
        pDialog->run();
        pDialog->hide();
    }
}

MultiplayerGtk::MultiplayerGtk(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &builder, Connection &gameLogic)
    : Gtk::Window{cobject}, m_builder{builder}, m_gameLogic{gameLogic}
{
    Glib::RefPtr<Gio::SimpleActionGroup> multiplayerActions = Gio::SimpleActionGroup::create();
    multiplayerActions->add_action("connect", sigc::mem_fun(*this, &MultiplayerGtk::Connect));
    multiplayerActions->add_action("server", sigc::mem_fun(*this, &MultiplayerGtk::CreateServer));
    //multiplayerActions->add_action("custom.cancel", sigc::mem_fun(*this, &CustomGtk::hide));
    insert_action_group("multiplayer", multiplayerActions);
}

void MultiplayerGtk::Connect()
{
    std::thread([&]() {
        Gtk::Label *pLabelStatus;
        Gtk::Button *pConnectBtn;
        Gtk::Spinner *pSpinner;
        Gtk::Entry *pIp, *pPort;

        m_builder->get_widget("multiplayer_client_status", pLabelStatus);
        m_builder->get_widget("multiplayer_client_connect", pConnectBtn);
        m_builder->get_widget("multiplayer_client_spinner", pSpinner);
        m_builder->get_widget("multiplayer_client_ip", pIp);
        m_builder->get_widget("multiplayer_client_port", pPort);

        pConnectBtn->set_sensitive(false);
        pLabelStatus->set_text("Connecting...");
        pSpinner->property_active() = true;

        try
        {
            auto port = std::stoul(pPort->get_text());
            auto [result, info] = m_gameLogic.Connect(pIp->get_text(), port).get();
            pLabelStatus->set_text(info);

            if (result)
                hide();
        }
        catch (std::exception e)
        {
            pLabelStatus->set_text("Incorrect port");
        }

        pSpinner->property_active() = false;
        pConnectBtn->set_sensitive(true);
    }).detach();
}

void MultiplayerGtk::CreateServer()
{
    std::thread([&]() {
        Gtk::Label *pLabelStatus;
        Gtk::Button *pCreateBtn;
        Gtk::Spinner *pSpinner;
        Gtk::Entry *pPort;

        m_builder->get_widget("multiplayer_server_status", pLabelStatus);
        m_builder->get_widget("multiplayer_server_create", pCreateBtn);
        m_builder->get_widget("multiplayer_server_spinner", pSpinner);
        m_builder->get_widget("multiplayer_server_port", pPort);

        pCreateBtn->set_sensitive(false);
        pLabelStatus->set_text("Waiting for connection...");
        pSpinner->property_active() = true;

        try
        {
            auto port = std::stoul(pPort->get_text());
            auto [result, info] = m_gameLogic.CreateServer(port).get();
            pLabelStatus->set_text(info);

            if (result)
                hide();
        }
        catch (std::exception e)
        {
            pLabelStatus->set_text("Incorrect port");
        }

        pSpinner->property_active() = false;
        pCreateBtn->set_sensitive(true);
    }).detach();
}
