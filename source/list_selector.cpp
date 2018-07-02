#include "list_selector.hpp"

#include <cmath>

#include "gui.hpp"

ListSelector::ListSelector(std::string title, std::string options, std::vector<std::string> &listItems) : m_title(title), m_options(options), m_listItems(listItems) {
  m_optionsWidth = 0;
  m_optionsHeight = 0;
  selectedItem = 0;

  yOffsetNext = 0;
  yOffset = 0;

  startYOffset = 500;
  startYOffsetNext = 0;
}

ListSelector::~ListSelector() {

}

void ListSelector::draw(Gui *gui) {
  float deltaOffset = yOffsetNext - yOffset;
  float scrollSpeed = deltaOffset / 4.0F;
  float deltaOffsetStart = startYOffsetNext - startYOffset;
  float scrollSpeedStart = deltaOffsetStart / 4.0F;

  gui->drawRectangled(0, 0, Gui::g_framebuffer_width, 220 + startYOffset, gui->makeColor(0x00, 0x00, 0x00, 0x80 * (1 - (startYOffset / 500.0F))));
  gui->drawRectangle(0, 220 + startYOffset, Gui::g_framebuffer_width, Gui::g_framebuffer_height - 120, currTheme.backgroundColor);

  if (m_listItems.size() != 0)
    for (s8 currItem = -1; currItem < (s8) m_listItems.size(); currItem++)
        gui->drawRectangle(200, fmax(440 + 60 * currItem - yOffset, 220) + startYOffset + 45, Gui::g_framebuffer_width - 400, 1, currTheme.separatorColor);


  gui->drawRectangled(220, 305 + 60 * 2 + deltaOffset - 5 + startYOffset, Gui::g_framebuffer_width - 440, 71, currTheme.highlightColor);
  gui->drawRectangle(226, 305 + 60 * 2 + deltaOffset + startYOffset, Gui::g_framebuffer_width - 455, 61, currTheme.selectedButtonColor);
  gui->drawShadow(220, 305 + 60 * 2 + deltaOffset - 5 + startYOffset, Gui::g_framebuffer_width - 440, 71);

  yOffsetNext = 60 * selectedItem;
  if (m_listItems.size() != 0) {
    for (u8 currItem = 0; currItem < m_listItems.size(); currItem++)
      gui->drawText(font20, 270, fmax(440 + 60 * currItem - yOffset, 220) + startYOffset, currTheme.textColor, m_listItems[currItem].c_str());

    gui->drawRectangle(0, 220 + startYOffset, Gui::g_framebuffer_width, 80, currTheme.backgroundColor);
    gui->drawRectangle(0, Gui::g_framebuffer_height - 70 + startYOffset, Gui::g_framebuffer_width, 70, currTheme.backgroundColor);
    gui->drawRectangle((u32)((Gui::g_framebuffer_width - 1220) / 2), 300 + startYOffset, 1220, 1, currTheme.textColor);
    gui->drawText(font24, 100, 240 + startYOffset, currTheme.textColor, m_title.c_str());

  }
  else
    gui->drawText(font20, 300, 340 + 60 * 2 + startYOffset, currTheme.textColor, "No items present!");

  gui->drawRectangle((u32)((Gui::g_framebuffer_width - 1220) / 2), Gui::g_framebuffer_height - 73 + startYOffset, 1220, 1, currTheme.textColor);
  gui->drawTextAligned(font20, Gui::g_framebuffer_width - 100, Gui::g_framebuffer_height - 50 + startYOffset, currTheme.textColor, m_options.c_str(), ALIGNED_RIGHT);

  if (yOffset != yOffsetNext) {
    if (yOffsetNext > yOffset)
      yOffset += ceil((abs(deltaOffset) > scrollSpeed) ? scrollSpeed : deltaOffset);
    else
      yOffset += floor((abs(deltaOffset) > scrollSpeed) ? scrollSpeed : deltaOffset);
  }

  if (startYOffset != startYOffsetNext) {
    if (startYOffsetNext > startYOffset)
      startYOffset += ceil((abs(deltaOffsetStart) > scrollSpeedStart) ? scrollSpeedStart : deltaOffsetStart);
    else
      startYOffset += floor((abs(deltaOffsetStart) > scrollSpeedStart) ? scrollSpeedStart : deltaOffsetStart);
  }

  if(startYOffset == startYOffsetNext && startYOffset == 500)
    hide();

}

ListSelector* ListSelector::setInputAction(std::function<void(u32, u16)> inputActions) {
  m_inputActions = inputActions;

  return this;
}

void ListSelector::onInput(u32 kdown) {
 if (kdown & KEY_B)
   startYOffsetNext = 500;

 if (kdown & KEY_UP)
   if (selectedItem > 0)
     selectedItem--;

 if (kdown & KEY_DOWN)
   if (selectedItem < (static_cast<s16>(m_listItems.size() - 1)))
     selectedItem++;

  m_inputActions(kdown, selectedItem);

}

void ListSelector::onTouch(touchPosition &touch) {
  if(touch.px > 250 && touch.px < Gui::g_framebuffer_width - 250) {
    if(touch.py > 325 && touch.py < (325 + 60 * 5)) {
      s8 touchPos = ((touch.py - 325) / 60.0F); //325 + 60 * (currItem + 2)

      if((selectedItem + touchPos - 2) >= 0 && (selectedItem + touchPos - 2) <= (static_cast<s16>(m_listItems.size() - 1)))
        selectedItem += (touchPos - 2);
    }
  }
}

void ListSelector::show() {
  Gui::g_currListSelector = this;
}

void ListSelector::hide() {
  Gui::g_currListSelector = nullptr;
}
