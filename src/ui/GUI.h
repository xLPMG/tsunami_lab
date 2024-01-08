#ifndef TSUNAMI_LAB_UI_GUI
#define TSUNAMI_LAB_UI_GUI

namespace tsunami_lab
{
  namespace ui
  {
    class GUI;
  }
}

class tsunami_lab::ui::GUI
{
private:
  void setupFolders();

public:
  int launch();
};

#endif