#include "guis/gui_tx_warning.hpp"

#include "helpers/config.hpp"

GuiTXWarning::GuiTXWarning() : Gui() {
  Config::getConfig()->hideSX = false;
}

GuiTXWarning::~GuiTXWarning() {

}

void GuiTXWarning::update() {
  Gui::update();
}

void GuiTXWarning::draw() {
  Gui::beginDraw();

  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, Gui::makeColor(0xC5, 0x39, 0x29, 0xFF));
  Gui::drawTextAligned(fontHuge, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 - 100, COLOR_WHITE, "\uE150", ALIGNED_CENTER);

  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2, COLOR_WHITE, "EdiZon检测到您正在运行'SX OS' CFW。请注意，此CFW错误地\n实现系统服务，可能导致意外故障、存档或备份数据损坏，\n编辑器无法加载存档或配置文件、RAM编辑不被支持\n和其他问题。为了您的SWITCH的安全，请改用免费的开源CFW。\n无论如何要继续按 \uE0E0，否则按 \uE0E1 退出。", ALIGNED_CENTER);

  Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 + 250, COLOR_WHITE, "\uE070  不再显示此警告", ALIGNED_CENTER);

  if (!Config::getConfig()->hideSX)
    Gui::drawRectangle(Gui::g_framebuffer_width / 2 - 228, Gui::g_framebuffer_height / 2 + 258, 14, 16, Gui::makeColor(0xC5, 0x39, 0x29, 0xFF));

  Gui::endDraw();
}

void GuiTXWarning::onInput(u32 kdown) {
  if (kdown & KEY_B)
    Gui::g_requestExit = true;

  if (kdown & KEY_A)
    Gui::g_nextGui = GUI_MAIN;
}

void GuiTXWarning::onTouch(touchPosition &touch) {
  if (touch.px > 400 && touch.px < 900 && touch.py > 600 && touch.py < 660) {
    Config::getConfig()->hideSX = !Config::getConfig()->hideSX;
    Config::writeConfig();
  }
}

void GuiTXWarning::onGesture(touchPosition startPosition, touchPosition endPosition, bool finish) {

}
