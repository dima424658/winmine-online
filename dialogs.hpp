#pragma once

#include <gtkmm.h>

#include <iostream> // For std::cout

#include "graphics.hpp"
#include "rtns.hpp"
#include "multiplayer.hpp"

class MultiplayerGtk;

class CustomGtk : public Gtk::Window
{
public:
    CustomGtk(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &builder, Connection &gameLogic);
    ~CustomGtk() override = default;

    void CustomDialogOk();

    Glib::RefPtr<Gtk::Builder> m_builder;
    Connection &m_gameLogic;
};

class MineGtk : public Gtk::Window
{
public:
    MineGtk(BaseObjectType *cobject,
            const Glib::RefPtr<Gtk::Builder> &builder);
    ~MineGtk() override;

protected:
    void OnQuit();
    void OnAbout();
    void OnBestTimes();
    void ChangeDifficulty();

    void LoadSettings();
    void UpdateTimes();

    Glib::RefPtr<Gtk::Builder> m_builder;
    Graphics::DrawingArea *m_drawingArea;
    Connection m_gameLogic;
    CustomGtk *m_customDialog;
    MultiplayerGtk *m_multiplayerClientDialog;
    MultiplayerGtk *m_multiplayerServerDialog;
};

class MultiplayerGtk : public Gtk::Window
{
public:
    MultiplayerGtk(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &builder, Connection &gameLogic);
    ~MultiplayerGtk() override = default;

    void Connect();
    void CreateServer();

    Glib::RefPtr<Gtk::Builder> m_builder;
    Connection &m_gameLogic;
};