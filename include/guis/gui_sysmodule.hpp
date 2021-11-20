#pragma once

#include "guis/gui.hpp"

#include <vector>
#include <string>
#include <unordered_map>
#include <set>

typedef struct {
  std::string name;
  std::string titleID;
  bool requiresReboot;
} sysModule_t;

class GuiSysmodule : public Gui {
public:
  GuiSysmodule();
  ~GuiSysmodule();

  void update();
  void draw();
  void onInput(u32 kdown);
  void onTouch(HidTouchState &touch);
  void onGesture(HidTouchScreenState startPosition, HidTouchScreenState endPosition, bool finish);

private:
  static inline int selection = 0;
  std::unordered_map<std::string, sysModule_t> m_sysmodules;
  std::set<std::string> m_runningSysmodules;
  std::set<std::string> m_presentSysmodules;
};
