#pragma once

#include <switch.h>

#include <functional>
#include <vector>

#include "gui.hpp"
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define fontIcons 5

class Button2 {
public:
  static inline std::vector<Button2*> g_buttons;
  static inline u16 g_pageOffsetX = 0;
  static inline u16 g_pageOffsetY = 0;
  static inline u16 g_targetOffsetX = 0;
  static inline u16 g_targetOffsetY = 0;
  static inline u16 g_pageLeftmostBoundary = 100;
  static inline u16 g_pageTopmostBoundary = 160;
  static inline u16 g_pageRightmostBoundary = SCREEN_WIDTH - 100;
  static inline u16 g_pageBottommostBoundary = SCREEN_HEIGHT - 160;
  static inline bool g_scrollBlocked = false;

  Button2(u16 x, u16 y, u16 w, u16 h, std::function<void(Gui*, u16, u16, bool*)> drawAction, std::function<void(u32, bool*)> inputAction, std::vector<s32> adjacentButton, bool activatable, std::function<bool()> usableCondition);

  void draw(Gui *gui);
  bool onInput(u32 kdown);
  void onTouch(HidTouchState &touch);

  inline bool isActivated() {
    return m_isActivated;
  }

  inline bool isSelected() {
    return m_isSelected;
  }

  static s16 getSelectedIndex();

  static void select(s16 buttonIndex);

private:

  u16 m_x, m_y, m_w, m_h;

  std::function<void(Gui*, u16, u16, bool*)> m_drawAction;
  std::function<void(u32, bool*)> m_inputAction;
  std::function<bool()> m_usableCondition;

  bool m_isActivated;
  bool m_isSelected;
  bool m_activatable;

  std::vector<s32> m_adjacentButton;
};
