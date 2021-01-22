#include "guis/gui_more.hpp"
#include "helpers/config.hpp"
GuiMore::GuiMore() : Gui() {
  Config::getConfig()->option_once = false;
  // m_edizon_dir = Config::getConfig()->edizon_dir;
}
GuiMore::~GuiMore() {
}
void GuiMore::update() {
  Gui::update();
}
static const char *const optionNames[] = {"没有自动附加 \uE0A2", "分离后没有自动退出 \uE0A3", "禁用此屏幕 \uE0B4"};
bool screen2 = false;
void draw2() {

}
void GuiMore::draw() {
  if (screen2)
  {
    draw2();
    return;
  }
  std::stringstream extra_seg_str;
  extra_seg_str << "\uE0B2 +  \uE0B1 -  额外MB " << Config::getConfig()->extraMB;
  Gui::beginDraw();
  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, Gui::makeColor(0x30, 0x39, 0x29, 0xFF));
  Gui::drawTextAligned(fontHuge, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 - 100, COLOR_WHITE, "更多选项", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2, COLOR_BLACK, "使用L，R，ZL，ZR和B键来选择要搜索的存储目录，请按A继续", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2+60, COLOR_BLACK, "使用X，Y和-键来切换选项，如果禁用此屏幕，请使用R + B退出，将在下次启动时显示", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 5, Gui::g_framebuffer_height / 2 + 200, Config::getConfig()->enabletargetedscan ? COLOR_WHITE : COLOR_BLACK, "\uE0AF 启用目标扫描", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 + 200, Config::getConfig()->enabletargetedscan ? COLOR_WHITE : COLOR_BLACK, extra_seg_str.str().c_str(), ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width *4 / 5, Gui::g_framebuffer_height / 2 + 200, Config::getConfig()->use_absolute_address ? COLOR_WHITE : COLOR_BLACK, "\uE0B0 使用绝对地址", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 5, Gui::g_framebuffer_height / 2 + 250, Config::getConfig()->easymode ? COLOR_WHITE : COLOR_BLACK, "\uE0C4 简易模式", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width * 4 / 5, Gui::g_framebuffer_height / 2+250, Config::getConfig()->freeze ? COLOR_WHITE : COLOR_BLACK, "\uE0C5 冻结游戏", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 + 250, COLOR_WHITE, m_edizon_dir.c_str() , ALIGNED_CENTER);//"\uE070  Don't show this warning anymore"
  // for (u8 i = 0; i < 3; i++)
  // {
  //   // Gui::drawRectangled((Gui::g_framebuffer_width / 4) * (i + 1), Gui::g_framebuffer_height / 2 + 270, 300, 60, currTheme.separatorColor);
  //   Gui::drawTextAligned(font20, (Gui::g_framebuffer_width / 4) * (i + 1), Gui::g_framebuffer_height / 2 + 300, Config::getConfig()->options[i] ? COLOR_WHITE : COLOR_BLACK, optionNames[i], ALIGNED_CENTER);
  // }
  // Gui::drawTextAligned(font20, (Gui::g_framebuffer_width / 5), Gui::g_framebuffer_height / 2 + 300, Config::getConfig()->extra_value ? COLOR_WHITE : COLOR_BLACK, "\uE0A2 use extra search value", ALIGNED_CENTER);
  // Gui::drawTextAligned(font20, (Gui::g_framebuffer_width / 2), Gui::g_framebuffer_height / 2 + 300, Config::getConfig()->show_previous_values ? COLOR_WHITE : COLOR_BLACK, "\uE0A3 show previous values", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, (Gui::g_framebuffer_width / 5), Gui::g_framebuffer_height / 2 + 300, Config::getConfig()->options[0] ? COLOR_WHITE : COLOR_BLACK, optionNames[0], ALIGNED_CENTER);
  Gui::drawTextAligned(font20, (Gui::g_framebuffer_width / 2), Gui::g_framebuffer_height / 2 + 300, Config::getConfig()->options[1] ? COLOR_WHITE : COLOR_BLACK, optionNames[1], ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width * 4 / 5, Gui::g_framebuffer_height / 2 + 300, Config::getConfig()->exclude_ptr_candidates ? COLOR_WHITE : COLOR_BLACK, "\uE0B4 排除候选指针地址", ALIGNED_CENTER);

  Gui::drawTextAligned(font20, Gui::g_framebuffer_width * 4 / 5, Gui::g_framebuffer_height / 2 + 150, COLOR_WHITE, "关于 \uE0B3", ALIGNED_CENTER);
  Gui::endDraw();
}
// u32 kheld = hidKeysHeld(CONTROLLER_PLAYER_1) | hidKeysHeld(CONTROLLER_HANDHELD);
void onInput2(u32 kdown)
{

}
void GuiMore::onInput(u32 kdown)
{
  if (screen2)
  {
    onInput2(kdown);
    return;
  }
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
    Config::getConfig()->options[0] = !Config::getConfig()->options[0];
    // Config::getConfig()->extra_value = !Config::getConfig()->extra_value;
  }
  else if (kdown & KEY_Y)
  {
    Config::getConfig()->options[1] = !Config::getConfig()->options[1];
    // Config::getConfig()->show_previous_values = !Config::getConfig()->show_previous_values;
  }
  else if (kdown & KEY_MINUS)
  {
    Config::getConfig()->exclude_ptr_candidates = !Config::getConfig()->exclude_ptr_candidates;
  }
  else if (kdown & KEY_DUP)
  {
    Config::getConfig()->enabletargetedscan = !Config::getConfig()->enabletargetedscan;
  }
  else if (kdown & KEY_DRIGHT)
  {
    Config::getConfig()->extraMB = Config::getConfig()->extraMB + 1;
  }
  else if (kdown & KEY_DLEFT)
  {
    if (Config::getConfig()->extraMB > 0)
      Config::getConfig()->extraMB = Config::getConfig()->extraMB - 1;
  }
  else if (kdown & KEY_DDOWN)
  {
    Config::getConfig()->use_absolute_address = !Config::getConfig()->use_absolute_address;
  }
  else if (kdown & KEY_PLUS)
  {
    Gui::g_nextGui = GUI_ABOUT;
  }
  else if (kdown & KEY_LSTICK) 
  {
    Config::getConfig()->easymode = !Config::getConfig()->easymode;
    Config::getConfig()->options[0] = true;
  }
  else if (kdown & KEY_RSTICK) 
  {
    Config::getConfig()->freeze = !Config::getConfig()->freeze;
  }
  else if (kdown & KEY_A)
  {
    Gui::g_nextGui = GUI_CHEATS;
    memcpy(Config::getConfig()->edizon_dir, m_edizon_dir.c_str(), m_edizon_dir.size());
    Config::getConfig()->edizon_dir[m_edizon_dir.size()] = 0;
    Config::writeConfig();
  }
}

void GuiMore::onTouch(touchPosition &touch) {
  if (touch.px > 400 && touch.px < 900 && touch.py > 600 && touch.py < 660) {
    Config::getConfig()->hideSX = !Config::getConfig()->hideSX;
    Config::writeConfig();
  }
}

void GuiMore::onGesture(touchPosition startPosition, touchPosition endPosition, bool finish) {

}
