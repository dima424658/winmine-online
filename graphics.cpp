#include "graphics.hpp"

using namespace Graphics;

DrawingArea::DrawingArea(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &builder, GameLogic &gameLogic) : Gtk::DrawingArea(cobject), r_gameLogic(gameLogic)
{
    add_events(Gdk::EventMask::BUTTON_PRESS_MASK);
    add_events(Gdk::EventMask::POINTER_MOTION_MASK);
    add_events(Gdk::EventMask::BUTTON_RELEASE_MASK);

    m_buttonImages = Cairo::ImageSurface::create_from_png("bmp/button.png");
    m_ledImages = Cairo::ImageSurface::create_from_png("bmp/led.png");
    m_blockImages = Cairo::ImageSurface::create_from_png("bmp/blocks.png");
}

bool DrawingArea::on_event(GdkEvent *event)
{
    auto buttonX = (get_width() - g_buttonSize) >> 1;
    auto buttonY = g_topLedY;

    if (event->motion.x >= buttonX && event->motion.x <= buttonX + g_buttonSize &&
        event->motion.y >= buttonY && event->motion.y <= buttonY + g_buttonSize)
    {
        if (event->type == GdkEventType::GDK_BUTTON_PRESS)
            r_gameLogic.SetButton(Button::Down);
        else if (event->type == GdkEventType::GDK_BUTTON_RELEASE)
        {
            r_gameLogic.StartGame();
            return false;
        }
    }

    static bool leftDown = false;
    static bool middleDown = false;

    if (r_gameLogic.GetStatus() == Status::Demo)
    {
        leftDown = middleDown = false;
        return false;
    }

    auto blkX = (static_cast<size_t>(event->motion.x - (g_gridOffsetX - g_blockSize)) >> 4) - 1;
    auto blkY = (static_cast<size_t>(event->motion.y - (g_gridOffsetY - g_blockSize)) >> 4) - 1;

    if (event->type == GdkEventType::GDK_MOTION_NOTIFY)
    {
        if (leftDown || middleDown)
            r_gameLogic.TrackMouse(blkX, blkY, middleDown);
    }
    else if (event->type == GdkEventType::GDK_BUTTON_PRESS)
    {

        if (event->button.button == 3)
            r_gameLogic.MakeGuess(blkX, blkY);

        if (event->button.button == 2)
        {
            r_gameLogic.TrackMouse(blkX, blkY, true);
            middleDown = true;
        }

        if (event->button.button == 1)
        {
            if (r_gameLogic.GetButton() != Button::Caution && r_gameLogic.GetButton() != Button::Down)
                r_gameLogic.SetButton(Button::Caution);

            r_gameLogic.TrackMouse(blkX, blkY, false);
            leftDown = true;
        }
    }
    else if (event->type == GdkEventType::GDK_BUTTON_RELEASE)
    {
        if (r_gameLogic.GetButton() == Button::Caution)
            r_gameLogic.SetButton(Button::Happy);

        if (event->button.button == 2)
        {
            r_gameLogic.DoButton1Up(true);
            middleDown = false;
        }

        if (event->button.button == 1)
        {
            r_gameLogic.DoButton1Up(false);
            leftDown = false;
        }
    }

    queue_draw();
    return false;
}

bool DrawingArea::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
    auto sizeX = r_gameLogic.GetWidth();
    auto sizeY = r_gameLogic.GetHeight();

    if (static_cast<size_t>(get_width()) != g_blockSize * sizeX + g_gridOffsetX + g_rightSpace ||
        static_cast<size_t>(get_height()) != g_blockSize * sizeY + g_gridOffsetY + g_bottomSpace)
    {
        Gdk::Geometry g;
        g.min_width = g.max_width = g_blockSize * sizeX + g_gridOffsetX + g_rightSpace;
        g.min_height = g.max_height = g_blockSize * sizeY + g_gridOffsetY + g_bottomSpace;
        get_parent_window()->set_geometry_hints(g, Gdk::WindowHints::HINT_MAX_SIZE | Gdk::WindowHints::HINT_MIN_SIZE);

        set_size_request(g_blockSize * sizeX + g_gridOffsetX + g_rightSpace, g_blockSize * sizeY + g_gridOffsetY + g_bottomSpace);
    }

    DrawBackground(cr);
    DrawButton(cr, r_gameLogic.GetButton());
    DrawTime(cr, r_gameLogic.GetTime());
    DrawBombCount(cr, r_gameLogic.GetBomb());

    for (size_t i = 0; i < sizeX; ++i)
        for (size_t j = 0; j < sizeY; ++j)
            DrawBlk(cr, i + 1, j + 1, (Block)r_gameLogic.Block(i, j));

    return true;
}

void DrawingArea::DrawBlk(const Cairo::RefPtr<Cairo::Context> &cr, int x, int y, Block id)
{
    int offset = (15 - static_cast<int>(id & 0b1111)) * g_blockSize;

    x = (x << 4) + (g_gridOffsetX - g_blockSize);
    y = (y << 4) + (g_gridOffsetY - g_blockSize);

    cr->set_source(m_blockImages, x, y - offset);
    cr->rectangle(x, y, g_blockSize, g_blockSize);
    cr->clip();
    cr->paint();
    cr->reset_clip();
}

