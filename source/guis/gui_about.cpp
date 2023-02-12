#include "guis/gui_about.hpp"
#include "unzip1.hpp"
#include <thread>
#include <curl/curl.h>
#include "version.h"
#include "update_manager.hpp"
#include "helpers/util.h"
#include "helpers/config.hpp"
#include "helpers/debugger.hpp"
#define VER_URL "https://github.com/tomvita/EdiZon-SE/releases/latest/download/version.txt"
#define APP_URL "https://github.com/tomvita/EdiZon-SE/releases/latest/download/EdiZon.zip"
#define APP_OUTPUT "/switch/EdiZon/EdiZon.nro"
#define VER_OUTPUT "/switch/EdiZon/version.txt"
#define TEMP_FILE "/switch/EdiZon/Edizontemp"
static std::string remoteVersion, remoteCommitSha, remoteCommitMessage;
static Thread networkThread;
static bool threadRunning;
static bool updateAvailable;

static void getVersionInfoAsync(void* args);

GuiAbout::GuiAbout() : Gui() {
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

GuiAbout::~GuiAbout() {

}

void GuiAbout::update() {
  Gui::update();
}

void GuiAbout::draw() {
  Gui::beginDraw();

  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, currTheme.backgroundColor);
  Gui::drawRectangle((u32)((Gui::g_framebuffer_width - 1220) / 2), 87, 1220, 1, currTheme.textColor);
  Gui::drawRectangle((u32)((Gui::g_framebuffer_width - 1220) / 2), Gui::g_framebuffer_height - 73, 1220, 1, currTheme.textColor);
  Gui::drawTextAligned(fontTitle, 70, 60, currTheme.textColor, "\uE017", ALIGNED_LEFT);
  Gui::drawTextAligned(font24, 70, 23, currTheme.textColor, "        About", ALIGNED_LEFT);

  if (updateAvailable)
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 51, currTheme.textColor, "\uE0F0 Install update     \uE0E1 Back     \uE0E0 OK", ALIGNED_RIGHT);
  else
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 51, currTheme.textColor, "\uE0E1 Back     \uE0E0 OK", ALIGNED_RIGHT);

  Gui::drawTextAligned(fontHuge, 100, 180, Gui::makeColor(0xFB, 0xA6, 0x15, 0xFF), "EdiZon SE v" VERSION_STRING, ALIGNED_LEFT);
  Gui::drawTextAligned(font20, 130, 190, currTheme.separatorColor, "by Tomvita", ALIGNED_LEFT);

  Gui::drawTextAligned(font14, 120, 250, currTheme.textColor, "Special thank to WerWolv who made the original EdiZon and the help and advise he gave.", ALIGNED_LEFT);
  // Gui::drawTextAligned(font14, 120, 270, currTheme.textColor, "Especially to all the config/cheat developers that brought this project to life!", ALIGNED_LEFT);

  // Gui::drawTextAligned(font14, 900, 250, Gui::makeColor(0x51, 0x97, 0xF0, 0xFF), "Twitter: https://twitter.com/WerWolv", ALIGNED_LEFT);
  // Gui::drawTextAligned(font14, 900, 275, Gui::makeColor(0x1A, 0x5E, 0xA7, 0xFF), "PayPal:  https://werwolv.net/donate", ALIGNED_LEFT);


  Gui::drawRectangled(50, 350, Gui::g_framebuffer_width - 100, 250, currTheme.textColor);
  Gui::drawRectangled(51, 351, Gui::g_framebuffer_width - 102, updateAvailable ? 210 : 248, currTheme.backgroundColor);
  Gui::drawShadow(52, 352, Gui::g_framebuffer_width - 104, 248);

  if (updateAvailable)
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, 565, currTheme.backgroundColor, "A update for EdiZon SE is available!", ALIGNED_CENTER);

  Gui::drawTextAligned(font20, 60, 360, currTheme.selectedColor, "EdiZon SE Update", ALIGNED_LEFT);

  Gui::drawTextAligned(font14, 80, 400, currTheme.textColor, std::string("Latest EdiZon SE version: " + (remoteVersion == "" ? "..." : remoteVersion)).c_str(), ALIGNED_LEFT);
  // Gui::drawTextAligned(font14, 80, 425, currTheme.textColor, std::string("Latest database commit: [ " + (remoteCommitSha == "" ? "..." : remoteCommitSha) + " ] ").c_str(), ALIGNED_LEFT);
  // Gui::drawTextAligned(font14, 90, 450, currTheme.separatorColor, (remoteCommitMessage == "" ? "..." : remoteCommitMessage.c_str()), ALIGNED_LEFT);

  Gui::endDraw();
}

