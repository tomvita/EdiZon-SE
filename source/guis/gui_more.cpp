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
static const char *const optionNames[] = {"No Auto Attach \uE0A2", "No Auto Exit after detach \uE0A3", "Disable this screen \uE0B4"};
bool screen2 = false;
void draw2() {

}
void GuiMore::draw() {
  if (screen2)
  {
    draw2();
    return;
  }
  std::stringstream extra_seg_str, two_value_range_str, bit_mask_str;
  extra_seg_str << "\uE0B2 +  \uE0B1 -  Extra MB " << Config::getConfig()->extraMB;
  two_value_range_str << "\uE0A5 +  \uE0A4 - 2 value search range " << Config::getConfig()->two_value_range;
  bit_mask_str << "\uE0A6 BM=" << std::uppercase << std::hex << std::setfill('0') << std::setw(16) << Config::getConfig()->bitmask;
  Gui::beginDraw();
  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, Gui::makeColor(0x30, 0x39, 0x29, 0xFF));
  Gui::drawTextAligned(fontHuge, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 - 100, COLOR_WHITE, "More options", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2, COLOR_BLACK, "", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2+60, COLOR_BLACK, "Use X, Y, - to toggle options, if you disable this screen use R+B to exit will show this on next launch", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 5, Gui::g_framebuffer_height / 2 + 200, Config::getConfig()->enabletargetedscan ? COLOR_WHITE : COLOR_BLACK, "\uE0AF Enable targeted scan", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 + 200, Config::getConfig()->enabletargetedscan ? COLOR_WHITE : COLOR_BLACK, extra_seg_str.str().c_str(), ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width *4 / 5, Gui::g_framebuffer_height / 2 + 200, Config::getConfig()->use_absolute_address ? COLOR_WHITE : COLOR_BLACK, "\uE0B0 Use absolute address", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 5, Gui::g_framebuffer_height / 2 + 250, Config::getConfig()->easymode ? COLOR_WHITE : COLOR_BLACK, "\uE0C4 Easy Mode", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width * 4 / 5, Gui::g_framebuffer_height / 2+250, Config::getConfig()->freeze ? COLOR_WHITE : COLOR_BLACK, "\uE0C5 add freeze game code", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 + 250, COLOR_WHITE, two_value_range_str.str().c_str() , ALIGNED_CENTER);//"\uE070  Don't show this warning anymore"
  // for (u8 i = 0; i < 3; i++)
  // {
  //   // Gui::drawRectangled((Gui::g_framebuffer_width / 4) * (i + 1), Gui::g_framebuffer_height / 2 + 270, 300, 60, currTheme.separatorColor);
  //   Gui::drawTextAligned(font20, (Gui::g_framebuffer_width / 4) * (i + 1), Gui::g_framebuffer_height / 2 + 300, Config::getConfig()->options[i] ? COLOR_WHITE : COLOR_BLACK, optionNames[i], ALIGNED_CENTER);
  // }
  // Gui::drawTextAligned(font20, (Gui::g_framebuffer_width / 5), Gui::g_framebuffer_height / 2 + 300, Config::getConfig()->extra_value ? COLOR_WHITE : COLOR_BLACK, "\uE0A2 use extra search value", ALIGNED_CENTER);
  // Gui::drawTextAligned(font20, (Gui::g_framebuffer_width / 2), Gui::g_framebuffer_height / 2 + 300, Config::getConfig()->show_previous_values ? COLOR_WHITE : COLOR_BLACK, "\uE0A3 show previous values", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, (Gui::g_framebuffer_width / 5), Gui::g_framebuffer_height / 2 + 300, Config::getConfig()->options[0] ? COLOR_WHITE : COLOR_BLACK, optionNames[0], ALIGNED_CENTER);
  Gui::drawTextAligned(font20, (Gui::g_framebuffer_width / 2), Gui::g_framebuffer_height / 2 + 300, Config::getConfig()->options[1] ? COLOR_WHITE : COLOR_BLACK, optionNames[1], ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width * 4 / 5, Gui::g_framebuffer_height / 2 + 300, Config::getConfig()->exclude_ptr_candidates ? COLOR_WHITE : COLOR_BLACK, "\uE0B4 exclue pointer candidates", ALIGNED_CENTER);

  Gui::drawTextAligned(font20, Gui::g_framebuffer_width * 4 / 5, Gui::g_framebuffer_height / 2 + 150, COLOR_WHITE, "About \uE0B3", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 5, Gui::g_framebuffer_height / 2 + 150, Config::getConfig()->use_bitmask ? COLOR_WHITE : COLOR_BLACK, Config::getConfig()->use_bitmask ? bit_mask_str.str().c_str() : "\uE0A6 use bit mask", ALIGNED_CENTER);
  Gui::endDraw();
}
// u32 kheld = hidKeysHeld(HidNpadIdType_No1) | hidKeysHeld(HidNpadIdType_Handheld);
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
  if (kdown & HidNpadButton_L)
  {
    // m_edizon_dir = "/switch/EdiZon/1";
    if (Config::getConfig()->two_value_range > 0)
        Config::getConfig()->two_value_range = Config::getConfig()->two_value_range - 1;
  }
  else if (kdown & HidNpadButton_R)
  {
    // m_edizon_dir = "/switch/EdiZon/2";
    Config::getConfig()->two_value_range = Config::getConfig()->two_value_range + 1;
  }

  if (kdown & HidNpadButton_ZL)
  {
    // m_edizon_dir = "/switch/EdiZon/3";
    Config::getConfig()->use_bitmask = !Config::getConfig()->use_bitmask;
    if (Config::getConfig()->use_bitmask) {
        if (Config::getConfig()->bitmask == 0) {
            Config::getConfig()->bitmask = 0xFFFFFFFFFFFFFFFF;
        };
        char input[19];
        char initialString[21];
        snprintf(initialString, sizeof(initialString), "0x%016lX", Config::getConfig()->bitmask);
        if (Gui::requestKeyboardInput("Enter value", "Enter the bit mask to apply to search value.", initialString, SwkbdType_QWERTY, input, 18)) {
            Config::getConfig()->bitmask = static_cast<u64>(std::stoul(input, nullptr, 16));
        }
    }
  }
  else if (kdown & HidNpadButton_ZR)
  {
    // m_edizon_dir = "/switch/EdiZon/4";
  }
  else if (kdown & HidNpadButton_B)
  {
    // m_edizon_dir = "/switch/EdiZon";
  }
  else if (kdown & HidNpadButton_X)
  {
    Config::getConfig()->options[0] = !Config::getConfig()->options[0];
    // Config::getConfig()->extra_value = !Config::getConfig()->extra_value;
  }
  else if (kdown & HidNpadButton_Y)
  {
    Config::getConfig()->options[1] = !Config::getConfig()->options[1];
    // Config::getConfig()->show_previous_values = !Config::getConfig()->show_previous_values;
  }
  else if (kdown & HidNpadButton_Minus)
  {
    Config::getConfig()->exclude_ptr_candidates = !Config::getConfig()->exclude_ptr_candidates;
  }
  else if (kdown & HidNpadButton_Up)
  {
    Config::getConfig()->enabletargetedscan = !Config::getConfig()->enabletargetedscan;
  }
  else if (kdown & HidNpadButton_Right)
  {
    Config::getConfig()->extraMB = Config::getConfig()->extraMB + 1;
  }
  else if (kdown & HidNpadButton_Left)
  {
    if (Config::getConfig()->extraMB > 0)
      Config::getConfig()->extraMB = Config::getConfig()->extraMB - 1;
  }
  else if (kdown & HidNpadButton_Down)
  {
    Config::getConfig()->use_absolute_address = !Config::getConfig()->use_absolute_address;
  }
  else if (kdown & HidNpadButton_Plus)
  {
    Gui::g_nextGui = GUI_ABOUT;
  }
  else if (kdown & HidNpadButton_StickL) 
  {
    Config::getConfig()->easymode = !Config::getConfig()->easymode;
    Config::getConfig()->options[0] = true;
  }
  else if (kdown & HidNpadButton_StickR) 
  {
    Config::getConfig()->freeze = !Config::getConfig()->freeze;
  }
  else if (kdown & HidNpadButton_A)
  {
    Gui::g_nextGui = GUI_CHEATS;
    memcpy(Config::getConfig()->edizon_dir, m_edizon_dir.c_str(), m_edizon_dir.size());
    Config::getConfig()->edizon_dir[m_edizon_dir.size()] = 0;
    Config::writeConfig();
  }
}

void GuiMore::onTouch(HidTouchState &touch) {
  if (touch.x > 400 && touch.x < 900 && touch.y > 600 && touch.y < 660) {
    Config::getConfig()->hideSX = !Config::getConfig()->hideSX;
    Config::writeConfig();
  }
}

void GuiMore::onGesture(HidTouchScreenState startPosition, HidTouchScreenState endPosition, bool finish) {

}
