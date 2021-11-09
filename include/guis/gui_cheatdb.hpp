#pragma once

#include "guis/gui.hpp"

#include <string>

class Guicheatdb : public Gui {
public:
  Guicheatdb();
  ~Guicheatdb();

  void update();
  void draw();
  void onInput(u32 kdown);
  void onTouch(HidTouchState &touch);
  void onGesture(HidTouchScreenState startPosition, HidTouchScreenState endPosition, bool finish);
};
