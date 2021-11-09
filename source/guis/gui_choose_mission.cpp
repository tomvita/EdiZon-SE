#include "guis/gui_choose_mission.hpp"
#include "helpers/config.hpp"

GuiChooseMission::GuiChooseMission() : Gui() {
  Config::getConfig()->option_once = false;
  // m_edizon_dir = Config::getConfig()->edizon_dir;
}
GuiChooseMission::~GuiChooseMission() {
}
void GuiChooseMission::update() {
  Gui::update();
}
static const char *const optionNames[] = {"\uE0A2 No Auto Attach", "\uE0A3 No Auto Exit after detach", "\uE0B4 Disable this screen"};
void GuiChooseMission::draw() {
  Gui::beginDraw();
  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, Gui::makeColor(0x00, 0x39, 0x29, 0xFF));
  Gui::drawTextAligned(fontHuge, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 - 100, COLOR_WHITE, "Welcome", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2, COLOR_WHITE, "Use L, R, ZL, ZR and B to choose storage directory for your search press A to continue", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2+60, COLOR_WHITE, "Use X, Y, - to toggle options, if you disable this screen use R+B to exit will show this on next launch", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 5, Gui::g_framebuffer_height / 2+200, Config::getConfig()->disablerangeonunknown ? COLOR_WHITE : COLOR_BLACK, "\uE0AF Disable Range on Unknown", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 5, Gui::g_framebuffer_height / 2 + 150, Config::getConfig()->separatebookmark ? COLOR_WHITE : COLOR_BLACK, "\uE0B1 Separate bookmark", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2+200, Config::getConfig()->enablecheats ? COLOR_WHITE : COLOR_BLACK, "\uE0B2 Enable cheats overwrite", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width *4 / 5, Gui::g_framebuffer_height / 2+200, COLOR_BLACK, "\uE0B0 Update cheats database", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 5, Gui::g_framebuffer_height / 2+250, Config::getConfig()->deletebookmark ? COLOR_WHITE : COLOR_BLACK, "\uE0C4 clear all bookmarks", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width * 4 / 5, Gui::g_framebuffer_height / 2 + 250, (Config::getConfig()->freeze || Config::getConfig()->enabletargetedscan || Config::getConfig()->easymode) ? COLOR_WHITE : COLOR_BLACK, "\uE0C5 More options", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 + 250, COLOR_WHITE, m_edizon_dir.c_str() , ALIGNED_CENTER);//"\uE070  Don't show this warning anymore"
  // for (u8 i = 0; i < 3; i++)
  // {
    // Gui::drawRectangled((Gui::g_framebuffer_width / 4) * (i + 1), Gui::g_framebuffer_height / 2 + 270, 300, 60, currTheme.separatorColor);
    // Gui::drawTextAligned(font20, (Gui::g_framebuffer_width / 5) , Gui::g_framebuffer_height / 2 + 300, Config::getConfig()->options[0] ? COLOR_WHITE : COLOR_BLACK, optionNames[0], ALIGNED_CENTER);
    // Gui::drawTextAligned(font20, (Gui::g_framebuffer_width / 2) , Gui::g_framebuffer_height / 2 + 300, Config::getConfig()->options[1] ? COLOR_WHITE : COLOR_BLACK, optionNames[1], ALIGNED_CENTER);
  Gui::drawTextAligned(font20, (Gui::g_framebuffer_width / 5), Gui::g_framebuffer_height / 2 + 300, Config::getConfig()->extra_value ? COLOR_WHITE : COLOR_BLACK, "\uE0A2 use extra search value", ALIGNED_CENTER);
  // Gui::drawTextAligned(font20, (Gui::g_framebuffer_width / 2), Gui::g_framebuffer_height / 2 + 300, Config::getConfig()->show_previous_values ? COLOR_WHITE : COLOR_BLACK, "\uE0A3 show previous values", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, (Gui::g_framebuffer_width / 2), Gui::g_framebuffer_height / 2 + 300, COLOR_WHITE, "\uE0A3 manage sysmodules", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, (Gui::g_framebuffer_width / 5) * 4, Gui::g_framebuffer_height / 2 + 300, Config::getConfig()->options[2] ? COLOR_WHITE : COLOR_BLACK, optionNames[2], ALIGNED_CENTER);
  // }
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width * 4 / 5, Gui::g_framebuffer_height / 2 + 150, COLOR_WHITE, "About \uE0B3", ALIGNED_CENTER);
  Gui::endDraw();
}
// u32 kheld = hidKeysHeld(HidNpadIdType_No1) | hidKeysHeld(HidNpadIdType_Handheld);
void GuiChooseMission::onInput(u32 kdown)
{
  if (kdown & HidNpadButton_L)
  {
    m_edizon_dir = "/switch/EdiZon/1";
  }
  else if (kdown & HidNpadButton_R)
  {
    m_edizon_dir = "/switch/EdiZon/2";
  }
    if (kdown & HidNpadButton_ZL)
  {
    m_edizon_dir = "/switch/EdiZon/3";
  }
  else if (kdown & HidNpadButton_ZR)
  {
    m_edizon_dir = "/switch/EdiZon/4";
  }
  else if (kdown & HidNpadButton_B)
  {
    m_edizon_dir = "/switch/EdiZon";
  }
  else if (kdown & HidNpadButton_X)
  {
    // Config::getConfig()->options[0] = !Config::getConfig()->options[0];
    Config::getConfig()->extra_value = !Config::getConfig()->extra_value;
  }
  else if (kdown & HidNpadButton_Y)
  {
    // Config::getConfig()->options[1] = !Config::getConfig()->options[1];
    Gui::g_nextGui = GUI_Sysmodule;
    // Config::getConfig()->show_previous_values = !Config::getConfig()->show_previous_values;
  }
  else if (kdown & HidNpadButton_Minus)
  {
    Config::getConfig()->options[2] = !Config::getConfig()->options[2];
  }
  else if (kdown & HidNpadButton_Up)
  {
    Config::getConfig()->disablerangeonunknown = !Config::getConfig()->disablerangeonunknown;
  }
  else if (kdown & HidNpadButton_Right)
  {
    Config::getConfig()->enablecheats = !Config::getConfig()->enablecheats;
  }
  else if (kdown & HidNpadButton_Left)
  {
    Config::getConfig()->separatebookmark = !Config::getConfig()->separatebookmark;
  }
    else if (kdown & HidNpadButton_Down)
  {
    Gui::g_nextGui = GUI_CHEATDB;
  }
  else if (kdown & HidNpadButton_Plus)
  {
    Gui::g_nextGui = GUI_ABOUT;
  }
  else if (kdown & HidNpadButton_StickL) 
  {
    Config::getConfig()->deletebookmark = !Config::getConfig()->deletebookmark;
  }
  else if (kdown & HidNpadButton_StickR) 
  {
    Gui::g_nextGui = GUI_MORE;
    // Config::getConfig()->freeze = !Config::getConfig()->freeze;
  }
  else if (kdown & HidNpadButton_A)
  {
    Gui::g_nextGui = GUI_CHEATS;
    memcpy(Config::getConfig()->edizon_dir, m_edizon_dir.c_str(), m_edizon_dir.size());
    Config::getConfig()->edizon_dir[m_edizon_dir.size()] = 0;
    Config::writeConfig();
  }
}

void GuiChooseMission::onTouch(HidTouchState &touch) {
  if (touch.x > 400 && touch.x < 900 && touch.y > 600 && touch.y < 660) {
    Config::getConfig()->hideSX = !Config::getConfig()->hideSX;
    Config::writeConfig();
  }
}

void GuiChooseMission::onGesture(HidTouchScreenState startPosition, HidTouchScreenState endPosition, bool finish) {

}
