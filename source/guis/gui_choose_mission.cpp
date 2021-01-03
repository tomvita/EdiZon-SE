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
static const char *const optionNames[] = {"\uE0A2 没有自动附加", "\uE0A3 分离后没有自动退出", "\uE0B4 禁用此屏幕"};
void GuiChooseMission::draw() {
  Gui::beginDraw();
  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, Gui::makeColor(0x00, 0x39, 0x29, 0xFF));
  Gui::drawTextAligned(fontHuge, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 - 100, COLOR_WHITE, "欢迎", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2, COLOR_WHITE, "使用L，R，ZL，ZR和B来选择要搜索的存储目录，请按A继续", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2+60, COLOR_WHITE, "使用X，Y，-来切换选项，如果禁用此屏幕，请使用R + B退出，将在下次启动时显示", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 5, Gui::g_framebuffer_height / 2+200, Config::getConfig()->disablerangeonunknown ? COLOR_WHITE : COLOR_BLACK, "\uE0AF 禁用未知范围", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 5, Gui::g_framebuffer_height / 2 + 150, Config::getConfig()->separatebookmark ? COLOR_WHITE : COLOR_BLACK, "\uE0B1 单独的书签", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2+200, Config::getConfig()->enablecheats ? COLOR_WHITE : COLOR_BLACK, "\uE0B2 启用金手指功能覆写", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width *4 / 5, Gui::g_framebuffer_height / 2+200, COLOR_BLACK, "\uE0B0 更新金手指数据库", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 5, Gui::g_framebuffer_height / 2+250, Config::getConfig()->deletebookmark ? COLOR_WHITE : COLOR_BLACK, "\uE0C4 清除所有书签", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width * 4 / 5, Gui::g_framebuffer_height / 2 + 250, (Config::getConfig()->freeze || Config::getConfig()->enabletargetedscan || Config::getConfig()->easymode) ? COLOR_WHITE : COLOR_BLACK, "\uE0C5 更多选项", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 + 250, COLOR_WHITE, m_edizon_dir.c_str() , ALIGNED_CENTER);//"\uE070  Don't show this warning anymore"
  // for (u8 i = 0; i < 3; i++)
  // {
    // Gui::drawRectangled((Gui::g_framebuffer_width / 4) * (i + 1), Gui::g_framebuffer_height / 2 + 270, 300, 60, currTheme.separatorColor);
    // Gui::drawTextAligned(font20, (Gui::g_framebuffer_width / 5) , Gui::g_framebuffer_height / 2 + 300, Config::getConfig()->options[0] ? COLOR_WHITE : COLOR_BLACK, optionNames[0], ALIGNED_CENTER);
    // Gui::drawTextAligned(font20, (Gui::g_framebuffer_width / 2) , Gui::g_framebuffer_height / 2 + 300, Config::getConfig()->options[1] ? COLOR_WHITE : COLOR_BLACK, optionNames[1], ALIGNED_CENTER);
  Gui::drawTextAligned(font20, (Gui::g_framebuffer_width / 5), Gui::g_framebuffer_height / 2 + 300, Config::getConfig()->extra_value ? COLOR_WHITE : COLOR_BLACK, "\uE0A2 使用额外的搜索值", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, (Gui::g_framebuffer_width / 2), Gui::g_framebuffer_height / 2 + 300, Config::getConfig()->show_previous_values ? COLOR_WHITE : COLOR_BLACK, "\uE0A3 显示之前的值", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, (Gui::g_framebuffer_width / 5) * 4, Gui::g_framebuffer_height / 2 + 300, Config::getConfig()->options[2] ? COLOR_WHITE : COLOR_BLACK, optionNames[2], ALIGNED_CENTER);
  // }
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width * 4 / 5, Gui::g_framebuffer_height / 2 + 150, COLOR_WHITE, "关于 \uE0B3", ALIGNED_CENTER);
  Gui::endDraw();
}
// u32 kheld = hidKeysHeld(CONTROLLER_PLAYER_1) | hidKeysHeld(CONTROLLER_HANDHELD);
void GuiChooseMission::onInput(u32 kdown)
{
  if (kdown & KEY_L)
  {
    m_edizon_dir = "/switch/EdiZon/1";
  }
  else if (kdown & KEY_R)
  {
    m_edizon_dir = "/switch/EdiZon/2";
  }
    if (kdown & KEY_ZL)
  {
    m_edizon_dir = "/switch/EdiZon/3";
  }
  else if (kdown & KEY_ZR)
  {
    m_edizon_dir = "/switch/EdiZon/4";
  }
  else if (kdown & KEY_B)
  {
    m_edizon_dir = "/switch/EdiZon";
  }
  else if (kdown & KEY_X)
  {
    // Config::getConfig()->options[0] = !Config::getConfig()->options[0];
    Config::getConfig()->extra_value = !Config::getConfig()->extra_value;
  }
  else if (kdown & KEY_Y)
  {
    // Config::getConfig()->options[1] = !Config::getConfig()->options[1];
    Config::getConfig()->show_previous_values = !Config::getConfig()->show_previous_values;
  }
  else if (kdown & KEY_MINUS)
  {
    Config::getConfig()->options[2] = !Config::getConfig()->options[2];
  }
  else if (kdown & KEY_DUP)
  {
    Config::getConfig()->disablerangeonunknown = !Config::getConfig()->disablerangeonunknown;
  }
  else if (kdown & KEY_DRIGHT)
  {
    Config::getConfig()->enablecheats = !Config::getConfig()->enablecheats;
  }
  else if (kdown & KEY_DLEFT)
  {
    Config::getConfig()->separatebookmark = !Config::getConfig()->separatebookmark;
  }
    else if (kdown & KEY_DDOWN)
  {
    Gui::g_nextGui = GUI_CHEATDB;
  }
  else if (kdown & KEY_PLUS)
  {
    Gui::g_nextGui = GUI_ABOUT;
  }
  else if (kdown & KEY_LSTICK) 
  {
    Config::getConfig()->deletebookmark = !Config::getConfig()->deletebookmark;
  }
  else if (kdown & KEY_RSTICK) 
  {
    Gui::g_nextGui = GUI_MORE;
    // Config::getConfig()->freeze = !Config::getConfig()->freeze;
  }
  else if (kdown & KEY_A)
  {
    Gui::g_nextGui = GUI_CHEATS;
    memcpy(Config::getConfig()->edizon_dir, m_edizon_dir.c_str(), m_edizon_dir.size());
    Config::getConfig()->edizon_dir[m_edizon_dir.size()] = 0;
    Config::writeConfig();
  }
}

void GuiChooseMission::onTouch(touchPosition &touch) {
  if (touch.px > 400 && touch.px < 900 && touch.py > 600 && touch.py < 660) {
    Config::getConfig()->hideSX = !Config::getConfig()->hideSX;
    Config::writeConfig();
  }
}

void GuiChooseMission::onGesture(touchPosition startPosition, touchPosition endPosition, bool finish) {

}
