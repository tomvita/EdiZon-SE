#include "guis/gui_sysmodule.hpp"
#include "unzip1.hpp"
#include "guis/button2.hpp"
#include "helpers/config.hpp"
// #include "ui_elements/button.hpp"
#include <switch.h>
#include <stdio.h>
#include <dirent.h>
#include <sstream>
#include <fstream>
#include <utility>

// #include "guis/list_selector.hpp"
// #include "guis/message_box.hpp"

#include "json.hpp"


#define CONTENTS_PATH "/atmosphere/contents/"


//Rows of buttons per column
#define ROWS 4

using json = nlohmann::json;

// extern "C" {
//   #include "pm_dmnt.h"
//   #include "hid_extra.h"
// }
//

static bool anyModulesPresent = false;

GuiSysmodule::GuiSysmodule() : Gui() {
  pmshellInitialize();
  pmdmntInitialize(); 

  anyModulesPresent = false;

  std::ifstream configFile("sdmc:/switch/HekateToolbox/config.json");
  json configJson;

  if (!configFile.fail()) {
    try {
      configFile >> configJson;
    } catch(json::parse_error& e) {}
  }

  DIR *contents_dir = opendir(CONTENTS_PATH);
  if (contents_dir != nullptr) {
    json toolboxJson;
    std::string toolbox_string;
    std::ofstream toolbox_file;
    struct dirent *ent;
    while ((ent = readdir(contents_dir)) != nullptr) {
      std::ifstream sysconfig(CONTENTS_PATH + std::string(ent->d_name) + "/toolbox.json");
      if (!sysconfig.fail()) {
        try {
          sysconfig >> toolboxJson;
          configJson["sysmodules"].push_back(toolboxJson);
        } catch(json::parse_error& e) {}
      }
      else if ( strcmp(ent->d_name, "0100000000000008") == 0 || strcmp(ent->d_name, "010000000000000D") == 0 || strcmp(ent->d_name, "010000000000002B") == 0 || strcmp(ent->d_name, "0100000000000032") == 0 ||
                strcmp(ent->d_name, "0100000000000034") == 0 || strcmp(ent->d_name, "0100000000000036") == 0 || strcmp(ent->d_name, "0100000000000037") == 0 || strcmp(ent->d_name, "010000000000003C") == 0 ||
                strcmp(ent->d_name, "0100000000000042") == 0 || strcmp(ent->d_name, "0100000000001013") == 0 || strcmp(ent->d_name, "0100000000001010") == 0 ) continue;
      else { 
        try {
          std::stringstream path, path2;
          path << CONTENTS_PATH << std::string(ent->d_name) << "/exefs.nsp";
          if (access(path.str().c_str(), F_OK) == -1)
            continue;
          toolboxJson["name"] = std::string(ent->d_name);
          toolboxJson["tid"] = std::string(ent->d_name);
          toolboxJson["requires_reboot"] = true;
          toolbox_string = toolboxJson.dump(4);
          path2 << CONTENTS_PATH << std::string(ent->d_name) << "/toolbox.json";
          toolbox_file.open(path2.str());
          toolbox_file << toolbox_string << "\n";
          toolbox_file.close();
          configJson["sysmodules"].push_back(toolboxJson);
        } catch (json::parse_error &e) {
          continue;
        }
      }
    }
  }
  closedir(contents_dir);

  for (auto sysmodule : configJson["sysmodules"]) {
    try {
      std::stringstream path;
      path << CONTENTS_PATH << sysmodule["tid"].get<std::string>() << "/exefs.nsp";

      if (access(path.str().c_str(), F_OK) == -1) continue;
      
      this->m_sysmodules.insert(std::make_pair(sysmodule["tid"].get<std::string>(), (sysModule_t){ sysmodule["name"].get<std::string>(), sysmodule["tid"].get<std::string>(), sysmodule["requires_reboot"].get<bool>() }));
      
      u64 sysmodulePid = 0;
      pmdmntGetProcessId(&sysmodulePid, std::stoul(sysmodule["tid"].get<std::string>(), nullptr, 16));

      if (sysmodulePid > 0)
        this->m_runningSysmodules.insert(sysmodule["tid"].get<std::string>());
      
    } catch(json::parse_error &e) {
      continue;
    }
  }

  u16 xOffset = 0, yOffset = 0;
  s32 cnt = 0;

  for (auto &sysmodule : this->m_sysmodules) {
    FILE *exefs = fopen(std::string(CONTENTS_PATH + sysmodule.second.titleID + "/exefs.nsp").c_str(), "r");

    if (exefs == nullptr)
      continue;

    fclose(exefs);

    anyModulesPresent = true;

   new Button2(100 + xOffset, 250 + yOffset, 500, 80, [&](Gui *gui, u16 x, u16 y, bool *isActivated){
      gui->drawTextAligned(font20, x + 37, y + 23, currTheme.textColor, sysmodule.second.name.c_str(), ALIGNED_LEFT);
      gui->drawTextAligned(font20, x + 420, y + 23, this->m_runningSysmodules.find(sysmodule.first) != this->m_runningSysmodules.end() ? currTheme.selectedColor : Gui::makeColor(0xB8, 0xBB, 0xC2, 0xFF), this->m_runningSysmodules.find(sysmodule.first) != this->m_runningSysmodules.end() ? "On" : "Off", ALIGNED_LEFT);
    }, [&](u32 kdown, bool *isActivated){
      if (kdown & HidNpadButton_A) {
        u64 pid;
        u64 tid = std::stol(sysmodule.first.c_str(), nullptr, 16);

        mkdir(std::string(CONTENTS_PATH + sysmodule.second.titleID + "/flags").c_str(), 777);
        std::stringstream path;
        path << CONTENTS_PATH << sysmodule.first << "/flags/boot2.flag";


        if (this->m_runningSysmodules.find(sysmodule.first) != this->m_runningSysmodules.end()) {
          if (!sysmodule.second.requiresReboot) {
            pmshellTerminateProgram(tid);
          } else {
            (new MessageBox("This sysmodule requires a reboot to fully work. \n Please restart your console in order use it.", MessageBox::OKAY))->show();
          }

          remove(path.str().c_str());
        }
        else {
          if (sysmodule.second.requiresReboot) {
            (new MessageBox("This sysmodule requires a reboot to fully work. \n Please restart your console in order use it.", MessageBox::OKAY))->show();
            FILE *fptr = fopen(path.str().c_str(), "wb+");
            if (fptr != nullptr) fclose(fptr);
          } else {
            NcmProgramLocation programLocation{
              .program_id = tid,
              .storageID = NcmStorageId_None,
            };
            if (R_SUCCEEDED(pmshellLaunchProgram(0, &programLocation, &pid))) {
              FILE *fptr = fopen(path.str().c_str(), "wb+");
              if (fptr != nullptr) fclose(fptr);
            }
          }
        }

        pid = 0;
        pmdmntGetProcessId(&pid, tid);

        if (!sysmodule.second.requiresReboot) {
          if (pid != 0)
            this->m_runningSysmodules.insert(sysmodule.first);
          else
            this->m_runningSysmodules.erase(sysmodule.first);     
        } else {
          if (access(path.str().c_str(), F_OK) == 0)
            this->m_runningSysmodules.insert(sysmodule.first);
          else
            this->m_runningSysmodules.erase(sysmodule.first);    
        }
      }
    }, {
        (cnt % ROWS) != 0    ? (cnt - 1) : -1, //UP
        (cnt % ROWS) != ROWS-1    ? (cnt + 1) : -1, //DOWN
        (cnt - ROWS), //LEFT
        (cnt + ROWS)  //RIGHT
      }, false, 
    [&]() -> bool { return true; });
  

    yOffset += 100;

    if (yOffset == ROWS * 100) {
      xOffset += 550;
      yOffset = 0;
    }

    cnt++;
  }
  Button2::select(selection);
}

