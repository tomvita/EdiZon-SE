#include <edizon.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>
#include "version.h"
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <curl/curl.h>

#include "guis/gui.hpp"
#include "guis/gui_main.hpp"
#include "guis/gui_editor.hpp"
// #include "guis/gui_tx_warning.hpp"
#include "guis/gui_choose_mission.hpp"
#include "guis/gui_more.hpp"
#include "guis/gui_cheats.hpp"
#include "guis/gui_sysmodule.hpp"
#include "guis/gui_about.hpp"
#include "guis/gui_cheatdb.hpp"
#include "guis/gui_first_run.hpp"
#include "guis/gui_memory_editor.hpp"
#include "guis/gui_guide.hpp"

#include "helpers/title.hpp"

#include "theme.h"
#include "helpers/util.h"
#include "helpers/config.hpp"

#define LONG_PRESS_DELAY 2
#define LONG_PRESS_ACTIVATION_DELAY 300

char *g_edizonPath;

static int debugOutputFile;
std::string m_edizon_dir = "/switch/EdiZon";
std::string m_store_extension = "A";
static bool updateThreadRunning = false;
static Mutex mutexCurrGui;
static Gui *currGui = nullptr;
static s64 inputTicker = 0;

static u32 kheld = 0, kheldOld = 0;
static u32 kdown = 0;

void initTitles()
{
  std::vector<FsSaveDataInfo> saveInfoList;

  _getSaveList(saveInfoList);

  s32 userCount = 0;
  s32 foundUserCount = 0;

  accountGetUserCount(&userCount);

  AccountUid userIDs[userCount];
  accountListAllUsers(userIDs, userCount, &foundUserCount);

  for (auto saveInfo : saveInfoList)
  {
    bool accountPresent = false;

    for (s32 i = 0; i < foundUserCount; i++)
      if (userIDs[i] == saveInfo.uid)
        accountPresent = true;

    if (!accountPresent)
      continue;

    if (Title::g_titles.find(saveInfo.application_id) == Title::g_titles.end())
      Title::g_titles.insert({(u64)saveInfo.application_id, new Title(saveInfo)});

    Title::g_titles[saveInfo.application_id]->addUserID(saveInfo.uid);

    if (Account::g_accounts.find(saveInfo.uid) == Account::g_accounts.end())
    {
      Account *account = new Account(saveInfo.uid);

      if (!account->isInitialized())
      {
        delete account;
        continue;
      }
      Account::g_accounts.insert(std::make_pair(static_cast<AccountUid>(saveInfo.uid), account));
    }
    // break; //hack
  }
}

void update()
{
  while (updateThreadRunning)
  {
    auto begin = std::chrono::steady_clock::now();

    mutexLock(&mutexCurrGui);
    if (currGui != nullptr)
      currGui->update();
    mutexUnlock(&mutexCurrGui);

    if (kheld & (HidNpadButton_AnyLeft | HidNpadButton_AnyRight | HidNpadButton_AnyUp | HidNpadButton_AnyDown))
      inputTicker++;
    else
      inputTicker = 0;

    svcSleepThread(1.0E6 - std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - begin).count());
  }
}

