#include "guis/gui_first_run.hpp"
#include <thread>
#include <curl/curl.h>
#include "update_manager.hpp"
#include "helpers/util.h"
#include "helpers/config.hpp"
#include "helpers/debugger.hpp"
#define VER_URL "https://github.com/tomvita/NXCheatCode/releases/latest/download/version.txt"
#define APP_URL "https://github.com/tomvita/NXCheatCode/releases/latest/download/titles.zip"
#define APP_OUTPUT "/switch/EdiZon/cheats/titles.zip"
#define CHEATS_DIR "/switch/EdiZon/cheats/"
#define VER_OUTPUT "/switch/EdiZon/version.txt"
#define TEMP_FILE "/switch/EdiZon/Edizontemp"
static std::string remoteVersion, remoteCommitSha, remoteCommitMessage;
static Thread networkThread;
static bool threadRunning;
static bool updateAvailable;
static void getVersionInfoAsync(void* args);
Guifirstrun::Guifirstrun() : Gui() {
  updateAvailable = false;
  remoteVersion = "";
  remoteCommitSha = "";
  remoteCommitMessage = "";
  if (!threadRunning) {
    threadRunning = true;
    threadCreate(&networkThread, getVersionInfoAsync, nullptr, nullptr, 0x2000, 0x2C, -2);
    threadStart(&networkThread);
  }
}
Guifirstrun::~Guifirstrun() {
}
void Guifirstrun::update() {
  Gui::update();
}
// static std::string status = "";
static size_t writeToFile(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}
static void getdb()
{
  mkdir(CHEATS_DIR, 0777);
  // status = "Downloading cheat database, this may take a while ";
  (new MessageBox("更新金手指数据库。\n \n这需要一点时间...", MessageBox::NONE))->show();
  requestDraw();
  CURL *curl = curl_easy_init();
  struct curl_slist *headers = NULL;
  headers = curl_slist_append(headers, "Cache-Control: no-cache");
  FILE *fp = fopen(TEMP_FILE, "wb");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_URL, APP_URL);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "API_AGENT");
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToFile);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
  if (curl_easy_perform(curl) == CURLE_OK)
  {
    fclose(fp);
    curl_easy_cleanup(curl);
    printf("remove(APP_OUTPUT) = %d\n", remove(APP_OUTPUT));
    // status = "Cheat database downloaded";
    (new MessageBox("金手指代码数据库已更新\n\n 请享受！", MessageBox::OKAY))->show();
    printf("rename(TEMP_FILE, APP_OUTPUT) = %d\n", rename(TEMP_FILE, APP_OUTPUT));
    updateAvailable = false;
    for (int i = 0; i < 6; i++)
      Config::getConfig()->dbversion[i] = remoteVersion[i];
    Config::writeConfig();
  }
  else
  {
    // status = "Cheat database download failed";
    (new MessageBox("无法更新金手指数据库\n 请稍后再试！", MessageBox::OKAY))->show();
    fclose(fp);
    curl_easy_cleanup(curl);
  }
}
void Guifirstrun::draw() {
  Gui::beginDraw();
  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, currTheme.backgroundColor);
  Gui::drawRectangle((u32)((Gui::g_framebuffer_width - 1220) / 2), 87, 1220, 1, currTheme.textColor);
  Gui::drawRectangle((u32)((Gui::g_framebuffer_width - 1220) / 2), Gui::g_framebuffer_height - 73, 1220, 1, currTheme.textColor);
  Gui::drawTextAligned(fontTitle, 70, 60, currTheme.textColor, "\uE017", ALIGNED_LEFT);
  Gui::drawTextAligned(font24, 70, 23, currTheme.textColor, "        欢迎使用EdiZon SE", ALIGNED_LEFT);
    Gui::drawTextAligned(fontHuge, 100, 180, Gui::makeColor(0xFB, 0xA6, 0x15, 0xFF), "EdiZon SE v" VERSION_STRING, ALIGNED_LEFT);
  Gui::drawTextAligned(font20, 130, 190, currTheme.separatorColor, "由Tomvita制作", ALIGNED_LEFT);
  Gui::drawTextAligned(font20, 50, Gui::g_framebuffer_height - 51, Config::getConfig()->easymode ? currTheme.textColor : COLOR_RED, Config::getConfig()->easymode ? "\uE0E2 建议模式" : "\uE0E2 专家模式", ALIGNED_LEFT);
  if (updateAvailable)
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 51, currTheme.textColor, "\uE0EF 程序更新检查                       \uE0F0 安装更新     \uE0E1 忽略", ALIGNED_RIGHT);
  else
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 51, currTheme.textColor, "\uE0EF 程序更新检查                                                 \uE0E1 退出", ALIGNED_RIGHT);
  Gui::drawTextAligned(font14, 120, 250, currTheme.textColor, "正在检查金手指数据库。特别感谢在Gbatemp Switch作弊代码论坛上做出的贡献的所有人。", ALIGNED_LEFT);
  Gui::drawRectangled(50, 350, Gui::g_framebuffer_width - 100, 250, currTheme.textColor);
  Gui::drawRectangled(51, 351, Gui::g_framebuffer_width - 102, updateAvailable ? 210 : 248, currTheme.backgroundColor);
  Gui::drawShadow(52, 352, Gui::g_framebuffer_width - 104, 248);
  if (updateAvailable)
  {
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, 565, currTheme.backgroundColor, "金手指数据库更新可用！", ALIGNED_CENTER);
    
    // getdb();
  }
  Gui::drawTextAligned(font20, 60, 360, currTheme.selectedColor, "金手指数据库更新", ALIGNED_LEFT);
  Gui::drawTextAligned(font14, 80, 400, currTheme.textColor, std::string("最新的金手指数据库版本：" + (remoteVersion == "" ? "..." : remoteVersion)).c_str(), ALIGNED_LEFT);
  // Gui::drawTextAligned(font14, 80, 440, currTheme.textColor, status.c_str(), ALIGNED_LEFT);
  Gui::endDraw();

}
void Guifirstrun::onInput(u32 kdown) {
  if (kdown & KEY_B) {
    if (threadRunning) {
      threadWaitForExit(&networkThread);
      threadClose(&networkThread);
      threadRunning = false;
    }
      Gui::g_nextGui = GUI_CHEATS;
      Config::writeConfig();
  }
  else if (kdown & KEY_X)
  {
    Config::getConfig()->easymode = !Config::getConfig()->easymode;
  }
    else if (kdown & KEY_PLUS)
  {
    Gui::g_nextGui = GUI_ABOUT;
  }
  else if (kdown & KEY_MINUS && updateAvailable)
  {
    getdb();
    // Gui::g_nextGui = GUI_CHEATS;
  }
}
void Guifirstrun::onTouch(touchPosition &touch) {
}
void Guifirstrun::onGesture(touchPosition startPosition, touchPosition endPosition, bool finish) {
}
static size_t writeToStr(const char * contents, size_t size, size_t nmemb, std::string * userp){
    auto totalBytes = (size * nmemb);
    userp->append(contents, totalBytes);
    return totalBytes;
}
static void getVersionInfoAsync(void* args) {
  CURL *curl = curl_easy_init();
  struct curl_slist * headers = NULL;
  headers = curl_slist_append(headers, "Cache-Control: no-cache");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_URL, VER_URL);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "API_AGENT");
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToStr);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &remoteVersion);
  remoteVersion = "";
  if (curl_easy_perform(curl) != CURLE_OK)
    remoteVersion = "???";
  curl_easy_cleanup(curl);
  Config::readConfig();
  if (remoteVersion.compare(0, 6, Config::getConfig()->dbversion) == 0 || strcmp(remoteCommitSha.c_str(), "???") == 0)
  {
    updateAvailable = false;
  }
  else
  {
    updateAvailable = true;
    // getdb();
  }
  if (access(APP_OUTPUT, F_OK)!=0) updateAvailable = true;
}