void DrawingArea::DrawButton(const Cairo::RefPtr<Cairo::Context> &cr, Button id)
{
    double x = (get_width() - g_buttonSize) >> 1;
    double y = g_topLedY;

    int offset = static_cast<int>(id) * g_buttonSize;

    cr->set_source(m_buttonImages, x, y - offset);
    cr->rectangle(x, y, g_buttonSize, g_buttonSize);
    cr->clip();
    cr->paint();
    cr->reset_clip();
}

void DrawingArea::DrawBombCount(const Cairo::RefPtr<Cairo::Context> &cr, int count)
{
    if (count >= 0)
    {
        DrawLed(cr, g_leftBombX, count / 100);
        DrawLed(cr, g_leftBombX + g_ledSizeX, (count %= 100) / 10);
        DrawLed(cr, g_leftBombX + 2 * g_ledSizeX, count % 10);
    }
    else
    {
        count = -count;
        DrawLed(cr, g_leftBombX, -1);
        DrawLed(cr, g_leftBombX + g_ledSizeX, (count %= 100) / 10);
        DrawLed(cr, g_leftBombX + 2 * g_ledSizeX, count % 10);
    }
}

void DrawingArea::DrawTime(const Cairo::RefPtr<Cairo::Context> &cr, int time)
{
    auto dxWindow = get_width();

    DrawLed(cr, dxWindow - (g_rightTimeX + 3 * g_ledSizeX), time / 100);
    DrawLed(cr, dxWindow - (g_rightTimeX + 2 * g_ledSizeX), (time %= 100) / 10);
    DrawLed(cr, dxWindow - (g_rightTimeX + g_ledSizeX), time % 10);
}

void DrawingArea::DrawLed(const Cairo::RefPtr<Cairo::Context> &cr, int x, int id)
{
    int offset;

    if (id < 0)
        offset = 0;
    else if (id < 10)
        offset = g_ledSizeY * (11 - id);
    else
        offset = g_ledSizeY;

    cr->set_source(m_ledImages, x, g_topLedY - offset);
    cr->rectangle(x, g_topLedY, g_ledSizeX, g_ledSizeY);
    cr->clip();
    cr->paint();
    cr->reset_clip();
}

void DrawingArea::DrawBorder(const Cairo::RefPtr<Cairo::Context> &cr, double fromX, double fromY, double toX, double toY, int width, int normal)
{
    cr->set_line_width(1);

    fromX += 0.5;
    fromY += 0.5;
    toX -= 0.5;
    toY -= 0.5;

    for (int i = 0; i < width; ++i)
    {
        cr->move_to(fromX, --toY);
        cr->line_to(fromX++, fromY);
        cr->line_to(--toX, fromY++);
    }

    if (normal == 1 || normal == 2)
        cr->set_source_rgb(1.0, 1.0, 1.0);
    else
        cr->set_source_rgb(0.5, 0.5, 0.5);

    cr->stroke();

    for (int i = 0; i < width; ++i)
    {
        cr->move_to(fromX--, ++toY);
        cr->line_to(toX++, toY);
        cr->line_to(toX, fromY--);
    }

    if (normal % 2 == 0)
        cr->set_source_rgb(1.0, 1.0, 1.0);
    else
        cr->set_source_rgb(0.5, 0.5, 0.5);

    cr->stroke();
}

void DrawingArea::DrawBackground(const Cairo::RefPtr<Cairo::Context> &cr)
{
    auto x = get_width();
    auto y = get_height();

    auto dxWindow = get_width();
    auto dyWindow = get_height();

    cr->rectangle(0, 0, dxWindow, dyWindow);
    cr->set_source_rgb(0.753, 0.753, 0.753);
    cr->fill_preserve();
    cr->stroke();

    DrawBorder(cr, 0, 0, x, y, 3, 1);

    x -= (g_rightSpace - 3);
    y -= (g_bottomSpace - 3);
    DrawBorder(cr, g_gridOffsetX - 3, g_gridOffsetY - 3, x, y, 3, 0);
    DrawBorder(cr, g_gridOffsetX - 3, g_topSpace - 3, x, g_topLedY + g_ledSizeY + (g_bottomSpace - 6), 2, 0);

    x = g_leftBombX + g_ledSizeX * 3;
    y = g_topLedY + g_ledSizeY;
    DrawBorder(cr, g_leftBombX - 1, g_topLedY - 1, x, y, 1, 0);

    x = dxWindow - (g_rightTimeX + 3 * g_ledSizeX + 1);
    DrawBorder(cr, x, g_topLedY - 1, x + (g_ledSizeX * 3 + 1), y, 1, 0);

    x = ((dxWindow - g_buttonSize) >> 1) - 1;
    DrawBorder(cr, x, g_topLedY - 1, x + g_buttonSize + 1, g_topLedY + g_buttonSize, 1, 2);
}