GuiSysmodule::~GuiSysmodule() {
  selection = Button2::getSelectedIndex();
  pmshellExit();
  pmdmntExit();

  Button2::g_buttons.clear();
}

void GuiSysmodule::update() {
  Gui::update();
}

void GuiSysmodule::draw() {
  Gui::beginDraw();
  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, currTheme.backgroundColor);
  Gui::drawRectangle((u32)((Gui::g_framebuffer_width - 1220) / 2), 87, 1220, 1, currTheme.textColor);
  Gui::drawRectangle((u32)((Gui::g_framebuffer_width - 1220) / 2), Gui::g_framebuffer_height - 73, 1220, 1, currTheme.textColor);
  Gui::drawTextAligned(font24, Gui::g_framebuffer_width / 2, 20, currTheme.textColor, "\uE130    Sysmodule manager", ALIGNED_CENTER);
  // Gui::drawTextAligned(font24, 70, 58, currTheme.textColor, "        Hekate Toolbox", ALIGNED_LEFT);

  // if (hidMitmInstalled())
  //   Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 25, currTheme.textColor, "\uE0E2 Key configuration     \uE0E1 Back     \uE0E0 OK", ALIGNED_RIGHT);
  // else
  // Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 51, currTheme.textColor, "\uE0E1 Back     \uE0E0 OK", ALIGNED_RIGHT);
  {
    // FILE *exefs = fopen(std::string(CONTENTS_PATH "0100000000001013/exefs.nsp").c_str(), "r");
    // if ((exefs == nullptr) && (access(CONTENTS_PATH "0100000000001013/exefs.nsp1", F_OK) == 0))
    //   Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 400, Gui::g_framebuffer_height - 51, currTheme.textColor, "\uE0E2 Make profile launch EdiZon SE", ALIGNED_RIGHT);
    // else {
    //   Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 400, Gui::g_framebuffer_height - 51, currTheme.textColor, "\uE0E3 Make profile launch profile", ALIGNED_RIGHT);
    //   fclose(exefs);
    // }
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 51, currTheme.textColor, "Make Profile launch  \uE0E2 EdiZon SE  \uE0E3 Profile  \uE0E4 Breeze", ALIGNED_RIGHT);
    if (!(access("/atmosphere/config/system_settings.ini", F_OK) == 0))
      Gui::drawTextAligned(font20, 50, Gui::g_framebuffer_height - 51, currTheme.textColor, "\uE0F0 Create system_settings.ini", ALIGNED_LEFT);
  }
  if (anyModulesPresent)
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, 100, currTheme.textColor, "Enable as little background services (sysmodules) as possible for maximum compatibility with \n games. If you encounter problem try disable all of them to see if it helps to improve stability. \n Do not enable tesla if you are on HOS11+ when running Edizon SE.", ALIGNED_CENTER);
  else
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, 550, currTheme.textColor, "You currently don't have any supported sysmodules installed. To use this \n feature, please install any supported sysmodule as an NSP.", ALIGNED_CENTER);
    

  for(Button2 *btn : Button2::g_buttons)
    btn->draw(this);

  Gui::endDraw();
}