bool multimissioncheck()
{
  Config::readConfig();
  if (Config::getConfig()->option_once || (Config::getConfig()->options[2] == 0))
    return true;
  else
    return false;
  // std::stringstream filenoiconStr;
  // filenoiconStr << EDIZON_DIR "/nomultimission.txt";
  // if (access(filenoiconStr.str().c_str(), F_OK) != 0)
  // {
  //   return true;
  // }
  // else
  //   return false;
}
bool firstruncheck()
{
  Config::readConfig();
  if (!Config::getConfig()->not_first_run)
  {
    Config::getConfig()->not_first_run = true;
    Config::getConfig()->options[0] = true; // No Auto Attach for fetching of game code
    Config::getConfig()->options[2] = true; // Disable config screen
    Config::getConfig()->easymode = true;
    Config::writeConfig();
    return true;
  }
  else
    return false;
}
bool showallgamesavecheck()
{
  Config::readConfig();
  std::stringstream filenoiconStr;
  filenoiconStr << EDIZON_DIR "/showallsaves.txt";
  if (access(filenoiconStr.str().c_str(), F_OK) == 0)
  {
    Config::getConfig()->showallsaves = true;
    Config::writeConfig();
    return true;
  }
  else
  {
    Config::getConfig()->showallsaves = false;
    Config::writeConfig();
    return false;
  }
}
void createFolders()
{
  printf(EDIZON_DIR "/saves\n");
  mkdir("/switch", 0777);
  mkdir(EDIZON_DIR "", 0777);
  mkdir(EDIZON_DIR "/saves", 0777);
  mkdir(EDIZON_DIR "/batch_saves", 0777);
  mkdir(EDIZON_DIR "/restore", 0777);
  mkdir(EDIZON_DIR "/editor", 0777);
  mkdir(EDIZON_DIR "/editor/scripts", 0777);
  mkdir(EDIZON_DIR "/editor/scripts/lib", 0777);
  mkdir(EDIZON_DIR "/editor/scripts/lib/python3.5", 0777);
  if (multimissioncheck())
  {
    mkdir(EDIZON_DIR "/1", 0777);
    mkdir(EDIZON_DIR "/2", 0777);
    mkdir(EDIZON_DIR "/3", 0777);
    mkdir(EDIZON_DIR "/4", 0777);
  };
}

void requestDraw()
{
  if (currGui != nullptr)
    currGui->draw();
}

void serviceInitialize()
{
  setsysInitialize();
  socketInitializeDefault();
  nsInitialize();
  accountInitialize(AccountServiceType_Administrator);
  plInitialize(PlServiceType_User);
  psmInitialize();
  pminfoInitialize();
  pmdmntInitialize();
  romfsInit();
  hidsysInitialize();
  pcvInitialize();
  clkrstInitialize();
  ledInit();

  curl_global_init(CURL_GLOBAL_ALL);

  u64 pid = 0;
  Title::g_activeTitle = 0;

  pmdmntGetApplicationProcessId(&pid);
  pminfoGetProgramId(&Title::g_activeTitle, pid);

  accountGetLastOpenedUser(&Account::g_activeUser);
}

void serviceExit()
{
  setsysExit();
  socketExit();
  nsExit();
  accountExit();
  plExit();
  psmExit();
  pminfoExit();
  pmdmntExit();
  romfsExit();
  hidsysExit();
  pcvExit();
  clkrstExit();

  curl_global_cleanup();

  close(debugOutputFile);
}