static size_t writeToFile(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

void GuiAbout::onInput(u32 kdown) {
  if (kdown & HidNpadButton_B) {
    if (threadRunning) {
      threadWaitForExit(&networkThread);
      threadClose(&networkThread);
      threadRunning = false;
    }
    Debugger *l_debugger = new Debugger();
    if (l_debugger->getRunningApplicationPID() != 0)
      if (Config::getConfig()->easymode)
        Gui::g_nextGui = GUI_FIRST_RUN;
      else
        Gui::g_nextGui = GUI_CHOOSE_MISSION;
    else
      Gui::g_nextGui = GUI_MAIN;
  }

  if (kdown & HidNpadButton_Minus && updateAvailable && true) {

    (new MessageBox("Updating EdiZon.\n \nThis may take a while...", MessageBox::NONE))->show();
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
      romfsExit();
      printf("remove(APP_OUTPUT) = %d\n", remove(APP_OUTPUT));
      (new MessageBox("Updated EdiZon\n Please restart EdiZon!", MessageBox::OKAY))->show();
      inst::zip::extractFile(TEMP_FILE, "sdmc:/");
      // printf("rename(TEMP_FILE, APP_OUTPUT) = %d\n", rename(TEMP_FILE, APP_OUTPUT));
      updateAvailable = false;
      if (threadRunning)
      {
        threadWaitForExit(&networkThread);
        threadClose(&networkThread);
        threadRunning = false;
      }
      if (!Config::getConfig()->easymode)
        Gui::g_requestExit = true;
    }
    else
    {
      (new MessageBox("Not able to updated EdiZon\n Please try again later!", MessageBox::OKAY))->show();
      fclose(fp);
      curl_easy_cleanup(curl);
    }
    
  }
}

void GuiAbout::onTouch(HidTouchState &touch) {

}

void GuiAbout::onGesture(HidTouchScreenState startPosition, HidTouchScreenState endPosition, bool finish) {

}

static size_t writeToStr(const char * contents, size_t size, size_t nmemb, std::string * userp){
    auto totalBytes = (size * nmemb);

    userp->append(contents, totalBytes);

    return totalBytes;
}



static void getVersionInfoAsync(void* args) {
  // if (!downloadFile(VER_URL, TEMP_FILE, OFF))
  // {
  //   remove(APP_OUTPUT);
  //   rename(TEMP_FILE, APP_OUTPUT);
  // }
  CURL *curl = curl_easy_init();

  struct curl_slist * headers = NULL;
  headers = curl_slist_append(headers, "Cache-Control: no-cache");
// FILE *fp = fopen(TEMP_FILE, "wb");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_URL, VER_URL);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "API_AGENT");
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToStr);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &remoteVersion);
//  fclose(fp); 

  remoteVersion = "";
  if (curl_easy_perform(curl) != CURLE_OK)
    remoteVersion = "???";

  if (remoteVersion.compare(0, 6, VERSION_STRING) == 0 || strcmp(remoteCommitSha.c_str(), "???") == 0)
  {
    updateAvailable = false;
  }
  else
  {
    updateAvailable = true;
  }

  // curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  // curl_easy_setopt(curl, CURLOPT_URL, EDIZON_URL "/info/latest_db_sha.php");
  // curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToStr);
  // curl_easy_setopt(curl, CURLOPT_WRITEDATA, &remoteCommitSha);

  // remoteCommitSha = "";

  // if (curl_easy_perform(curl) != CURLE_OK)
  //   remoteCommitSha = "???";

  // curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  // curl_easy_setopt(curl, CURLOPT_URL, EDIZON_URL "/info/latest_db_message.php");
  // curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToStr);
  // curl_easy_setopt(curl, CURLOPT_WRITEDATA, &remoteCommitMessage);

  // remoteCommitMessage = "";

  // if (curl_easy_perform(curl) != CURLE_OK)
  //   remoteCommitMessage = "???";

  curl_easy_cleanup(curl);

  // updateAvailable = (strcmp(remoteCommitSha.c_str(), Config::getConfig()->latestCommit) != 0 && strcmp(remoteCommitSha.c_str(), "???") != 0);
}
