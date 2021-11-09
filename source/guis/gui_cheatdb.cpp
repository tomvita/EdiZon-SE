#include "guis/gui_cheatdb.hpp"

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

Guicheatdb::Guicheatdb() : Gui() {
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

Guicheatdb::~Guicheatdb() {

}

void Guicheatdb::update() {
  Gui::update();
}

void Guicheatdb::draw() {
  Gui::beginDraw();

  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, currTheme.backgroundColor);
  Gui::drawRectangle((u32)((Gui::g_framebuffer_width - 1220) / 2), 87, 1220, 1, currTheme.textColor);
  Gui::drawRectangle((u32)((Gui::g_framebuffer_width - 1220) / 2), Gui::g_framebuffer_height - 73, 1220, 1, currTheme.textColor);
  Gui::drawTextAligned(fontTitle, 70, 60, currTheme.textColor, "\uE133", ALIGNED_LEFT);
  Gui::drawTextAligned(font24, 70, 23, currTheme.textColor, "        Cheat database updater", ALIGNED_LEFT);

  if (updateAvailable)
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 51, currTheme.textColor, "\uE0F0 Install update     \uE0E1 Exit     \uE0E0 OK", ALIGNED_RIGHT);
  else
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 51, currTheme.textColor, "\uE0E1 Exit     \uE0E0 OK", ALIGNED_RIGHT);

  // Gui::drawTextAligned(font24, 100, 180, Gui::makeColor(0xFB, 0xA6, 0x15, 0xFF), "EdiZon SE v" VERSION_STRING, ALIGNED_LEFT);
  // Gui::drawTextAligned(font20, 130, 190, currTheme.separatorColor, "by Tomvita", ALIGNED_LEFT);

  Gui::drawTextAligned(font14, 120, 250, currTheme.textColor, "Special thank to everyone who has contributed at Gbatemp Switch cheat code forum.", ALIGNED_LEFT);
  // Gui::drawTextAligned(font14, 120, 270, currTheme.textColor, "Especially to all the config/cheat developers that brought this project to life!", ALIGNED_LEFT);

  // Gui::drawTextAligned(font14, 900, 250, Gui::makeColor(0x51, 0x97, 0xF0, 0xFF), "Twitter: https://twitter.com/WerWolv", ALIGNED_LEFT);
  // Gui::drawTextAligned(font14, 900, 275, Gui::makeColor(0x1A, 0x5E, 0xA7, 0xFF), "PayPal:  https://werwolv.net/donate", ALIGNED_LEFT);


  Gui::drawRectangled(50, 350, Gui::g_framebuffer_width - 100, 250, currTheme.textColor);
  Gui::drawRectangled(51, 351, Gui::g_framebuffer_width - 102, updateAvailable ? 210 : 248, currTheme.backgroundColor);
  Gui::drawShadow(52, 352, Gui::g_framebuffer_width - 104, 248);

  if (updateAvailable)
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, 565, currTheme.backgroundColor, "A update for Cheat database is available!", ALIGNED_CENTER);

  Gui::drawTextAligned(font20, 60, 360, currTheme.selectedColor, "Cheat database Update", ALIGNED_LEFT);

  Gui::drawTextAligned(font14, 80, 400, currTheme.textColor, std::string("Latest cheat database version: " + (remoteVersion == "" ? "..." : remoteVersion)).c_str(), ALIGNED_LEFT);
  // Gui::drawTextAligned(font14, 80, 425, currTheme.textColor, std::string("Latest database commit: [ " + (remoteCommitSha == "" ? "..." : remoteCommitSha) + " ] ").c_str(), ALIGNED_LEFT);
  // Gui::drawTextAligned(font14, 90, 450, currTheme.separatorColor, (remoteCommitMessage == "" ? "..." : remoteCommitMessage.c_str()), ALIGNED_LEFT);

  Gui::endDraw();
}

static size_t writeToFile(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

void Guicheatdb::onInput(u32 kdown) {
  if (kdown & HidNpadButton_B) {
    if (threadRunning) {
      threadWaitForExit(&networkThread);
      threadClose(&networkThread);
      threadRunning = false;
    }
    // Debugger *l_debugger = new Debugger();
    // if (l_debugger->getRunningApplicationPID() != 0)
    //   Gui::g_nextGui = GUI_CHOOSE_MISSION;
    // else
      Gui::g_requestExit = true;
  }

  if (kdown & HidNpadButton_Minus && updateAvailable)  {

    mkdir(CHEATS_DIR, 0777);
    (new MessageBox("Updating cheat database.\n \nThis may take a while...", MessageBox::NONE))->show();
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
      (new MessageBox("Updated cheat code database\n\n Enjoy!", MessageBox::OKAY))->show();
      printf("rename(TEMP_FILE, APP_OUTPUT) = %d\n", rename(TEMP_FILE, APP_OUTPUT));
      updateAvailable = false;
      for (int i=0;i<6;i++)
        Config::getConfig()->dbversion[i] = remoteVersion[i];
      Config::writeConfig();
    }
    else
    {
      (new MessageBox("Not able to updated cheat database\n Please try again later!", MessageBox::OKAY))->show();
      fclose(fp);
      curl_easy_cleanup(curl);
    }
  }
}

void Guicheatdb::onTouch(HidTouchState &touch) {

}

void Guicheatdb::onGesture(HidTouchScreenState startPosition, HidTouchScreenState endPosition, bool finish) {

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
  Config::readConfig();
  // std::string version = Config::getConfig()->version;
  if (remoteVersion.compare(0, 6, Config::getConfig()->dbversion) == 0 || strcmp(remoteCommitSha.c_str(), "???") == 0)
  {
    updateAvailable = false;
  }
  else
  {
    updateAvailable = true;
  }
  if (access(APP_OUTPUT, F_OK)!=0) updateAvailable = true;

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
