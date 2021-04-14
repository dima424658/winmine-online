#include "dialogs.hpp"

#include <X11/Xlib.h>

int main(int argc, char *argv[])
{
  XInitThreads();

  auto app = Gtk::Application::create(argc, argv, "org.dima424658.minesweeper");

  MineGtk *pWindow = nullptr;

  Glib::RefPtr<Gtk::Builder> builder = Gtk::Builder::create_from_file("minesweeper.ui");

  builder->get_widget_derived("game_window", pWindow);
  if (!pWindow)
    std::cout << "Could not get 'game_window' from the builder." << std::endl;

  pWindow->set_icon_from_file("bmp/winmine.png");

  return app->run(*pWindow);
}