void redirectStdio()
{
  nxlinkStdio();

  debugOutputFile = open(EDIZON_DIR "/EdiZon.log", O_APPEND | O_WRONLY);

  if (debugOutputFile >= 0)
  {
    fflush(stdout);
    dup2(debugOutputFile, STDOUT_FILENO);
    fflush(stderr);
    dup2(debugOutputFile, STDERR_FILENO);
  }
}
int main(int argc, char **argv)
{
  void *haddr;

  serviceInitialize();

  redirectStdio();
  showallgamesavecheck();
  framebufferCreate(&Gui::g_fb_obj, nwindowGetDefault(), 1280, 720, PIXEL_FORMAT_RGBA_8888, 2);
  framebufferMakeLinear(&Gui::g_fb_obj);

  ColorSetId colorSetId;
  setsysGetColorSetId(&colorSetId);
  setTheme(colorSetId);

  Config::readConfig();
  memcpy(Config::getConfig()->version, VERSION_STRING, sizeof(VERSION_STRING));
  Config::getConfig()->version[sizeof(VERSION_STRING)] = 0;
  Config::writeConfig();

  Debugger *l_debugger = new Debugger(); //Debugger *m_debugger;
  if (l_debugger->getRunningApplicationPID() != 0)
  {
    Gui::g_splashDisplayed = true;
    Config::readConfig();
    m_edizon_dir = Config::getConfig()->edizon_dir;
    if (Config::getConfig()->store_extension[0] > 0)
      m_store_extension = Config::getConfig()->store_extension;
    if (m_edizon_dir.compare(0, sizeof(EDIZON_DIR)-1, EDIZON_DIR) != 0)
      m_edizon_dir = EDIZON_DIR;
    if (firstruncheck())
    {
      Gui::g_nextGui = GUI_FIRST_RUN;
    }
    else if (multimissioncheck())
      Gui::g_nextGui = GUI_CHOOSE_MISSION;
    else
      Gui::g_nextGui = GUI_CHEATS;
  }

  if (!Gui::g_splashDisplayed)
  {
    currGui = new GuiMain();
    currGui->draw();
  }

  Config::readConfig(); 
  initTitles();
  // while (!(kheld & HidNpadButton_ZL))
  // {
  //   hidScanInput();
  //   kheld = hidKeysHeld(HidNpadIdType_No1) | hidKeysHeld(HidNpadIdType_Handheld);
  //   kdown = hidKeysDown(HidNpadIdType_No1)|hidKeysDown(HidNpadIdType_Handheld);
  //   Gui::beginDraw();
  //   Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, currTheme.backgroundColor);
  //   Gui::drawTextAligned(fontHuge, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 - 100, currTheme.textColor, "\uE12C", ALIGNED_CENTER);
  //   Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2, currTheme.textColor, "Use L and R to choose your sessoin press ZL to continue", ALIGNED_CENTER);
  //   Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, "\uE0E1 Back", ALIGNED_RIGHT);
  //   Gui::endDraw();
  // }
  // if (kheld & HidNpadButton_ZR)
  //   m_edizon_dir = "/switch/EdiZon1";
  // if (kheld & HidNpadButton_L)
  //   m_edizon_dir = "/switch/EdiZon2";
  // printf("%s\n", m_edizon_dir.c_str());

  createFolders();



  // if (isServiceRunning("tx") && !isServiceRunning("rnx") && !Config::getConfig()->hideSX)
  //   Gui::g_nextGui = GUI_TX_WARNING;

  g_edizonPath = new char[strlen(argv[0]) + 1];
  strcpy(g_edizonPath, argv[0] + 5);

  mutexInit(&mutexCurrGui);

  updateThreadRunning = true;
  std::thread updateThread(update);

  padConfigureInput(1, HidNpadStyleSet_NpadStandard);
  PadState pad;
  padInitializeDefault(&pad);
  
  while (appletMainLoop())
  {
    padUpdate(&pad);
    kheld = padGetButtons(&pad);
    kdown = padGetButtonsDown(&pad);

    if (Gui::g_nextGui != GUI_INVALID)
    {
      mutexLock(&mutexCurrGui);
      if (currGui != nullptr)
      {
        delete currGui;
        currGui = nullptr;
      }

      do
      {
        gui_t nextGuiStart = Gui::g_nextGui;
        switch (Gui::g_nextGui)
        {
        case GUI_MAIN:
          currGui = new GuiMain();
          break;
        case GUI_EDITOR:
          currGui = new GuiEditor();
          break;
        // case GUI_TX_WARNING:
        //   currGui = new GuiTXWarning();
        //   break;
        case GUI_CHEATS:
          currGui = new GuiCheats();
          break;
        case GUI_GUIDE:
          currGui = new GuiGuide();
          break;
        case GUI_ABOUT:
          currGui = new GuiAbout();
          break;
        case GUI_CHOOSE_MISSION:
          currGui = new GuiChooseMission();
          break;
        case GUI_MEMORY_EDITOR:
          currGui = new GuiMemoryEditor();
          break;
        case GUI_MORE:
          currGui = new GuiMore();
          break;
        case GUI_CHEATDB:
          currGui = new Guicheatdb();
          break;
        case GUI_FIRST_RUN:
          currGui = new Guifirstrun();
          break;
        case GUI_Sysmodule:
          currGui = new GuiSysmodule();
          break;
        case GUI_INVALID:
          [[fallthrough]] default : break;
        }
        if (nextGuiStart == Gui::g_nextGui)
          Gui::g_nextGui = GUI_INVALID;
      } while (Gui::g_nextGui != GUI_INVALID);

      mutexUnlock(&mutexCurrGui);
    }

    if (currGui != nullptr)
    {
      currGui->draw();

      if (Gui::g_splashDisplayed)
      {
        if (inputTicker > LONG_PRESS_ACTIVATION_DELAY && (inputTicker % LONG_PRESS_DELAY) == 0)
        {
          if (Gui::g_currMessageBox != nullptr)
            Gui::g_currMessageBox->onInput(kheld);
          else if (Gui::g_currListSelector != nullptr)
            Gui::g_currListSelector->onInput(kheld);
          else
            currGui->onInput(kheld);
        }
        else if (kdown || padGetButtonsUp(&pad))
        {
          if (Gui::g_currMessageBox != nullptr)
            Gui::g_currMessageBox->onInput(kdown);
          else if (Gui::g_currListSelector != nullptr)
            Gui::g_currListSelector->onInput(kdown);
          else
            currGui->onInput(kdown);
        }
      }
    }

    if (kheld != kheldOld)
    {
      inputTicker = 0;
    }

    static HidTouchScreenState touchPosStart, touchPosCurr, touchPosOld;
    static u8 touchCount, touchCountOld;
    static bool touchHappend = false;

    HidTouchScreenState state = {0};
    hidGetTouchScreenStates(&state, 0);
    touchCount = state.count;

    if (touchCount > 0)
      hidGetTouchScreenStates(&touchPosCurr, 0);

    if (touchCount > 0 && touchCountOld == 0)
      hidGetTouchScreenStates(&touchPosStart, 0);

    if (abs(static_cast<s16>(touchPosStart.touches[0].x - touchPosCurr.touches[0].x)) < 10 && abs(static_cast<s16>(touchPosStart.touches[0].y - touchPosCurr.touches[0].y)) < 10)
    {
      if (touchCount == 0 && touchCountOld > 0)
      {
        touchHappend = true;

        if (Gui::g_currMessageBox != nullptr)
          Gui::g_currMessageBox->onTouch(touchPosCurr.touches[0]);
        else if (Gui::g_currListSelector != nullptr)
          Gui::g_currListSelector->onTouch(touchPosCurr.touches[0]);
        else
          currGui->onTouch(touchPosCurr.touches[0]);
      }
    }
    else if (touchCount > 0)
    {
      if (Gui::g_currMessageBox != nullptr)
        Gui::g_currMessageBox->onGesture(touchPosStart, touchPosCurr, false);
      else if (Gui::g_currListSelector != nullptr)
        Gui::g_currListSelector->onGesture(touchPosStart, touchPosCurr, false);
      else
        currGui->onGesture(touchPosStart, touchPosCurr, false);
    }

    if (touchCount == 0 && touchCountOld > 0 && !touchHappend)
    {
      if (Gui::g_currMessageBox != nullptr)
        Gui::g_currMessageBox->onGesture(touchPosStart, touchPosCurr, true);
      else if (Gui::g_currListSelector != nullptr)
        Gui::g_currListSelector->onGesture(touchPosStart, touchPosCurr, true);
      else
        currGui->onGesture(touchPosStart, touchPosCurr, true);
    }

    touchCountOld = touchCount;
    touchPosOld = touchPosCurr;
    touchHappend = false;

    kheldOld = kheld;

    if (Gui::g_requestExit)
    {
      if (Gui::g_currMessageBox == nullptr)
        break;
    }
  }

  updateThreadRunning = false;

  updateThread.join();

  delete[] g_edizonPath;

  for (auto it = Title::g_titles.begin(); it != Title::g_titles.end(); it++)
    delete it->second;

  for (auto it = Account::g_accounts.begin(); it != Account::g_accounts.end(); it++)
    delete it->second;

  Title::g_titles.clear();
  Account::g_accounts.clear();

  if (currGui != nullptr)
    delete currGui;

  framebufferClose(&Gui::g_fb_obj);

  serviceExit();

  return 0;
}
