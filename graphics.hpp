#pragma once

#include <cairomm/cairomm.h>
#include <gtkmm.h>

#include "rtns.hpp"

namespace Graphics
{
    constexpr int g_topSpace = 12;
    constexpr int g_bottomSpace = 12;
    constexpr int g_leftSpace = 12;
    constexpr int g_rightSpace = 12;

    constexpr int g_buttonSize = 24;
    constexpr int g_blockSize = 16;

    constexpr int g_ledSizeX = 13;
    constexpr int g_ledSizeY = 23;

    constexpr int g_topLedY = g_topSpace + 4;

    constexpr int g_gridOffsetX = g_leftSpace;
    constexpr int g_gridOffsetY = (g_topLedY + g_ledSizeY + 16);

    constexpr int g_leftBombX = g_leftSpace + 5;
    constexpr int g_rightTimeX = g_rightSpace + 5;

    class DrawingArea : public Gtk::DrawingArea
    {
    public:
        DrawingArea(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &builder, GameLogic &gameLogic);

    protected:
        virtual bool on_event(GdkEvent *gdk_event) override;
        virtual bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) override;

        void DrawBackground(const Cairo::RefPtr<Cairo::Context> &cr);
        void DrawBlk(const Cairo::RefPtr<Cairo::Context> &cr, int x, int y, Block id);
        void DrawButton(const Cairo::RefPtr<Cairo::Context> &cr, Button id);
        void DrawBombCount(const Cairo::RefPtr<Cairo::Context> &cr, int count);
        void DrawTime(const Cairo::RefPtr<Cairo::Context> &cr, int time);

        void DrawLed(const Cairo::RefPtr<Cairo::Context> &cr, int x, int id);
        void DrawBorder(const Cairo::RefPtr<Cairo::Context> &cr, double fromX, double fromY, double toX, double toY, int width, int normal);

    protected:
        Cairo::RefPtr<Cairo::ImageSurface> m_buttonImages;
        Cairo::RefPtr<Cairo::ImageSurface> m_ledImages;
        Cairo::RefPtr<Cairo::ImageSurface> m_blockImages;

        GameLogic &r_gameLogic;
    };
} // namespace Graphics