void GuiSysmodule::onInput(u32 kdown) {
  for(Button2 *btn : Button2::g_buttons) {
    if (btn->isSelected())
      if (btn->onInput(kdown)) break;
  }
  
  if (kdown & HidNpadButton_B)
  {
    Config::readConfig();
    if (Config::getConfig()->easymode)
      Gui::g_nextGui = GUI_CHEATS;
    else
      Gui::g_nextGui = GUI_CHOOSE_MISSION;
  }
  else if ((kdown & HidNpadButton_X) && (access("sdmc:/switch/edizon/profile.zip", F_OK) == 0))
  {
    // try
    // {
    //   if (access(CONTENTS_PATH "0100000000001013/exefs.nsp", F_OK) == -1)
    //     rename(CONTENTS_PATH "0100000000001013/exefs.nsp1", CONTENTS_PATH "0100000000001013/exefs.nsp");
    // }
    // catch (json::parse_error &e)
    // {
    // }
    inst::zip::extractFile("sdmc:/switch/edizon/profile.zip", "sdmc:/");
    (new Snackbar("Profile set to launch EdiZon SE"))->show();
  }
  else if ((kdown & HidNpadButton_Y) && (access(CONTENTS_PATH "0100000000001013/exefs.nsp", F_OK) == 0))
  {
    try
    {
        remove(CONTENTS_PATH "0100000000001013/exefs.nsp");
    }
    catch (json::parse_error &e)
    {
    }
    (new Snackbar("Profile set to launch Profile"))->show();
  }
  else if ((kdown & HidNpadButton_L) && (access("sdmc:/switch/breeze/profile.zip", F_OK) == 0))
  {
    inst::zip::extractFile("sdmc:/switch/breeze/profile.zip", "sdmc:/");
    (new Snackbar("Profile set to launch Breeze"))->show();
  }
  else if ((kdown & HidNpadButton_Minus) && (!(access("/atmosphere/config/system_settings.ini", F_OK) == 0)))
  {
    const char inst1[] = "[atmosphere]\n";
    const char inst2[] = "dmnt_cheats_enabled_by_default = u8!0x0\n";
    const char inst3[] = "dmnt_always_save_cheat_toggles = u8!0x1\n";
    FILE *fp1 = fopen("/atmosphere/config/system_settings.ini", "wb");
    fwrite(inst1, 1, sizeof(inst1) - 1, fp1);
    fwrite(inst2, 1, sizeof(inst2) - 1, fp1);
    fwrite(inst3, 1, sizeof(inst3) - 1, fp1);
    fclose(fp1);
    (new MessageBox("System_settings.ini created. \n Cheat code will be default to off and cheat enable \n will be saved when you quite the game. \n Requires a reboot to take effect. ", MessageBox::OKAY))->show();
    // Gui::g_requestExit = true;
  }

  // if (hidMitmInstalled() && kdown & HidNpadButton_X)
  //   Gui::g_nextGui = GUI_HID_MITM;

}

void GuiSysmodule::onTouch(HidTouchState &touch) {
  for(Button2 *btn : Button2::g_buttons) {
    btn->onTouch(touch);
  }
}

void GuiSysmodule::onGesture(HidTouchScreenState startPosition, HidTouchScreenState endPosition, bool finish) {

}
