#include "guis/button2.hpp"

extern "C" {
  #include "theme.h"
}

Button2::Button2(u16 x, u16 y, u16 w, u16 h, std::function<void(Gui*, u16, u16, bool*)> drawAction, std::function<void(u32, bool*)> inputAction, std::vector<s32> adjacentButton, bool activatable, std::function<bool()> usableCondition)
 : m_x(x), m_y(y), m_w(w), m_h(h), m_drawAction(drawAction), m_inputAction(inputAction), m_usableCondition(usableCondition), m_activatable(activatable), m_adjacentButton(adjacentButton){
  Button2::g_buttons.push_back(this);

  m_isActivated = false;
  m_isSelected = false;

  select(0);
}

s16 Button2::getSelectedIndex() {
  for(size_t i=0; i != g_buttons.size(); ++i) {
    if (g_buttons[i]->m_isSelected)
      return i;
  }
  return -1;
}

void Button2::select(s16 buttonIndex) {
  if (buttonIndex < 0) return;
  if (Button2::g_buttons.size() <= static_cast<u16>(buttonIndex)) return;

  for(Button2 *btn : Button2::g_buttons) {
    btn->m_isSelected = false;
    btn->m_isActivated = false;
  }

  auto button = Button2::g_buttons[buttonIndex];
  button->m_isSelected = true;

  if (g_scrollBlocked)
    return;

  auto leftmostDiff = (button->m_x) - (g_pageOffsetX) - g_pageLeftmostBoundary;
  auto topmostDiff = (button->m_y) - (g_pageOffsetY) - g_pageTopmostBoundary;
  auto rightmostDiff = (button->m_x + button->m_w) - (g_pageOffsetX) - g_pageRightmostBoundary;
  auto bottommostDiff = (button->m_y + button->m_h) - (g_pageOffsetY) - g_pageBottommostBoundary;

  if (leftmostDiff < 0)
    g_pageOffsetX += leftmostDiff;

  if (rightmostDiff > 0)
    g_pageOffsetX += rightmostDiff;

  if (topmostDiff < 0)
    g_pageOffsetY += topmostDiff;

  if (bottommostDiff > 0)
    g_pageOffsetY += bottommostDiff;
}

void Button2::draw(Gui *gui) {
  // Offset calculation
  g_targetOffsetX = g_pageOffsetX;
  // g_targetOffsetY = g_pageOffsetY;
  s32 resultX = m_x - g_targetOffsetX;
  s32 resultY = m_y - g_targetOffsetY;
  if (resultX + m_w < 0 || resultY + m_h < 0 || resultX > SCREEN_WIDTH || resultY > SCREEN_HEIGHT)
    return;
  s32 borderX = resultX - 5;
  s32 borderY = resultY - 5;
  if(m_isSelected) {
    gui->drawRectangled(borderX, borderY, m_w + 10, m_h + 10, m_isActivated ? currTheme.activatedColor : currTheme.highlightColor);
    gui->drawShadow(borderX, borderY, m_w + 10, m_h + 10);
  } else gui->drawShadow(resultX, resultY, m_w, m_h);

  gui->drawRectangled(resultX, resultY, m_w, m_h, currTheme.selectedButtonColor);

  m_drawAction(gui, resultX, resultY, &m_isActivated);

  if (!m_usableCondition())
    gui->drawRectangled(resultX, resultY, m_w, m_h, gui->makeColor(0x80, 0x80, 0x80, 0x80));
}

bool Button2::onInput(u32 kdown) {
  if (!m_isSelected)
    return false;

  if (!m_isActivated) {
    if ((kdown & HidNpadButton_A) && m_activatable) {
      m_isActivated = true;
      kdown = 0;
    } else {
      if (kdown & HidNpadButton_AnyUp)    Button2::select(m_adjacentButton[0]);
      if (kdown & HidNpadButton_AnyDown)  Button2::select(m_adjacentButton[1]);
      if (kdown & HidNpadButton_AnyLeft)  Button2::select(m_adjacentButton[2]);
      if (kdown & HidNpadButton_AnyRight) Button2::select(m_adjacentButton[3]);
      if (kdown & (HidNpadButton_AnyUp | HidNpadButton_AnyDown | HidNpadButton_AnyLeft | HidNpadButton_AnyRight)) return true;
    }
  }

  if(!m_usableCondition()) return false;

  m_inputAction(kdown, &m_isActivated);

  return false;
}

void Button2::onTouch(HidTouchState &touch) {
  if (!m_usableCondition()) return;

  u16 resultX = m_x > g_targetOffsetX ? m_x - g_targetOffsetX : 0;
  u16 resultY = m_y > g_targetOffsetY ? m_y - g_targetOffsetY : 0;

  if (touch.x >= resultX && touch.x <= (resultX + m_w) && touch.y >= resultY && touch.y <= (resultY + m_h)) {
    if (m_isSelected) {
      if (m_activatable) m_isActivated = true;
      else m_inputAction(HidNpadButton_A, &m_isActivated);
      return;
    }

    for(size_t i=0; i != g_buttons.size(); ++i) {
      if (g_buttons[i] == this) {
        select(i);
        break;
      }
    }
  }
}
