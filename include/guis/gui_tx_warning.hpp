#pragma once

#include "guis/gui.hpp"

#include <vector>
#include <unordered_map>
#include <stdbool.h>

class GuiTXWarning : public Gui {
public:
  GuiTXWarning();
  ~GuiTXWarning();

  void update();
  void draw();
  void onInput(u32 kdown);
  void onTouch(HidTouchState &touch);
  void onGesture(HidTouchScreenState startPosition, HidTouchScreenState endPosition, bool finish);
};
