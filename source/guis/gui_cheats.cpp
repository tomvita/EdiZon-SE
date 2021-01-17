#include "guis/gui_cheats.hpp"

#include <sstream>
#include <limits>
#include <utility>

#include <bits/stdc++.h>
#include <thread>

#include "helpers/util.h"
#include "helpers/config.hpp"
#include "edizon_logo_bin.h"
#include "lz.h"
#include "version.h"
#include "unzipper.h"
// #define checkheap
// #define printpointerchain
#define MAX_BUFFER_SIZE 0x1000000 // increase size for faster speed
#define STARTTIMER \
  time_t unixTime1 = time(NULL);
#define ENDTIMER \
  printf(" Time used is %ld \n", time(NULL) - unixTime1);
#define R_UNLESS(expr, res)                   \
  ({                                          \
    if (!(expr))                              \
    {                                         \
      return static_cast<::ams::Result>(res); \
    }                                         \
  })
static const std::vector<std::string> dataTypes = {"u8", "s8", "u16", "s16", "u32", "s32", "u64", "s64", "f32", "f64", "ptr", "  "};
static const std::vector<u8> dataTypeSizes = {1, 1, 2, 2, 4, 4, 8, 8, 4, 8, 8};
static const std::vector<std::string> buttonNames = {"\uE0A0 ", "\uE0A1 ", "\uE0A2 ", "\uE0A3 ", "\uE0C4 ", "\uE0C5 ", "\uE0A4 ", "\uE0A5 ", "\uE0A6 ", "\uE0A7 ", "\uE0B3 ", "\uE0B4 ", "\uE0B1 ", "\uE0AF ", "\uE0B2 ", "\uE0B0 ", "\uE091 ", "\uE092 ", "\uE090 ", "\uE093 ", "\uE145 ", "\uE143 ", "\uE146 ", "\uE144 "};
// static const std::vector<std::string> buttonNames = {"\uE0A0", "\uE0A1", "\uE0A2", "\uE0A3", "\uE0C4", "\uE0C5", "\uE0A4", "\uE0A5", "\uE0A6", "\uE0A7", "\uE0B3", "\uE0B4", "\uE0B1", "\uE0AF", "\uE0B2", "\uE0B0"};
// static const std::vector<std::string> buttonNames = {"\uE0E0", "\uE0E1", "\uE0E2", "\uE0E3", "\uE104", "\uE105", "\uE0E4", "\uE0E5", "\uE0E6", "\uE0E7", "\uE0EF", "\uE0F0", "\uE0ED", "\uE0EB", "\uE0EE", "\uE0EC"};
static const std::vector<u32> buttonCodes = {0x80000001,
                                             0x80000002,
                                             0x80000004,
                                             0x80000008,
                                             0x80000010,
                                             0x80000020,
                                             0x80000040,
                                             0x80000080,
                                             0x80000100,
                                             0x80000200,
                                             0x80000400,
                                             0x80000800,
                                             0x80001000,
                                             0x80002000,
                                             0x80004000,
                                             0x80008000,
                                             0x80010000,
                                             0x80020000,
                                             0x80040000,
                                             0x80080000,
                                             0x80100000,
                                             0x80200000,
                                             0x80400000,
                                             0x80800000};
// 0x81000000,
// 0x82000000};
static const std::vector<s128> dataTypeMaxValues = {std::numeric_limits<u8>::max(), std::numeric_limits<s8>::max(), std::numeric_limits<u16>::max(), std::numeric_limits<s16>::max(), std::numeric_limits<u32>::max(), std::numeric_limits<s32>::max(), std::numeric_limits<u64>::max(), std::numeric_limits<s64>::max(), std::numeric_limits<s32>::max(), std::numeric_limits<s64>::max(), std::numeric_limits<u64>::max()};
static const std::vector<s128> dataTypeMinValues = {std::numeric_limits<u8>::min(), std::numeric_limits<s8>::min(), std::numeric_limits<u16>::min(), std::numeric_limits<s16>::min(), std::numeric_limits<u32>::min(), std::numeric_limits<s32>::min(), std::numeric_limits<u64>::min(), std::numeric_limits<s64>::min(), std::numeric_limits<s32>::min(), std::numeric_limits<s64>::min(), std::numeric_limits<u64>::min()};

static std::string titleNameStr, tidStr, pidStr, buildIDStr, cheatpathStr;

static u32 cheatListOffset = 0;

static bool _isAddressFrozen(uintptr_t);
static std::string _getAddressDisplayString(u64, Debugger *debugger, searchType_t searchType);
static std::string _getValueDisplayString(searchValue_t searchValue, searchType_t searchType);
static searchValue_t _get_entry(searchValue_t value, searchType_t type);
// static void _moveLonelyCheats(u8 *buildID, u64 titleID);
static bool _wrongCheatsPresent(u8 *buildID, u64 titleID);
static bool compareentry(MultiSearchEntry_t e1, MultiSearchEntry_t e2);
static bool comparefromto(fromto_t e1, fromto_t e2);
GuiCheats::GuiCheats() : Gui()
{
  if (Config::getConfig()->deletebookmark)
  {
    if (Config::getConfig()->separatebookmark)
    remove((m_edizon_dir + "/memdumpbookmark.dat").c_str());
    else
    remove(EDIZON_DIR "/memdumpbookmark.dat");
  }
  // Check if dmnt:cht is running and we're not on sxos
  m_sysmodulePresent = isServiceRunning("dmnt:cht") && !(isServiceRunning("tx") && !isServiceRunning("rnx"));

  // dmntchtForceOpenCheatProcess();
  // dmntchtPauseCheatProcess();
  m_debugger = new Debugger();
  if (m_sysmodulePresent)
    dmntchtInitialize();
  if (!autoattachcheck())
    m_debugger->attachToProcess();
  if (!m_debugger->m_dmnt) { m_sysmodulePresent = true;  }
  // printf(" envIsSyscallHinted(0x60) = %d \n",envIsSyscallHinted(0x60));
  // printf("init debugger success m_rc = %x m_debugHandle = %x m_dmnt = %x\n",(u32) m_debugger->m_rc, m_debugger->m_debugHandle, m_debugger->m_dmnt);
  m_cheats = nullptr;
  m_memoryDump = nullptr;
  // start mod bookmark;
  m_memoryDumpBookmark = nullptr;
  m_memoryDump1 = nullptr;
  m_pointeroffsetDump = nullptr;

  m_searchValue[0]._u64 = 0;
  m_searchValue[1]._u64 = 0;
  m_searchType = SEARCH_TYPE_UNSIGNED_16BIT;
  m_searchMode = SEARCH_MODE_EQ;
  m_searchRegion = SEARCH_REGION_HEAP_AND_MAIN;
  freeze();
  m_cheatCnt = 0;

  // if (!m_sysmodulePresent)
  //   return;


  DmntCheatProcessMetadata metadata;
  if (m_debugger->m_dmnt)
    dmntchtGetCheatProcessMetadata(&metadata);
  else
  {
    LoaderModuleInfo proc_modules[2] = {};
    s32 num_modules=2;
    ldrDmntInitialize();
    Result rc = ldrDmntGetProcessModuleInfo(m_debugger->getRunningApplicationPID(), proc_modules, std::size(proc_modules), &num_modules);
    ldrDmntExit();
    const LoaderModuleInfo *proc_module = nullptr;
    if (num_modules == 2)
    {
      proc_module = &proc_modules[1];
    }
    else if (num_modules == 1)
    {
      proc_module = &proc_modules[0];
    }
    if (rc != 0)
      printf("num_modules = %x, proc_module->base_address = %lx , pid = %ld, rc = %x\n ", num_modules, proc_module->base_address, m_debugger->getRunningApplicationPID(), rc);
    metadata.main_nso_extents.base = proc_module->base_address;
    metadata.main_nso_extents.size = proc_module->size;
    std::memcpy(metadata.main_nso_build_id, proc_module->build_id, sizeof((metadata.main_nso_build_id)));
  };

  m_addressSpaceBaseAddr = metadata.address_space_extents.base;
  m_addressSpaceSize = metadata.address_space_extents.size;
  m_heapBaseAddr = metadata.heap_extents.base;
  m_mainBaseAddr = metadata.main_nso_extents.base;
  m_EditorBaseAddr = m_heapBaseAddr;

  m_heapSize = metadata.heap_extents.size;
  m_mainSize = metadata.main_nso_extents.size;

  if (m_mainBaseAddr < m_heapBaseAddr) // not used but have to move lower for it to be correct
  {
    m_low_main_heap_addr = m_mainBaseAddr;
    m_high_main_heap_addr = m_heapEnd;
  }
  else
  {
    m_low_main_heap_addr = m_heapBaseAddr;
    m_high_main_heap_addr = m_mainend;
  }

  memcpy(m_buildID, metadata.main_nso_build_id, 0x20);

  _moveLonelyCheats(m_buildID, m_debugger->getRunningApplicationTID());
  // reloadcheatsfromfile(m_buildID, m_debugger->getRunningApplicationTID());
  // dumpcodetofile();
  iconloadcheck();
  Config::getConfig()->lasttitle = m_debugger->getRunningApplicationTID();
  Config::writeConfig();

  dmntchtGetCheatCount(&m_cheatCnt);

  if (m_cheatCnt > 0)
  {
    m_cheats = new DmntCheatEntry[m_cheatCnt];
    m_cheatDelete = new bool[m_cheatCnt];
    for (u64 i = 0; i < m_cheatCnt; i++)
      m_cheatDelete[i] = false;
    dmntchtGetCheats(m_cheats, m_cheatCnt, 0, &m_cheatCnt);
  }
  else if (_wrongCheatsPresent(m_buildID, m_debugger->getRunningApplicationTID()))
    m_cheatsPresent = true;

  u64 frozenAddressCnt = 0;
  dmntchtGetFrozenAddressCount(&frozenAddressCnt);

  if (frozenAddressCnt != 0)
  {
    DmntFrozenAddressEntry frozenAddresses[frozenAddressCnt];
    dmntchtGetFrozenAddresses(frozenAddresses, frozenAddressCnt, 0, nullptr);

    for (u16 i = 0; i < frozenAddressCnt; i++)
      m_frozenAddresses.insert({frozenAddresses[i].address, frozenAddresses[i].value.value});
  }

#ifdef checkheap
  printf("m_heapBaseAddr = %lx m_heapSize = %lx m_mainBaseAddr + m_mainSize = %lx\n", m_heapBaseAddr, m_heapSize, m_mainBaseAddr + m_mainSize);
#endif
  MemoryInfo meminfo = {0};
  u64 lastAddr = 0;
  m_heapBaseAddr = 0;
  m_heapSize = 0;
  m_heapEnd = 0;
  m_mainend = 0;
  u32 mod = 0;

  do
  {
    lastAddr = meminfo.addr;
    meminfo = m_debugger->queryMemory(meminfo.addr + meminfo.size);

    if (meminfo.type == MemType_Heap)
    {
      if (m_heapBaseAddr == 0)
      {
        m_heapBaseAddr = meminfo.addr;
      }
      m_heapSize += meminfo.size;              // not going to use this but calculate this anyway this don't match for some game
      m_heapEnd = meminfo.addr + meminfo.size; // turns out that m_heapEnd may not be same as m_heapBaseAddr + m_heapSize
    }

    if (meminfo.type == MemType_CodeMutable && mod == 2)
    {
      m_mainend = meminfo.addr + meminfo.size; // same for m_mainend not the same as m_mainBaseAddr + m_mainSize;
    }

    if (meminfo.type == MemType_CodeStatic && meminfo.perm == Perm_Rx)
    {
      if (mod == 1)
        m_mainBaseAddr = meminfo.addr;
      mod++;
    }

    m_memoryInfo.push_back(meminfo);
  } while (lastAddr < meminfo.addr + meminfo.size);

if (!(m_debugger->m_dmnt)){

  m_addressSpaceBaseAddr = metadata.main_nso_extents.base;
  m_addressSpaceSize = metadata.main_nso_extents.size;
  m_EditorBaseAddr = m_heapBaseAddr;
  m_heapSize = m_heapEnd-m_heapBaseAddr;
  m_mainSize = m_mainend-m_mainBaseAddr;
}

#ifdef checkheap
  printf("m_heapBaseAddr = %lx m_heapSize = %lx m_heapEnd = %lx m_mainend =%lx\n", m_heapBaseAddr, m_heapSize, m_heapEnd, m_mainend);
  // for some game heap info was very far off
#endif

  if (m_mainend < 0xFFFFFFFF && m_heapEnd < 0xFFFFFFFF)
    m_32bitmode = true;
  else
    m_32bitmode = false;

  for (MemoryInfo meminfo : m_memoryInfo)
  {
    // if (m_mainBaseAddr == 0x00 && (meminfo.type == MemType_CodeStatic)) // wasn't executed since it isn't 0x00 but this code is getting wrong address
    //   m_mainBaseAddr = meminfo.addr;

    for (u64 addrOffset = meminfo.addr; addrOffset < meminfo.addr + meminfo.size; addrOffset += 0x20000000)
    {
      switch (meminfo.type)
      {
      case MemType_CodeStatic:
      case MemType_CodeMutable:
        m_memory[addrOffset / 0x20000000] = Gui::makeColor(0xFF, 0x00, 0x00, 0xFF);
        break;
      case MemType_SharedMem:
        m_memory[addrOffset / 0x20000000] = Gui::makeColor(0x00, 0xFF, 0x00, 0xFF);
        break;
      case MemType_Heap:
        m_memory[addrOffset / 0x20000000] = Gui::makeColor(0x00, 0x00, 0xFF, 0xFF);
        break;
      case MemType_KernelStack:
      case MemType_ThreadLocal:
        m_memory[addrOffset / 0x20000000] = Gui::makeColor(0xFF, 0xFF, 0x00, 0xFF);
        break;
      case MemType_Unmapped:
        break;
      default:
        m_memory[addrOffset / 0x20000000] = Gui::makeColor(0x80, 0x80, 0x80, 0xFF);
        break;
      }
    }
  }
  //BM Begin title id and icon init

  // size_t appControlDataSize = 0;
  // NacpLanguageEntry *languageEntry = nullptr;
  // std::memset(&appControlData, 0x00, sizeof(NsApplicationControlData));
  // nsGetApplicationControlData(NsApplicationControlSource_Storage, m_debugger->getRunningApplicationTID(), &appControlData, sizeof(NsApplicationControlData), &appControlDataSize);
  // nacpGetLanguageEntry(&appControlData.nacp, &languageEntry);
  // m_titleName = std::string(languageEntry->name);
  // m_versionString = std::string(appControlData.nacp.display_version);

  //BM Begin pointer search init

  // MemoryDump *m_pointeroffsetDump = new MemoryDump(EDIZON_DIR "/pointerdump1.dat", DumpType::POINTER, false);
  // m_pointeroffsetDump->setPointerSearchParams(m_max_depth, m_numoffset, m_max_range, m_buildID);
  if ((m_heapEnd - m_heapBaseAddr) > 0xFFFFFFFF ) m_64bit_offset = true;
  load_meminfos();
  load_multisearch_setup();
  m_memoryDump = new MemoryDump(EDIZON_DIR "/memdump1.dat", DumpType::UNDEFINED, false);
  // start mod make list of memory found toggle between current find and bookmark
  m_memoryDumpBookmark = new MemoryDump(EDIZON_DIR "/memdumpbookmark.dat", DumpType::ADDR, false);
  // end mod

  if (m_debugger->getRunningApplicationPID() == 0 || m_memoryDump->getDumpInfo().heapBaseAddress != m_heapBaseAddr)
  {
    m_memoryDump->clear();

    remove(EDIZON_DIR "/memdump2.dat");
    remove(EDIZON_DIR "/memdump3.dat");
    remove(EDIZON_DIR "/BMDump.dat");

    // m_searchType = SEARCH_TYPE_NONE;
    // m_searchRegion = SEARCH_REGION_NONE;
    m_searchType = SEARCH_TYPE_UNSIGNED_16BIT;
    m_searchMode = SEARCH_MODE_EQ;
    m_searchRegion = SEARCH_REGION_HEAP_AND_MAIN;
    m_searchValue[0]._u64 = 0;
    m_searchValue[1]._u64 = 0;
  }
  else
  {
    m_searchType = m_memoryDump->getDumpInfo().searchDataType;
    m_searchRegion = m_memoryDump->getDumpInfo().searchRegion;
    m_searchMode = m_memoryDump->getDumpInfo().searchMode;
    m_use_range = m_memoryDump->getDumpInfo().use_range;
    Config::readConfig();
    if (Config::getConfig()->disablerangeonunknown) m_use_range = false;
    m_searchValue[0] = m_memoryDump->getDumpInfo().searchValue[0];
    m_searchValue[1] = m_memoryDump->getDumpInfo().searchValue[1];
  }

  m_memoryDump->setBaseAddresses(m_addressSpaceBaseAddr, m_heapBaseAddr, m_mainBaseAddr, m_heapSize, m_mainSize);

  // start mod bookmark BM2
  std::stringstream filebuildIDStr;
  {
    std::stringstream buildIDStr;
    for (u8 i = 0; i < 8; i++)
      buildIDStr << std::nouppercase << std::hex << std::setfill('0') << std::setw(2) << (u16)m_buildID[i];
    // buildIDStr.str("attdumpbookmark");
    if (Config::getConfig()->separatebookmark)
      filebuildIDStr << m_edizon_dir + "/" << buildIDStr.str() << ".dat";
    else
      filebuildIDStr << EDIZON_DIR "/" << buildIDStr.str() << ".dat";
    m_PCDump_filename << EDIZON_DIR "/" << buildIDStr.str() << ".dmp1";
  }
  if (Config::getConfig()->deletebookmark)
  {
    std::stringstream savebookmark;
    savebookmark << filebuildIDStr.str() << "1";
    REPLACEFILE(filebuildIDStr.str().c_str(), savebookmark.str().c_str());
    Config::getConfig()->deletebookmark = false;
    Config::writeConfig();
  }
  m_AttributeDumpBookmark = new MemoryDump(filebuildIDStr.str().c_str(), DumpType::ADDR, false);
  if (m_debugger->getRunningApplicationPID() == 0 || m_memoryDumpBookmark->getDumpInfo().heapBaseAddress != m_heapBaseAddr)
  // is a different run need to refresh the list
  {
    // delete m_AttributeDumpBookmark;
    // rename(filebuildIDStr.str().c_str(), EDIZON_DIR "/tempbookmark.dat");
    MemoryDump *tempdump;
    tempdump = new MemoryDump(EDIZON_DIR "/tempbookmark.dat", DumpType::ADDR, true);
    m_memoryDumpBookmark->clear();
    delete m_memoryDumpBookmark;
    m_memoryDumpBookmark = new MemoryDump(EDIZON_DIR "/memdumpbookmark.dat", DumpType::ADDR, true);

    if (m_AttributeDumpBookmark->size() > 0)
    {
      bookmark_t bookmark;
      u64 address;
      for (u64 i = 0; i < m_AttributeDumpBookmark->size(); i += sizeof(bookmark_t))
      {
        m_AttributeDumpBookmark->getData(i, (u8 *)&bookmark, sizeof(bookmark_t));
        if (bookmark.heap)
        {
          address = bookmark.offset + m_heapBaseAddr;
        }
        else
        {
          address = bookmark.offset + m_mainBaseAddr;
        }
        if (bookmark.deleted)
          continue; // don't add deleted bookmark
        // check memory before adding
        MemoryInfo meminfo;
        meminfo = m_debugger->queryMemory(address);
        if (meminfo.perm == Perm_Rw)
        {
          m_memoryDumpBookmark->addData((u8 *)&address, sizeof(u64));
          tempdump->addData((u8 *)&bookmark, sizeof(bookmark_t));
        }
        else
        {
          m_memoryDumpBookmark->addData((u8 *)&m_heapBaseAddr, sizeof(u64));
          tempdump->addData((u8 *)&bookmark, sizeof(bookmark_t));
        }
      }
    }
    tempdump->setBaseAddresses(m_addressSpaceBaseAddr, m_heapBaseAddr, m_mainBaseAddr, m_heapSize, m_mainSize);
    tempdump->flushBuffer();
    delete tempdump;

    m_AttributeDumpBookmark->clear();
    delete m_AttributeDumpBookmark;

    // remove(filebuildIDStr.str().c_str());
    // while (access(filebuildIDStr.str().c_str(), F_OK) == 0)
    // {
    //   printf("waiting for delete\n");
    // }
    // rename(EDIZON_DIR "/tempbookmark.dat", filebuildIDStr.str().c_str());
    // while (access(filebuildIDStr.str().c_str(), F_OK) != 0)
    // {
    //   printf("waiting for rename\n");
    // }
    REPLACEFILE(EDIZON_DIR "/tempbookmark.dat", filebuildIDStr.str().c_str());

    m_AttributeDumpBookmark = new MemoryDump(filebuildIDStr.str().c_str(), DumpType::ADDR, false);
    // m_AttributeDumpBookmark = new MemoryDump(filebuildIDStr.str().c_str(), DumpType::ADDR, true);

    // if (tempdump->size() > 0) // create new bookmark list from past
    // {

    //   u64 offset = 0;
    //   u64 bufferSize = MAX_BUFFER_SIZE % sizeof(bookmark_t) * sizeof(bookmark_t); // need to be multiple of
    //   u8 *buffer = new u8[bufferSize];

    //   while (offset < tempdump->size())
    //   {
    //     if (tempdump->size() - offset < bufferSize)
    //       bufferSize = tempdump->size() - offset;
    //     tempdump->getData(offset, buffer, bufferSize);
    //     bookmark_t bookmark;
    //     u64 address;
    //     for (u64 i = 0; i < bufferSize; i += sizeof(bookmark_t))
    //     {
    //       memcpy(&bookmark, buffer + i, sizeof(bookmark_t));
    //       if (bookmark.heap)
    //       {
    //         address = bookmark.offset + m_heapBaseAddr;
    //       }
    //       else
    //       {
    //         address = bookmark.offset + m_mainBaseAddr;
    //       }
    //       // check memory before adding
    //       MemoryInfo meminfo;
    //       meminfo = m_debugger->queryMemory(address);
    //       if (meminfo.perm == Perm_Rw)
    //       {
    //         m_memoryDumpBookmark->addData((u8 *)&address, sizeof(u64));
    //         m_AttributeDumpBookmark->addData((u8 *)&bookmark, sizeof(bookmark_t));
    //       }
    //     }

    //     offset += bufferSize;
    //   }

    //   delete buffer;
    //   m_AttributeDumpBookmark->flushBuffer();
    //   m_memoryDumpBookmark->flushBuffer();
    // }

    // delete tempdump;
    // remove(EDIZON_DIR "/tempbookmark.dat");
    // }
    // else
    //   m_AttributeDumpBookmark = new MemoryDump(filebuildIDStr.str().c_str(), DumpType::ADDR, false);
    m_memoryDumpBookmark->setBaseAddresses(m_addressSpaceBaseAddr, m_heapBaseAddr, m_mainBaseAddr, m_heapSize, m_mainSize);
    m_AttributeDumpBookmark->setBaseAddresses(m_addressSpaceBaseAddr, m_heapBaseAddr, m_mainBaseAddr, m_heapSize, m_mainSize);
  }
  else
  {
    m_memoryDumpBookmark->setBaseAddresses(m_addressSpaceBaseAddr, m_heapBaseAddr, m_mainBaseAddr, m_heapSize, m_mainSize);
    m_AttributeDumpBookmark->setBaseAddresses(m_addressSpaceBaseAddr, m_heapBaseAddr, m_mainBaseAddr, m_heapSize, m_mainSize);
  }
  // end mod
  // updatebookmark(false, false);
  std::stringstream ss;

  // check this
  printf("%s\n", "before part");
  if ((m_debugger->getRunningApplicationTID() != 0) && HAVESAVE)
  {
    if (Title::g_titles[m_debugger->getRunningApplicationTID()]->getTitleName().length() < 24)
      ss << Title::g_titles[m_debugger->getRunningApplicationTID()]->getTitleName();
    else
      ss << Title::g_titles[m_debugger->getRunningApplicationTID()]->getTitleName().substr(0, 21) << "...";
    titleNameStr = ss.str();
    ss.str("");
  }
  else
    titleNameStr = "Unknown title name!";
  printf("%s\n", "after part");
  ss << "TID: " << std::uppercase << std::hex << std::setfill('0') << std::setw(sizeof(u64) * 2) << m_debugger->getRunningApplicationTID();
  tidStr = ss.str();
  ss.str("");

  ss << "PID: " << std::dec << m_debugger->getRunningApplicationPID();
  pidStr = ss.str();
  ss.str("");

  ss << "BID: ";
  for (u8 i = 0; i < 8; i++)
    ss << std::uppercase << std::hex << std::setfill('0') << std::setw(2) << (u16)m_buildID[i];

  buildIDStr = ss.str();

  if (m_memoryDump->size() == 0)
    m_menuLocation = CHEATS;
  if (m_cheatCnt == 0)
    m_menuLocation = CANDIDATES;

  appletSetMediaPlaybackState(true);
}

GuiCheats::~GuiCheats()
{
  unfreeze();
  // dmntchtResumeCheatProcess();
  if (m_debugger != nullptr)
  {
    delete m_debugger;
  }

  if (m_memoryDump1 != nullptr)
    delete m_memoryDump1;

  if (m_memoryDumpBookmark != nullptr)
    delete m_memoryDumpBookmark;

  if (m_cheats != nullptr)
    delete[] m_cheats;

  if (m_PC_Dump != nullptr)
    delete m_PC_Dump;

  if (m_PC_DumpM != nullptr)
    delete m_PC_DumpM;

  if (m_sysmodulePresent)
  {
    dmntchtExit();
  }

  setLedState(false);
  appletSetMediaPlaybackState(false);

  printf("%s\n", "~GuiCheats()");
}

void GuiCheats::update()
{
  Gui::update();
}
void GuiCheats::draw_easymode()
{
  static u32 splashCnt = 0;
  std::stringstream ss;
  Gui::beginDraw();
#if SPLASH_ENABLED
  if (!Gui::g_splashDisplayed)
  {
    Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, Gui::makeColor(0x5D, 0x4F, 0x4E, 0xFF));
    Gui::drawImage(Gui::g_framebuffer_width / 2 - 128, Gui::g_framebuffer_height / 2 - 128, 256, 256, edizon_logo_bin, IMAGE_MODE_BGR24);

    if (splashCnt++ >= 70)
      Gui::g_splashDisplayed = true;

    Gui::endDraw();
    return;
  }
#endif
  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, currTheme.backgroundColor);
  Gui::drawRectangle((u32)((Gui::g_framebuffer_width - 1220) / 2), Gui::g_framebuffer_height - 73, 1220, 1, currTheme.textColor);
  if (m_debugger->getRunningApplicationPID() == 0)
  {
    Gui::drawTextAligned(fontHuge, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 - 100, currTheme.textColor, "\uE12C", ALIGNED_CENTER);
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2, currTheme.textColor, "A title needs to be running in the background to use the RAM editor. \n Please launch an application and try again.", ALIGNED_CENTER);
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, "\uE0E1 Back", ALIGNED_RIGHT);
    Gui::endDraw();
    return;
  }
  m_menuLocation = CHEATS;
  {
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 51, currTheme.textColor, "\uE0F0 Check for update  \uE0E6 Page Up  \uE0E7 Page Down  \uE0E0 Cheat on/off  \uE0E1 Quit", ALIGNED_RIGHT);
  }
  Gui::drawRectangle(256, 50, Gui::g_framebuffer_width - 256, 206, currTheme.separatorColor);
  // Don't draw icon
  if ((m_debugger->getRunningApplicationTID() != 0) && HAVESAVE)
    Gui::drawImage(0, 0, 256, 256, Title::g_titles[m_debugger->getRunningApplicationTID()]->getTitleIcon(), IMAGE_MODE_RGB24);
  else
    Gui::drawRectangle(0, 0, 256, 256, Gui::makeColor(0x00, 0x00, 0xFF, 0xFF));

  Gui::drawRectangle(660, 65, 20, 20, Gui::makeColor(0xFF, 0x00, 0x00, 0xFF));  // Code
  Gui::drawRectangle(660, 85, 20, 20, Gui::makeColor(0x00, 0xFF, 0x00, 0xFF));  // Shared Memory
  Gui::drawRectangle(660, 105, 20, 20, Gui::makeColor(0x00, 0x00, 0xFF, 0xFF)); // Heap
  Gui::drawRectangle(660, 125, 20, 20, Gui::makeColor(0xFF, 0xFF, 0x00, 0xFF)); // Stack
  Gui::drawRectangle(660, 145, 20, 20, Gui::makeColor(0x80, 0x80, 0x80, 0xFF)); // Others

  Gui::drawTextAligned(font14, 700, 62, currTheme.textColor, "Code", ALIGNED_LEFT);
  Gui::drawTextAligned(font14, 700, 82, currTheme.textColor, "Shared Memory", ALIGNED_LEFT);
  Gui::drawTextAligned(font14, 700, 102, currTheme.textColor, "Heap", ALIGNED_LEFT);
  Gui::drawTextAligned(font14, 700, 122, currTheme.textColor, "Stack", ALIGNED_LEFT);
  Gui::drawTextAligned(font14, 700, 142, currTheme.textColor, "Others", ALIGNED_LEFT);

  ss.str("");
  if (m_debugger->m_dmnt)
    ss << "EdiZon SE " VERSION_STRING;
  else
  {
    ss << "EdiZon SE " VERSION_STRING;
    ss << " (dmnt not attached)"; //"dmnt not attached to game process";
  }
  if (m_32bitmode)
    ss << "(32bit)";
  Gui::drawTextAligned(font14, 900, 62, currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);
  ss.str("");
  Gui::drawTextAligned(font14, 900, 92, currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);
  ss.str("");
  ss << "HEAP  :  0x" << std::uppercase << std::setfill('0') << std::setw(10) << std::hex << m_heapBaseAddr;
  ss << " - 0x" << std::uppercase << std::setfill('0') << std::setw(10) << std::hex << m_heapEnd;
  Gui::drawTextAligned(font14, 900, 122, currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);
  ss.str("");
  ss << "MAIN  :  0x" << std::uppercase << std::setfill('0') << std::setw(10) << std::hex << m_mainBaseAddr;
  ss << " - 0x" << std::uppercase << std::setfill('0') << std::setw(10) << std::hex << m_mainend;
  Gui::drawTextAligned(font14, 900, 152, currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);
  if (m_memoryDump1 != nullptr)
    Gui::drawRectangle(256, 50, 394, 137, COLOR_LIGHTGREEN);
  else
    Gui::drawRectangle(256, 50, 394, 137, COLOR_WHITE);
  Gui::drawTextAligned(font20, 280, 70, COLOR_BLACK, titleNameStr.c_str(), ALIGNED_LEFT);
  Gui::drawTextAligned(font14, 290, 110, COLOR_BLACK, tidStr.c_str(), ALIGNED_LEFT);
  Gui::drawTextAligned(font14, 290, 130, COLOR_BLACK, pidStr.c_str(), ALIGNED_LEFT);
  Gui::drawTextAligned(font14, 290, 150, COLOR_BLACK, buildIDStr.c_str(), ALIGNED_LEFT);
    if (m_cheatCnt > 0)
    {
      Gui::drawRectangle(50, 256, 650, 46 + std::min(static_cast<u32>(m_cheatCnt), 8U) * 40, currTheme.textColor);
      Gui::drawTextAligned(font14, 375, 262, currTheme.backgroundColor, "Cheats", ALIGNED_CENTER);
      Gui::drawShadow(50, 256, 650, 46 + std::min(static_cast<u32>(m_cheatCnt), 8U) * 40);

      for (u8 line = cheatListOffset; line < 8 + cheatListOffset; line++)
      {
        if (line >= m_cheatCnt)
          break;
        // WIP
        ss.str("");
        ss << "\uE070  " << buttonStr(m_cheats[line].definition.opcodes[0]) << ((m_editCheat && line == m_selectedEntry) ? "Press button for conditional execute" : (m_cheatDelete[line] ? " Press \uE104 to delete" : (m_cheats[line].definition.readable_name)));

        Gui::drawRectangle(52, 300 + (line - cheatListOffset) * 40, 646, 40, (m_selectedEntry == line && m_menuLocation == CHEATS) ? currTheme.highlightColor : line % 2 == 0 ? currTheme.backgroundColor : currTheme.separatorColor);
        Gui::drawTextAligned(font14, 70, 305 + (line - cheatListOffset) * 40, (m_selectedEntry == line && m_menuLocation == CHEATS) ? COLOR_BLACK : currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);

        if (!m_cheats[line].enabled)
        {
          color_t highlightColor = currTheme.highlightColor;
          highlightColor.a = 0xFF;

          Gui::drawRectangled(74, 313 + (line - cheatListOffset) * 40, 10, 10, (m_selectedEntry == line && m_menuLocation == CHEATS) ? highlightColor : line % 2 == 0 ? currTheme.backgroundColor : currTheme.separatorColor);
        }
      }
    }
    else if (m_mainBaseAddr == 0)
      Gui::drawTextAligned(font24, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 + 50, currTheme.textColor, "Dmnt detached from game process, press ZL+B to attach,\n \n relaunch EdiZon SE to access this game", ALIGNED_CENTER);
    else if (m_cheatsPresent && m_memoryDump->size() == 0)
      Gui::drawTextAligned(font24, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 + 50, currTheme.textColor, "Cheats for this game present but title version or region doesn't match!", ALIGNED_CENTER);
    else
      Gui::drawTextAligned(font24, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 + 50, currTheme.textColor, "No cheats available for this game", ALIGNED_CENTER);

    Gui::drawShadow(0, 0, Gui::g_framebuffer_width, 256);
    Gui::drawShadow(256, 50, Gui::g_framebuffer_width, 136);

    for (u16 x = 0; x < 1024; x++)
      Gui::drawRectangle(256 + x, 0, 2, 50, m_memory[x]);
    Gui::endDraw();
}

void GuiCheats::draw()
{
  if (Config::getConfig()->easymode) 
  {
    draw_easymode();
    return;
  }
  static u32 splashCnt = 0;
  std::stringstream ss;

  Gui::beginDraw();

#if SPLASH_ENABLED

  if (!Gui::g_splashDisplayed)
  {
    Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, Gui::makeColor(0x5D, 0x4F, 0x4E, 0xFF));
    Gui::drawImage(Gui::g_framebuffer_width / 2 - 128, Gui::g_framebuffer_height / 2 - 128, 256, 256, edizon_logo_bin, IMAGE_MODE_BGR24);

    if (splashCnt++ >= 70)
      Gui::g_splashDisplayed = true;

    Gui::endDraw();
    return;
  }

#endif

  Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, currTheme.backgroundColor);

  Gui::drawRectangle((u32)((Gui::g_framebuffer_width - 1220) / 2), Gui::g_framebuffer_height - 73, 1220, 1, currTheme.textColor);

  if (m_debugger->getRunningApplicationPID() == 0)
  {
    Gui::drawTextAligned(fontHuge, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 - 100, currTheme.textColor, "\uE12C", ALIGNED_CENTER);
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2, currTheme.textColor, "A title needs to be running in the background to use the RAM editor. \n Please launch an application and try again.", ALIGNED_CENTER);
    Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, "\uE0E1 Back", ALIGNED_RIGHT);
    Gui::endDraw();
    return;
  }
  // else if (!m_sysmodulePresent)
  // {
  //   Gui::drawTextAligned(fontHuge, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 - 100, currTheme.textColor, "\uE142", ALIGNED_CENTER);
  //   Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2, currTheme.textColor, "EdiZon depends on Atmosphere's dmnt:cht service which doesn't seem to be \n running on this device. Please install a supported CFW to \n use the cheat engine.", ALIGNED_CENTER);
  //   Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, "\uE0E1 Back", ALIGNED_RIGHT);
  //   Gui::endDraw();
  //   return;
  // }

  if (m_menuLocation == CHEATS)
  {
    if (m_memoryDump1 == nullptr)
    {
      Gui::drawTextAligned(font14, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 65, currTheme.textColor, "\uE105 Modify  \uE0F2 Delete  \uE0E6+\uE104 Write to File  \uE0E6+\uE0E1 Detach  \uE0E4 BM toggle   \uE0E3 Search RAM   \uE0E0 Cheat on/off   \uE0E1 Quit", ALIGNED_RIGHT);
      Gui::drawTextAligned(font14, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 35, currTheme.textColor, "\uE0E5+\uE0E1 Show Option on next Launch \uE0E6+\uE105 Remove condition key  \uE0E6+\uE0E2 Preparation for pointer Search", ALIGNED_RIGHT);
    }
    else
    {
      Gui::drawTextAligned(font14, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 65, currTheme.textColor, "\uE0EF BM add   \uE105 Modify  \uE0F2 Delete  \uE0E6+\uE104 Write to File  \uE0E6+\uE0E1 Detach  \uE0E4 BM toggle   \uE0E3 Search RAM   \uE0E0 Cheat on/off   \uE0E1 Quit", ALIGNED_RIGHT);
      Gui::drawTextAligned(font14, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 35, currTheme.textColor, "\uE0E5+\uE0E1 Show Option on next Launch \uE0E6+\uE105 Remove condition key  \uE0E6+\uE0E2 Preparation for pointer Search", ALIGNED_RIGHT);
    }
  }
  else if (m_memoryDump1 == nullptr)
  {
    if (m_memoryDump->size() == 0)
    {
      if (m_frozenAddresses.size() != 0)
        Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, "\uE0F0 Frozen es     \uE0E3 Search RAM     \uE0E1 Quit", ALIGNED_RIGHT);
      else
        Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, "\uE0E4 BM toggle      \uE0E3 Search RAM     \uE0E1 Quit", ALIGNED_RIGHT);
      // Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, "\uE0E4 E4 \uE0E5 E5 \uE0E6 E6 \uE0E7 E7 \uE0E8 E8 \uE0E9 E9 \uE0EA EA \uE0EF EF \uE0F0 F0 \uE0F1 F1 \uE0F2 F2 \uE0F3 F3 \uE0F4 F4 \uE0F5 F5 ", ALIGNED_RIGHT);
      // Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, "\uE0E1 E1 \uE0E0 E0 \uE0E1 E1 \uE0E2 E2 \uE0E3 E3 \uE0D9 D9 \uE0DA DA \uE0DF DF \uE0F0 F0 \uE0F6 F6 \uE0F7 F7 \uE0F8 F8 \uE0F9 F9 \uE0FA FA ", ALIGNED_RIGHT);
    }
    else
    {
      if (m_memoryDump->size() > 0)
      {
        Gui::drawTextAligned(font14, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 65, currTheme.textColor, "\uE0E6+\uE0E1 Detach debugger  \uE0E4 BM toggle \uE0E5 Hex Mode  \uE0EF BM add \uE0F0 Reset search \uE0E3 Search again \uE0E2 Freeze value  \uE0E0 Edit value   \uE0E1 Quit", ALIGNED_RIGHT);
        Gui::drawTextAligned(font14, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 35, currTheme.textColor, "\uE0E6+\uE0E4 \uE0E6+\uE0E5 Type  \uE0E5+\uE0E1 Show Option on next Launch \uE0E6+\uE0E2 Preparation for pointer Search  \uE0E6+\uE0E7 Page Up  \uE0E7 Page Down  \uE105 Memory Editor", ALIGNED_RIGHT);
      }
      else
        Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, "\uE0F0 Reset search     \uE0E1 Quit", ALIGNED_RIGHT);
    }
  }
  else
  {
    if (m_memoryDumpBookmark->size() > 0)
    {
      Gui::drawTextAligned(font14, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 65, currTheme.textColor, "\uE0E6+\uE0E1 Detach  \uE0E4 BM toggle   \uE0E5 Hex Mode  \uE0EF BM label  \uE0E6+\uE0E0 Add Cheat  \uE0F0 Delete BM   \uE0E2 Freeze value  \uE0E7 Page Down  \uE0E0 Edit value  \uE0E1 Quit", ALIGNED_RIGHT);
      Gui::drawTextAligned(font14, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 35, currTheme.textColor, "\uE0E6+\uE0E4 \uE0E6+\uE0E5 Change Type  \uE0E6+\uE0F0 Refresh Bookmark  \uE0E6+\uE0EF Import Bookmark  \uE0E6+\uE0E3 Pointer Search  \uE0E6+\uE0E7 Page Up  \uE105 Memory Editor", ALIGNED_RIGHT);
      //
    }
    else
      Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, "\uE0E4 BM toggle \uE0E1 Quit", ALIGNED_RIGHT);
  }

  Gui::drawRectangle(256, 50, Gui::g_framebuffer_width - 256, 206, currTheme.separatorColor);

  // Don't draw icon
  if ((m_debugger->getRunningApplicationTID() != 0) && HAVESAVE)
    Gui::drawImage(0, 0, 256, 256, Title::g_titles[m_debugger->getRunningApplicationTID()]->getTitleIcon(), IMAGE_MODE_RGB24);
  else
    Gui::drawRectangle(0, 0, 256, 256, Gui::makeColor(0x00, 0x00, 0xFF, 0xFF));

  Gui::drawRectangle(660, 65, 20, 20, Gui::makeColor(0xFF, 0x00, 0x00, 0xFF));  // Code
  Gui::drawRectangle(660, 85, 20, 20, Gui::makeColor(0x00, 0xFF, 0x00, 0xFF));  // Shared Memory
  Gui::drawRectangle(660, 105, 20, 20, Gui::makeColor(0x00, 0x00, 0xFF, 0xFF)); // Heap
  Gui::drawRectangle(660, 125, 20, 20, Gui::makeColor(0xFF, 0xFF, 0x00, 0xFF)); // Stack
  Gui::drawRectangle(660, 145, 20, 20, Gui::makeColor(0x80, 0x80, 0x80, 0xFF)); // Others

  Gui::drawTextAligned(font14, 700, 62, currTheme.textColor, "Code", ALIGNED_LEFT);
  Gui::drawTextAligned(font14, 700, 82, currTheme.textColor, "Shared Memory", ALIGNED_LEFT);
  Gui::drawTextAligned(font14, 700, 102, currTheme.textColor, "Heap", ALIGNED_LEFT);
  Gui::drawTextAligned(font14, 700, 122, currTheme.textColor, "Stack", ALIGNED_LEFT);
  Gui::drawTextAligned(font14, 700, 142, currTheme.textColor, "Others", ALIGNED_LEFT);

  ss.str("");
  if (m_debugger->m_dmnt)
    ss << "EdiZon SE " VERSION_STRING;
  else
  {
    ss << "EdiZon SE " VERSION_STRING;
    ss << " (dmnt not attached)"; //"dmnt not attached to game process";
  }
  if (m_32bitmode)
    ss << "(32bit)";
  Gui::drawTextAligned(font14, 900, 62, currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);
  ss.str("");
    if (Config::getConfig()->enabletargetedscan && m_targetmemInfos.size() != 0)
    {
      ss << "Target  :  0x" << std::uppercase << std::setfill('0') << std::setw(10) << std::hex << m_targetmemInfos[0].addr; //metadata.address_space_extents.size
      ss << " - 0x" << std::uppercase << std::setfill('0') << std::setw(10) << std::hex << (m_targetmemInfos[m_targetmemInfos.size()-1].addr + m_targetmemInfos[m_targetmemInfos.size()-1].size);
    }
    else if (m_debugger->m_dmnt)
    {

      ss << "BASE  :  0x" << std::uppercase << std::setfill('0') << std::setw(10) << std::hex << m_addressSpaceBaseAddr; //metadata.address_space_extents.size
      ss << " - 0x" << std::uppercase << std::setfill('0') << std::setw(10) << std::hex << m_addressSpaceBaseAddr + m_addressSpaceSize;
    }
  Gui::drawTextAligned(font14, 900, 92, currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);
  ss.str("");
  if (m_64bit_offset)
    ss << "HEAP64:  0x";
  else
    ss << "HEAP  :  0x";
  ss << std::uppercase << std::setfill('0') << std::setw(10) << std::hex << m_heapBaseAddr;
  ss << " - 0x" << std::uppercase << std::setfill('0') << std::setw(10) << std::hex << m_heapEnd;
  Gui::drawTextAligned(font14, 900, 122, currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);
  ss.str("");
  ss << "MAIN  :  0x" << std::uppercase << std::setfill('0') << std::setw(10) << std::hex << m_mainBaseAddr;
  ss << " - 0x" << std::uppercase << std::setfill('0') << std::setw(10) << std::hex << m_mainend;
  Gui::drawTextAligned(font14, 900, 152, currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);

  if (m_memoryDump1 != nullptr)
    Gui::drawRectangle(256, 50, 394, 137, COLOR_LIGHTGREEN);
  else
    Gui::drawRectangle(256, 50, 394, 137, COLOR_WHITE);

  Gui::drawTextAligned(font20, 280, 70, COLOR_BLACK, titleNameStr.c_str(), ALIGNED_LEFT);
  Gui::drawTextAligned(font14, 290, 110, COLOR_BLACK, tidStr.c_str(), ALIGNED_LEFT);
  Gui::drawTextAligned(font14, 290, 130, COLOR_BLACK, pidStr.c_str(), ALIGNED_LEFT);
  Gui::drawTextAligned(font14, 290, 150, COLOR_BLACK, buildIDStr.c_str(), ALIGNED_LEFT);

  // if ((Account::g_activeUser.uid[0] != 0) && (Account::g_activeUser.uid[1] != 0))
  // {
  //   ss.str("");
  //   ss << Account::g_accounts[Account::g_activeUser]->getUserName() << " [ " << std::hex << (Account::g_activeUser.uid[1]) << " " << (Account::g_activeUser.uid[0]) << " ]";
  //   Gui::drawTextAligned(font20, 768, 205, currTheme.textColor, ss.str().c_str(), ALIGNED_CENTER);
  // }
  //draw pointer chain if availabe on bookmark
  // status bar
  // if (false)
  if (m_memoryDump1 != nullptr && m_menuLocation == CANDIDATES)
  {
    bookmark_t bookmark;
    m_AttributeDumpBookmark->getData((m_selectedEntry + m_addresslist_offset) * sizeof(bookmark_t), &bookmark, sizeof(bookmark_t));
    if (bookmark.pointer.depth > 0)
    {
      ss.str("");
      int i = 0;
      ss << "z=" << bookmark.pointer.depth << " main"; //[0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << m_mainBaseAddr << "]";
      u64 nextaddress = m_mainBaseAddr;
      for (int z = bookmark.pointer.depth; z >= 0; z--)
      {
        ss << "+" << std::uppercase << std::hex << bookmark.pointer.offset[z];
        // ss << " z= " << z << " ";
        // printf("+%lx z=%d ", pointer_chain.offset[z], z);
        nextaddress += bookmark.pointer.offset[z];
        // ss << "[0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << nextaddress << "]";
        // printf("[%lx]", nextaddress);
        MemoryInfo meminfo = m_debugger->queryMemory(nextaddress);
        if (meminfo.perm == Perm_Rw)
          m_debugger->readMemory(&nextaddress, ((m_32bitmode) ? sizeof(u32) : sizeof(u64)), nextaddress);
        else
        {
          ss << "(*access denied*)";
          // printf("*access denied*");
          break;
        }
        ss << "(" << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << nextaddress << ")";
        i++;
        if ((i == 4) || (i == 8))
          ss << "\n";
        // printf("(%lx)", nextaddress);
      }
      ss << " " << dataTypes[bookmark.type];
      Gui::drawTextAligned(font14, 768, 205, currTheme.textColor, ss.str().c_str(), ALIGNED_CENTER);
    }
    else
    {
      ss.str("");
      if (bookmark.heap == true)
      {
        ss << "Heap + ";
      }
      else
      {
        ss << "Main + ";
      }
      ss << std::uppercase << std::hex << std::setfill('0') << std::setw(8) << bookmark.offset;
      ss << " " << dataTypes[bookmark.type];
      Gui::drawTextAligned(font14, 768, 205, currTheme.textColor, ss.str().c_str(), ALIGNED_CENTER);
    }
  }
  else 
  {
      static const char *const regionNames[] = {"HEAP", "MAIN", "HEAP + MAIN", "RAM", "  "};
      static const char *const modeNames[] = {"==", "!=", ">", "StateB", "<", "StateA", "A..B", "SAME", "DIFF", "+ +", "- -", "PTR", "  "};
      ss.str("");
      ss << "Search Type [ " << dataTypes[m_searchType] << " ]";
      ss << "   Search Mode [ " << modeNames[m_searchMode] << " ]";
      ss << "   Search Region [ " << regionNames[m_searchRegion] << " ]";
      if (m_use_range) 
      ss << "   [ " << _getValueDisplayString(m_searchValue[0], m_searchType) << " .. " << _getValueDisplayString(m_searchValue[1], m_searchType) << " ]";
      Gui::drawTextAligned(font14, 768, 205, currTheme.textColor, ss.str().c_str(), ALIGNED_CENTER);
  }
    if (m_cheatCnt > 0)
    {
      Gui::drawRectangle(50, 256, 650, 46 + std::min(static_cast<u32>(m_cheatCnt), 8U) * 40, currTheme.textColor);
      Gui::drawTextAligned(font14, 375, 262, currTheme.backgroundColor, "Cheats", ALIGNED_CENTER);
      Gui::drawShadow(50, 256, 650, 46 + std::min(static_cast<u32>(m_cheatCnt), 8U) * 40);

      for (u8 line = cheatListOffset; line < 8 + cheatListOffset; line++)
      {
        if (line >= m_cheatCnt)
          break;
        // WIP
        ss.str("");
        ss << "\uE070  " << buttonStr(m_cheats[line].definition.opcodes[0]) << ((m_editCheat && line == m_selectedEntry) ? "Press button for conditional execute" : (m_cheatDelete[line] ? " Press \uE104 to delete" : (m_cheats[line].definition.readable_name)));

        Gui::drawRectangle(52, 300 + (line - cheatListOffset) * 40, 646, 40, (m_selectedEntry == line && m_menuLocation == CHEATS) ? currTheme.highlightColor : line % 2 == 0 ? currTheme.backgroundColor : currTheme.separatorColor);
        Gui::drawTextAligned(font14, 70, 305 + (line - cheatListOffset) * 40, (m_selectedEntry == line && m_menuLocation == CHEATS) ? COLOR_BLACK : currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);

        if (!m_cheats[line].enabled)
        {
          color_t highlightColor = currTheme.highlightColor;
          highlightColor.a = 0xFF;

          Gui::drawRectangled(74, 313 + (line - cheatListOffset) * 40, 10, 10, (m_selectedEntry == line && m_menuLocation == CHEATS) ? highlightColor : line % 2 == 0 ? currTheme.backgroundColor : currTheme.separatorColor);
        }
      }
    }
    else if (m_mainBaseAddr == 0)
      Gui::drawTextAligned(font24, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 + 50, currTheme.textColor, "Dmnt detached from game process, press ZL+B to attach,\n \n relaunch EdiZon SE to access this game", ALIGNED_CENTER);
    else if (m_cheatsPresent && m_memoryDump->size() == 0)
      Gui::drawTextAligned(font24, Gui::g_framebuffer_width / 2, Gui::g_framebuffer_height / 2 + 50, currTheme.textColor, "Cheats for this game present but title version or region doesn't match!", ALIGNED_CENTER);

    if (m_memoryDump->getDumpInfo().dumpType == DumpType::DATA)
    {
      if (m_memoryDump->size() > 0)
      {
        Gui::drawRectangle(Gui::g_framebuffer_width - 552, 256, 500, 366, currTheme.textColor);
        Gui::drawTextAligned(font14, Gui::g_framebuffer_width - 302, 262, currTheme.backgroundColor, "Found candidates", ALIGNED_CENTER);
        Gui::drawShadow(Gui::g_framebuffer_width - 552, 256, 500, 366 * 40);
        Gui::drawRectangle(Gui::g_framebuffer_width - 550, 300, 496, 320, currTheme.separatorColor);

        ss.str("");
        ss << (static_cast<double>(m_memoryDump->size()) / (0x100000)) << "MB dumped";
        Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 302, 450, currTheme.textColor, ss.str().c_str(), ALIGNED_CENTER);
      }
    }
    else if (m_memoryDump->getDumpInfo().dumpType == DumpType::ADDR)
    {
      if (m_memoryDump->size() > 0)
      {
        if (m_memoryDump1 == nullptr)
        {
          Gui::drawRectangle(Gui::g_framebuffer_width - 552, 256, 500, 46 + std::min(static_cast<u32>(m_memoryDump->size() / sizeof(u64)), 8U) * 40, currTheme.textColor);
          ss.str("");
          ss << "Found candidates   " << std::dec << (((m_menuLocation == CANDIDATES) ? m_selectedEntry : 0) + m_addresslist_offset + 1) << " / " << std::dec << ((m_memoryDump->size() / sizeof(u64)));
          Gui::drawTextAligned(font14, Gui::g_framebuffer_width - 302, 262, currTheme.backgroundColor, ss.str().c_str(), ALIGNED_CENTER);
          Gui::drawShadow(Gui::g_framebuffer_width - 552, 256, 500, 46 + std::min(static_cast<u32>(m_memoryDump->size() / sizeof(u64)), 8U) * 40);
        }
        else
        {
          Gui::drawRectangle(Gui::g_framebuffer_width - 557, 256, 549, 46 + std::min(static_cast<u32>(m_memoryDump->size() / sizeof(u64)), 8U) * 40, currTheme.textColor);
          ss.str("");
          ss << "   Bookmarks   " << std::dec << (((m_menuLocation == CANDIDATES) ? m_selectedEntry : 0) + m_addresslist_offset + 1) << " / " << std::dec << ((m_memoryDump->size() / sizeof(u64)));
          Gui::drawTextAligned(font14, Gui::g_framebuffer_width - 302, 262, currTheme.backgroundColor, ss.str().c_str(), ALIGNED_CENTER);
          // Gui::drawTextAligned(font14, Gui::g_framebuffer_width - 302, 262, currTheme.backgroundColor, "Book Marks", ALIGNED_CENTER);
          Gui::drawShadow(Gui::g_framebuffer_width - 557, 256, 549, 46 + std::min(static_cast<u32>(m_memoryDump->size() / sizeof(u64)), 8U) * 40);
        }
      }
      // mod start memory line offset

      if (m_memoryDump1 == nullptr)
        for (u8 line = 0; line < 8; line++)
        {
          if ((line + m_addresslist_offset) >= (m_memoryDump->size() / sizeof(u64)))
            break;

          ss.str("");

          if (line < 8) // && (m_memoryDump->size() / sizeof(u64)) != 8)
          {
            u64 address = 0;
            m_memoryDump->getData((line + m_addresslist_offset) * sizeof(u64), &address, sizeof(u64));
            // candidate display
            if (Config::getConfig()->use_absolute_address)
              ss << "[ 0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << (address) << " ]";
            else if (address >= m_heapBaseAddr && address < m_heapEnd)
              ss << "[ HEAP + 0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << (address - m_heapBaseAddr) << " ]";
            else if (address >= m_mainBaseAddr && address < m_mainend)
              ss << "[ MAIN + 0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << (address - m_mainBaseAddr) << " ]";
            else
              ss << "[ BASE + 0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << (address - m_memoryDump->getDumpInfo().addrSpaceBaseAddress) << " ]";

            ss << "  ( " << _getAddressDisplayString(address, m_debugger, (searchType_t)m_searchType) << " )";

            if (m_frozenAddresses.find(address) != m_frozenAddresses.end())
              ss << "   \uE130";
          }
          else
            ss << "And " << std::dec << ((m_memoryDump->size() / sizeof(u64)) - 8) << " others...";

          Gui::drawRectangle(Gui::g_framebuffer_width - 550, 300 + line * 40, 496, 40, (m_selectedEntry == line && m_menuLocation == CANDIDATES) ? currTheme.highlightColor : line % 2 == 0 ? currTheme.backgroundColor : currTheme.separatorColor);
          Gui::drawTextAligned(font14, Gui::g_framebuffer_width - 530, 305 + line * 40, (m_selectedEntry == line && m_menuLocation == CANDIDATES) ? COLOR_BLACK : currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);
        }
      else // Bookmark screen
        for (u8 line = 0; line < 8; line++)
        {
          if ((line + m_addresslist_offset) >= (m_memoryDump->size() / sizeof(u64)))
            break;

          ss.str("");

          bookmark_t bookmark;
          if (line < 8) // && (m_memoryDump->size() / sizeof(u64)) != 8)
          {
            u64 address = 0;
            m_memoryDump->getData((line + m_addresslist_offset) * sizeof(u64), &address, sizeof(u64));
            m_AttributeDumpBookmark->getData((line + m_addresslist_offset) * sizeof(bookmark_t), &bookmark, sizeof(bookmark_t));
            // if (false)
            if (bookmark.pointer.depth > 0) // check if pointer chain point to valid address update address if necessary
            {
              bool updateaddress = true;
              u64 nextaddress = m_mainBaseAddr;
              for (int z = bookmark.pointer.depth; z >= 0; z--)
              {
                nextaddress += bookmark.pointer.offset[z];
                MemoryInfo meminfo = m_debugger->queryMemory(nextaddress);
                if (meminfo.perm == Perm_Rw)
                  if (z == 0)
                  {
                    if (address == nextaddress)
                      updateaddress = false;
                    else
                    {
                      address = nextaddress;
                    }
                  }
                  else
                    m_debugger->readMemory(&nextaddress, ((m_32bitmode) ? sizeof(u32) : sizeof(u64)), nextaddress);
                else
                {
                  updateaddress = false;
                  break;
                }
              }
              if (updateaddress)
              {
                m_memoryDump->putData((line + m_addresslist_offset) * sizeof(u64), &address, sizeof(u64));
                m_memoryDump->flushBuffer();
              }
            }
            // bookmark display
            ss << "[0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << (address) << "]"; //<< std::left << std::setfill(' ') << std::setw(18) << bookmark.label <<

            ss << "  ( " << _getAddressDisplayString(address, m_debugger, (searchType_t)bookmark.type) << " )";

            if (m_frozenAddresses.find(address) != m_frozenAddresses.end())
              ss << " \uE130";
            if (bookmark.pointer.depth > 0) // have pointer
              ss << " *";
          }
          else
            ss << "And " << std::dec << ((m_memoryDump->size() / sizeof(u64)) - 8) << " others...";

          Gui::drawRectangle(Gui::g_framebuffer_width - 555, 300 + line * 40, 545, 40, (m_selectedEntry == line && m_menuLocation == CANDIDATES) ? currTheme.highlightColor : line % 2 == 0 ? currTheme.backgroundColor : currTheme.separatorColor);
          Gui::drawTextAligned(font14, Gui::g_framebuffer_width - 545, 305 + line * 40, (m_selectedEntry == line && m_menuLocation == CANDIDATES) ? COLOR_BLACK : currTheme.textColor, bookmark.deleted ? "To be deleted" : bookmark.label, ALIGNED_LEFT);
          Gui::drawTextAligned(font14, Gui::g_framebuffer_width - 340, 305 + line * 40, (m_selectedEntry == line && m_menuLocation == CANDIDATES) ? COLOR_BLACK : currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);
        }
    }

    Gui::drawShadow(0, 0, Gui::g_framebuffer_width, 256);
    Gui::drawShadow(256, 50, Gui::g_framebuffer_width, 136);

    for (u16 x = 0; x < 1024; x++)
      Gui::drawRectangle(256 + x, 0, 2, 50, m_memory[x]);

    drawSearchRAMMenu();
    drawEditRAMMenu();
    drawEditRAMMenu2();
    drawEditExtraSearchValues();
    drawSEARCH_pickjump();
    drawSearchPointerMenu();
    Gui::endDraw();
  }
// BM2

void GuiCheats::drawSearchPointerMenu()
{
  if (m_searchMenuLocation != SEARCH_POINTER)
    return;
  static u32 cursorBlinkCnt = 0;
  u32 strWidth = 0;
  std::stringstream ss;

  Gui::drawRectangled(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, Gui::makeColor(0x00, 0x00, 0x00, 0xA0));

  Gui::drawRectangle(50, 50, Gui::g_framebuffer_width - 100, Gui::g_framebuffer_height - 100, currTheme.backgroundColor);
  Gui::drawRectangle(100, 135, Gui::g_framebuffer_width - 200, 1, currTheme.textColor);
  Gui::drawText(font24, 120, 70, currTheme.textColor, "\uE132   Search Pointer");
  Gui::drawTextAligned(font14, Gui::g_framebuffer_width / 2, 500, currTheme.textColor,
                       "Set the parameters of your pointer search. You can keep the time require within reasonable range by trading off between \n"
                       "max depth, max range and max source. The impact of these setting to the time taken to complete the search will largely \n"
                       "depends on the game itself too. Dump forward only assume pointer pointing to larger address is forward which may not be.",
                       ALIGNED_CENTER);

  //Gui::drawRectangle(300, 250, Gui::g_framebuffer_width - 600, 80, currTheme.separatorColor);
  // Gui::drawRectangle(300, 327, Gui::g_framebuffer_width - 600, 3, currTheme.textColor);
  //  m_max_depth = 2;
  //  m_max_range = 0x300;
  //  m_max_source = 200;

  // if (m_searchValueFormat == FORMAT_DEC)
  //   ss << _getValueDisplayString(m_searchValue[0], m_searchType);
  // else if (m_searchValueFormat == FORMAT_HEX)

  Gui::drawText(font20, 310, 160, currTheme.textColor, "Max Depth");
  ss.str("");
  ss << std::uppercase << std::dec << m_max_depth;
  Gui::getTextDimensions(font20, ss.str().c_str(), &strWidth, nullptr);
  Gui::drawTextAligned(font20, 620, 160, currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);
  if (cursorBlinkCnt++ % 60 > 10 && m_selectedEntry == 0)
    Gui::drawRectangled(622 + strWidth, 160, 3, 35, currTheme.highlightColor);

  Gui::drawText(font20, 310, 200, currTheme.textColor, "Max Range");
  ss.str("");
  ss << "0x" << std::uppercase << std::hex << m_max_range;
  Gui::getTextDimensions(font20, ss.str().c_str(), &strWidth, nullptr);
  Gui::drawTextAligned(font20, 620, 200, currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);
  if (cursorBlinkCnt++ % 60 > 10 && m_selectedEntry == 1)
    Gui::drawRectangled(622 + strWidth, 200, 3, 35, currTheme.highlightColor);

  Gui::drawText(font20, 310, 240, currTheme.textColor, "Max Source");
  ss.str("");
  ss << std::uppercase << std::dec << m_max_source;
  Gui::getTextDimensions(font20, ss.str().c_str(), &strWidth, nullptr);
  Gui::drawTextAligned(font20, 620, 240, currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);
  if (cursorBlinkCnt++ % 60 > 10 && m_selectedEntry == 2)
    Gui::drawRectangled(622 + strWidth, 240, 3, 35, currTheme.highlightColor);

  Gui::drawText(font20, 310, 280, currTheme.textColor, "Target Address");
  ss.str("");
  ss << "0x" << std::uppercase << std::hex << m_EditorBaseAddr;
  if (m_pointersearch_canresume)
    ss << " Resumable";
  Gui::getTextDimensions(font20, ss.str().c_str(), &strWidth, nullptr);
  Gui::drawTextAligned(font20, 620, 280, currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);
  if (cursorBlinkCnt++ % 60 > 10 && m_selectedEntry == 3)
    Gui::drawRectangled(622 + strWidth, 280, 3, 35, currTheme.highlightColor);

  Gui::drawText(font20, 310, 320, currTheme.textColor, "Dump Forward only");
  ss.str("");
  if (m_forwarddump)
    ss << "YES";
  else
    ss << "NO";
  Gui::getTextDimensions(font20, ss.str().c_str(), &strWidth, nullptr);
  Gui::drawTextAligned(font20, 620, 320, currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);
  if (cursorBlinkCnt++ % 60 > 10 && m_selectedEntry == 4)
    Gui::drawRectangled(622 + strWidth, 320, 3, 35, currTheme.highlightColor);

  Gui::drawText(font20, 310, 360, currTheme.textColor, "Max num of Offsets");
  ss.str("");
  ss << "0x" << std::uppercase << std::hex << m_numoffset;
  Gui::getTextDimensions(font20, ss.str().c_str(), &strWidth, nullptr);
  Gui::drawTextAligned(font20, 620, 360, currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);
  if (cursorBlinkCnt++ % 60 > 10 && m_selectedEntry == 5)
    Gui::drawRectangled(622 + strWidth, 360, 3, 35, currTheme.highlightColor);

  Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 100, Gui::g_framebuffer_height - 100, currTheme.textColor, "\uE0E6+\uE0E3 Make Dump for pointersearcher SE    \uE0EF Start Search   \uE0E1 Abort     \uE0E4 \uE0E5 Edit Value", ALIGNED_RIGHT);

  // if (m_selectedEntry == 3)
  //   Gui::drawRectangled(Gui::g_framebuffer_width / 2 - 155, 345, 310, 90, currTheme.highlightColor);

  // if (m_searchType != SEARCH_TYPE_NONE && m_searchMode != SEARCH_MODE_NONE && m_searchRegion != SEARCH_REGION_NONE)
  // {
  //   Gui::drawRectangled(Gui::g_framebuffer_width / 2 - 150, 350, 300, 80, currTheme.selectedColor);
  //   Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, 375, currTheme.backgroundColor, "Search Now!", ALIGNED_CENTER);
  // }
  // else
  // {
  //   Gui::drawRectangled(Gui::g_framebuffer_width / 2 - 150, 350, 300, 80, currTheme.selectedButtonColor);
  //   Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, 375, currTheme.separatorColor, "Search Now!", ALIGNED_CENTER);
  // }

  //   break;
  // case SEARCH_NONE:
  //   break;
}

void GuiCheats::drawEditRAMMenu()
{
  // static u32 cursorBlinkCnt = 0;
  // u32 strWidth = 0;
  std::stringstream ss;

  if (m_searchMenuLocation != SEARCH_editRAM) // need
    return;

  Gui::drawRectangled(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, Gui::makeColor(0x00, 0x00, 0x00, 0xA0));

  Gui::drawRectangle(50, 50, Gui::g_framebuffer_width - 100, Gui::g_framebuffer_height - 100, currTheme.backgroundColor);
  Gui::drawRectangle(100, 135, Gui::g_framebuffer_width - 200, 1, currTheme.textColor);
  Gui::drawText(font24, 120, 70, currTheme.textColor, "\uE132   Edit Memory");
  Gui::drawTextAligned(font20, 100, 160, currTheme.textColor, "\uE149 \uE0A4", ALIGNED_LEFT);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 100, 160, currTheme.textColor, "\uE0A5 \uE14A", ALIGNED_RIGHT);
  Gui::drawTextAligned(font20, 260, 160, m_searchMenuLocation == SEARCH_TYPE ? currTheme.selectedColor : currTheme.textColor, "U8", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, 510, 160, m_searchMenuLocation == SEARCH_MODE ? currTheme.selectedColor : currTheme.textColor, "U16", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, 760, 160, m_searchMenuLocation == SEARCH_REGION ? currTheme.selectedColor : currTheme.textColor, "u32", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, 1010, 160, m_searchMenuLocation == SEARCH_VALUE ? currTheme.selectedColor : currTheme.textColor, "u64", ALIGNED_CENTER);

  // strcpy(initialString, _getAddressDisplayString(address, m_debugger, m_searchType).c_str());
  // ss << "[ HEAP + 0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << (address - m_memoryDump->getDumpInfo().heapBaseAddress) << " ]";
  // std::stringstream ss;
  //
  // dmntchtReadCheatProcessMemory(addr, &out, sizeof(u32));
  // address = m_EditorBaseAddr - (m_EditorBaseAddr % 16) - 0x20 + (m_selectedEntry -1 - (m_selectedEntry div 5))*4;
  // m_selectedEntry = (m_EditorBaseAddr % 10) / 4 + 11;
  u64 addr = m_EditorBaseAddr - (m_EditorBaseAddr % 16) - 0x20;
  u32 out;

  u64 address = m_EditorBaseAddr - (m_EditorBaseAddr % 16) - 0x20 + (m_selectedEntry - 1 - (m_selectedEntry / 5)) * 4 + m_addressmod;
  ss.str("");
  ss << "[ " << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << (address) << " ]";
  Gui::drawText(font24, 520, 70, currTheme.textColor, ss.str().c_str());
  // Next to display the value in the selected type now is u32 in hex
  ss.str("");
  // dmntchtReadCheatProcessMemory(address, &out, sizeof(u32));
  m_debugger->readMemory(&out, sizeof(u32), address);
  // ss << "0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(8) << out << "";
  Gui::drawText(font24, 820, 70, currTheme.textColor, _getAddressDisplayString(address, m_debugger, m_searchType).c_str()); //ss.str().c_str()

  for (u8 i = 0; i < 40; i++)
  {
    if (m_selectedEntry == i)
      Gui::drawRectangled(88 + (i % 5) * 225, 235 + (i / 5) * 50, 225, 50, m_searchMode == static_cast<searchMode_t>(i) ? currTheme.selectedColor : currTheme.highlightColor);
    if ((i % 5) != 0)
    {
      Gui::drawRectangled(93 + (i % 5) * 225, 240 + (i / 5) * 50, 215, 40, currTheme.separatorColor);
      ss.str("");
      // dmntchtReadCheatProcessMemory(addr, &out, sizeof(u32));
      m_debugger->readMemory(&out, sizeof(u32), addr);
      ss << "0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(8) << out << "";
      Gui::drawTextAligned(font20, 200 + (i % 5) * 225, 245 + (i / 5) * 50, currTheme.textColor, ss.str().c_str(), ALIGNED_CENTER);
      addr += 4;
    }
    else
    {
      ss.str("");
      ss << "[ " << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << (addr) << " ]";
      Gui::drawTextAligned(font20, 200 + (i % 5) * 225, 245 + (i / 5) * 50, currTheme.textColor, ss.str().c_str(), ALIGNED_CENTER);
    }
  }
}
// WIP edit ram
std::string GuiCheats::buttonStr(u32 buttoncode)
{
  for (u32 i = 0; i < buttonCodes.size(); i++)
  {
    if (buttoncode == buttonCodes[i])
      return buttonNames[i].c_str();
  }
  return "";
}
#define line1 10
#define line2 80
#define line3 120
#define line4 160
#define line5 195

void GuiCheats::drawEditRAMMenu2()
{
  std::stringstream ss;
  if (m_searchMenuLocation != SEARCH_editRAM2)
    return;
  if (m_EditorBaseAddr >= m_mainBaseAddr && m_EditorBaseAddr <= m_mainend)
    Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, Gui::makeColor(0xFF, 0xFF, 0xC0, 0x40));
  else
    Gui::drawRectangle(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, currTheme.backgroundColor); //background
  Gui::drawText(font24, 30, 10 , currTheme.textColor, "\uE132   Memory Explorer");
  Gui::drawRectangle(10, 70 , Gui::g_framebuffer_width - 10, 1, currTheme.textColor);//the line

  // Gui::drawTextAligned(font20, 100, line2, currTheme.textColor, "\uE149 \uE0A4", ALIGNED_LEFT); // the start bracket
  // Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 100, line2, currTheme.textColor, "\uE0A5 \uE14A", ALIGNED_RIGHT); // the end bracket
  // Gui::drawTextAligned(font20, 260, line2, m_searchMenuLocation == SEARCH_TYPE ? currTheme.selectedColor : currTheme.textColor, "U8", ALIGNED_CENTER);
  // Gui::drawTextAligned(font20, 510, line2, m_searchMenuLocation == SEARCH_MODE ? currTheme.selectedColor : currTheme.textColor, "U16", ALIGNED_CENTER);
  // Gui::drawTextAligned(font20, 760, line2, m_searchMenuLocation == SEARCH_REGION ? currTheme.selectedColor : currTheme.textColor, "u32", ALIGNED_CENTER);
  // Gui::drawTextAligned(font20, 1010, line2, m_searchMenuLocation == SEARCH_VALUE ? currTheme.selectedColor : currTheme.textColor, "u64", ALIGNED_CENTER);

// status line
  u64 addr = m_EditorBaseAddr - (m_EditorBaseAddr % 16) - 0x20; 
  u32 out;
  u64 address = m_EditorBaseAddr - (m_EditorBaseAddr % 16) - 0x20 + (m_selectedEntry - 1 - (m_selectedEntry / 5)) * 4 ;
  ss.str("");
  ss << "[ " << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << (address + m_addressmod) << " ] " << dataTypes[m_searchType];
  Gui::drawText(font24, 420, line1, currTheme.textColor, ss.str().c_str());
  ss.str("");
  //dmntchtReadCheatProcessMemory(address, &out, sizeof(u32));
  m_debugger->readMemory(&out, sizeof(u32), address);
  Gui::drawText(font24, 830, line1, currTheme.textColor, _getAddressDisplayString(address + m_addressmod, m_debugger, m_searchType).c_str()); //ss.str().c_str()

  // display all datatypes
  // m_searchtype = SEARCH_TYPE_POINTER;
  searchType_t m_searchType2;

  m_searchType2 = SEARCH_TYPE_UNSIGNED_8BIT;
  ss.str("");
  ss << dataTypes[m_searchType2] << ":" << std::uppercase << std::dec << std::setfill('0') << std::setw(3) << _getAddressDisplayString(address, m_debugger, m_searchType2).c_str();
  ss << ":" << std::uppercase << std::dec << std::setfill('0') << std::setw(3) << _getAddressDisplayString(address + 1, m_debugger, m_searchType2).c_str();
  ss << ":" << std::uppercase << std::dec << std::setfill('0') << std::setw(3) << _getAddressDisplayString(address + 2, m_debugger, m_searchType2).c_str();
  ss << ":" << std::uppercase << std::dec << std::setfill('0') << std::setw(3) << _getAddressDisplayString(address + 3, m_debugger, m_searchType2).c_str();
  Gui::drawTextAligned(font20, 30, line2, currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT); // the start bracket
  m_searchType2 = SEARCH_TYPE_UNSIGNED_16BIT;
  ss.str("");
  ss << dataTypes[m_searchType2]<<":" << std::uppercase << std::dec << std::setfill('0') << std::setw(5) << _getAddressDisplayString(address, m_debugger, m_searchType2).c_str();
  ss << ":" << std::uppercase << std::dec << std::setfill('0') << std::setw(5) << _getAddressDisplayString(address+2, m_debugger, m_searchType2).c_str();
  Gui::drawTextAligned(font20, 360, line2, currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);
  m_searchType2 = SEARCH_TYPE_SIGNED_32BIT;
  ss.str("");
  ss << dataTypes[m_searchType2]<<":" << std::uppercase << std::dec << _getAddressDisplayString(address, m_debugger, m_searchType2).c_str();
  Gui::drawTextAligned(font20, 610, line2, currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);
  m_searchType2 = SEARCH_TYPE_SIGNED_64BIT;
  ss.str("");
  ss << dataTypes[m_searchType2]<<":" << std::uppercase << std::dec << _getAddressDisplayString(address, m_debugger, m_searchType2).c_str();
  Gui::drawTextAligned(font20, 860, line2, currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);
  m_searchType2 = SEARCH_TYPE_FLOAT_32BIT;
  ss.str("");
  ss << dataTypes[m_searchType2]<<":" << std::uppercase << std::dec << _getAddressDisplayString(address, m_debugger, m_searchType2).c_str();
  Gui::drawTextAligned(font20, 610, line3, currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);
  m_searchType2 = SEARCH_TYPE_FLOAT_64BIT;
  ss.str("");
  ss << dataTypes[m_searchType2]<<":" << std::uppercase << std::dec << _getAddressDisplayString(address, m_debugger, m_searchType2).c_str();
  Gui::drawTextAligned(font20, 860, line3, currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);

  // pointer display if address in range
  if (address % 8 == 0)
  {
    u64 pointed_address;
    m_debugger->readMemory(&pointed_address, sizeof(u64), address);
    ss.str("");
    if (pointed_address >= m_mainBaseAddr && pointed_address <= m_mainend)
      ss << "Main + " << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << pointed_address - m_mainBaseAddr;
    else if (pointed_address >= m_heapBaseAddr && pointed_address <= m_heapEnd)
      ss << "Heap + " << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << pointed_address - m_heapBaseAddr;
    if (ss.str().size() != 0)
    {
      Gui::drawTextAligned(font20, 30, line3, currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);
      ss.str("");
      ss << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << pointed_address;
      Gui::drawTextAligned(font20, 360, line3, currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);
    }
  };

  // ss.str("");
  // ss << "Bookmark line 1 drawTextAligned(font20, Gui::g_framebuffer_width - 100, line2, currTheme.text";
  // Gui::drawTextAligned(font20, 30, line4, currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);
  // ss.str("");
  // ss << "Bookmark line 2 Gui::drawTextAligned(font20, 1010, line2, m_searchMenuLocation == SEARCH_";
  // Gui::drawTextAligned(font20, 30, line5, currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);
//ME1 Memory explorer pointer chain display 
// if (false)
  {
    if (m_bookmark.pointer.depth > 0)
    {
      ss.str("");
      int i = 0;
      ss << "z=" << std::dec << std::setfill('0') << std::setw(2) << m_depth_count << ((m_bookmark.heap)? " heap":" main"); //[0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << m_mainBaseAddr << "]";
      m_depth_count = -1;
      u64 nextaddress;
      if (m_bookmark.heap)
        nextaddress = m_heapBaseAddr;
      else
        nextaddress = m_mainBaseAddr;
      for (int z = m_bookmark.pointer.depth; z >= 0; z--)
      {
        ss << "+" << std::uppercase << std::hex << (s32)m_bookmark.pointer.offset[z];
        if (z == m_z)
        {
          if (address + m_addressmod >= nextaddress)
            ss << "[\uE130 " << std::uppercase << std::hex << std::setfill('0') << std::setw(2) << address - nextaddress + m_addressmod << " ]";
          else
            ss << "[\uE130 -" << std::uppercase << std::hex << std::setfill('0') << std::setw(2) << nextaddress - address - m_addressmod << " ]";
        }
        nextaddress += m_bookmark.pointer.offset[z];
        m_jump_stack[z].from = nextaddress;
        MemoryInfo meminfo = m_debugger->queryMemory(nextaddress);
        if (meminfo.perm == Perm_Rw)
          m_debugger->readMemory(&nextaddress, ((m_32bitmode) ? sizeof(u32) : sizeof(u64)), nextaddress);
        else
        {
          m_jump_stack[z].to = 0;
          ss << "(*access denied*)";
          break;
        }
        m_depth_count++;
        if (z > 0)
        {
            ss << "(" << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << nextaddress << ")";
        }
        m_jump_stack[z].to = nextaddress;
        i++;
        if ((i == 6) || (i == 13))
          ss << "\n";
      }
      // ss << " " << dataTypes[m_bookmark.type];
      Gui::drawTextAligned(font14, 30, line4, currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);
    }
  }

//

  // Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 100, line2, currTheme.textColor, "\uE0A5 \uE14A", ALIGNED_RIGHT); // the end bracket
  // Gui::drawTextAligned(font20, 260, line2, m_searchMenuLocation == SEARCH_TYPE ? currTheme.selectedColor : currTheme.textColor, "U8", ALIGNED_CENTER);
  // Gui::drawTextAligned(font20, 510, line2, m_searchMenuLocation == SEARCH_MODE ? currTheme.selectedColor : currTheme.textColor, "U16", ALIGNED_CENTER);
  // Gui::drawTextAligned(font20, 760, line2, m_searchMenuLocation == SEARCH_REGION ? currTheme.selectedColor : currTheme.textColor, "u32", ALIGNED_CENTER);
  // Gui::drawTextAligned(font20, 1010, line2, m_searchMenuLocation == SEARCH_VALUE ? currTheme.selectedColor : currTheme.textColor, "u64", ALIGNED_CENTER);


// Grid display

  for (u8 i = 0; i < 40; i++) // 8 Row X 5 column
  {
    if (m_selectedEntry == i)
      Gui::drawRectangled(88 + (i % 5) * 225, 235 + (i / 5) * 50, 225, 50, currTheme.highlightColor);
    if ((i % 5) != 0)
    {
      color_t separatorColor;
      if (addr % 8 == 0 && m_show_ptr)
      {
        separatorColor = currTheme.separatorColor;
        u64 pointed_address;
        m_debugger->readMemory(&pointed_address, sizeof(u64), addr);
        // ss.str("");
        if (pointed_address >= m_mainBaseAddr && pointed_address <= m_mainend)
        separatorColor = Gui::makeColor(0xFF, 0xFF, 0x00, 0x40); 
          // ss << "M+" << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << pointed_address - m_mainBaseAddr;
        else if (pointed_address >= m_heapBaseAddr && pointed_address <= m_heapEnd)
        separatorColor = Gui::makeColor(0x00, 0xFF, 0x00, 0x40); 
          // ss << "H+" << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << pointed_address - m_heapBaseAddr;
        // if (ss.str().size() != 0)
        // {
        //   Gui::drawTextAligned(font20, 200 + (i % 5) * 225, 245 + (i / 5) * 50, currTheme.textColor, ss.str().c_str(), ALIGNED_CENTER);
        //   addr += 8;
        //   i++;
        //   ss.str("");
        //   ss << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << pointed_address;
        //   Gui::drawTextAligned(font20, 200 + (i % 5) * 225, 245 + (i / 5) * 50, currTheme.textColor, ss.str().c_str(), ALIGNED_CENTER);
        //   continue;
        // }
      };
      Gui::drawRectangled(93 + (i % 5) * 225, 240 + (i / 5) * 50, 215, 40, separatorColor);
      ss.str("");
      // dmntchtReadCheatProcessMemory(addr, &out, sizeof(u32));
      m_debugger->readMemory(&out, sizeof(u32), addr);
      if (dataTypeSizes[m_searchType] == 1)
      {
        u8 outb[4];
        memcpy(&outb, &out, 4);
        ss << ((m_addressmod == 0 && m_selectedEntry == i) ? "[" : " ") << std::uppercase << std::hex << std::setfill('0') << std::setw(2) << (u16)outb[0] << ((m_addressmod == 0 && m_selectedEntry == i) ? "]" : " ");
        ss << ((m_addressmod == 1 && m_selectedEntry == i) ? "[" : " ") << std::uppercase << std::hex << std::setfill('0') << std::setw(2) << (u16)outb[1] << ((m_addressmod == 1 && m_selectedEntry == i) ? "]" : " ");
        ss << ((m_addressmod == 2 && m_selectedEntry == i) ? "[" : " ") << std::uppercase << std::hex << std::setfill('0') << std::setw(2) << (u16)outb[2] << ((m_addressmod == 2 && m_selectedEntry == i) ? "]" : " ");
        ss << ((m_addressmod == 3 && m_selectedEntry == i) ? "[" : " ") << std::uppercase << std::hex << std::setfill('0') << std::setw(2) << (u16)outb[3] << ((m_addressmod == 3 && m_selectedEntry == i) ? "]" : " ");
      }
      else if (dataTypeSizes[m_searchType] == 2)
      {
        u16 outb[2];
        memcpy(&outb, &out, 4);
        ss << ((m_addressmod == 0 && m_selectedEntry == i) ? "[" : " ") << std::uppercase << std::hex << std::setfill('0') << std::setw(4) << (u16)outb[0] << ((m_addressmod == 0 && m_selectedEntry == i) ? "]" : " ");
        ss << ((m_addressmod == 2 && m_selectedEntry == i) ? "[" : " ") << std::uppercase << std::hex << std::setfill('0') << std::setw(4) << (u16)outb[1] << ((m_addressmod == 2 && m_selectedEntry == i) ? "]" : " ");
      }
      else
      {
        ss << std::uppercase << std::hex << std::setfill('0') << std::setw(8) << out << "";
      }
      Gui::drawTextAligned(font20, 200 + (i % 5) * 225, 245 + (i / 5) * 50, currTheme.textColor, ss.str().c_str(), ALIGNED_CENTER);
      addr += 4;
    }
    else
    {
      ss.str("");
      ss << "[ " << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << (addr) << " ]";
      Gui::drawTextAligned(font20, 200 + (i % 5) * 225, 245 + (i / 5) * 50, currTheme.textColor, ss.str().c_str(), ALIGNED_CENTER);
    }
  }

  // key hints
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 70, currTheme.textColor, "\uE105 MarkSearch \uE0E3 Change offset \uE0EF BM add \uE0E0 Edit value \uE0E4 Backward \uE0E5 Forward \uE0E1 JumpBack", ALIGNED_RIGHT); //\uE0E4 Change Mode  
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 35, currTheme.textColor, "\uE0E6+\uE0E4 \uE0E6+\uE0E5 Change Type  \uE0E6+\uE0E3 Goto any address  \uE0E7 PageDown  \uE0E6+\uE0E7 PageUp  \uE0E6+\uE0E1 Quit", ALIGNED_RIGHT);
}
void GuiCheats::drawEditExtraSearchValues()
{
  std::stringstream ss;
  if (m_searchMenuLocation != SEARCH_editExtraSearchValues)
    return;
  Gui::drawRectangled(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, Gui::makeColor(0x00, 0x00, 0x00, 0xA0));
  // Gui::drawRectangle(50, 50, Gui::g_framebuffer_width - 100, Gui::g_framebuffer_height - 100, currTheme.backgroundColor);
  Gui::drawRectangle(0, 50, Gui::g_framebuffer_width, Gui::g_framebuffer_height - 100, currTheme.backgroundColor);
  Gui::drawRectangle(100, 135, Gui::g_framebuffer_width - 200, 1, currTheme.textColor);
  {
      static const char *const regionNames[] = {"HEAP", "MAIN", "HEAP + MAIN", "RAM", "  "};
      ss.str("");
      ss << "\uE132   Multi Target Memory Search";
      ss << "   [ " << regionNames[m_searchRegion] << " ]";
      // ss << " line = " << m_selectedEntry / 6;
  }
  Gui::drawText(font24, 120, 70, currTheme.textColor, ss.str().c_str());
  ss.str("");
#define shift1 15 + 50
#define shift2 40
#define c0 75
#define c1 135 + shift1
#define c2 260 + shift1
#define c3 385 + shift1
#define c4 510 + shift1
#define c5 760 + shift1 - shift2
#define c6 1010 + shift1 + shift2
#define linegape 30
#define M_ENTRY m_multisearch.Entries[i / 6]
#define M_ENTRYi m_multisearch.Entries[i]
#define labelline 145
  color_t cellColor;
  Gui::drawTextAligned(font20, c0, labelline, currTheme.textColor, "LABEL", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, c1, labelline, currTheme.textColor, "OFFSET", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, c2, labelline, currTheme.textColor, "On/OFF", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, c3, labelline, currTheme.textColor, "MODE", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, c4, labelline, currTheme.textColor, "TYPE", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, c5, labelline, currTheme.textColor, "VALUE 1", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, c6, labelline, currTheme.textColor, "VALUE 2", ALIGNED_CENTER);

  for (u8 i = 0; i < 60; i++) // 10 Row X 6 column
  {
    if (m_selectedEntry == i)
      cellColor = currTheme.selectedColor;
    else
      cellColor = currTheme.textColor;
    if ((i % 6) == 0)
    {
      ss.str("");
      ss << M_ENTRY.label;
      Gui::drawTextAligned((ss.str().size() > 8) ? font14 : font20, c0, 160 + linegape * (1 + i / 6), ((i / 6) == (m_selectedEntry / 6)) ? currTheme.selectedColor : currTheme.textColor, ss.str().c_str(), ALIGNED_CENTER);

      ss.str("");
      ss << "0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(3) << m_multisearch.Entries[i / 6].offset;
      Gui::drawTextAligned(font20, c1, 160 + linegape * (1 + i / 6), cellColor, ss.str().c_str(), ALIGNED_CENTER);
    }
    else if ((i % 6) == 1)
    {
      if (M_ENTRY.on == TARGET) //(i / 6 == m_multisearch.target)
        Gui::drawTextAligned(font20, c2, 160 + linegape * (1 + i / 6), cellColor, "Target", ALIGNED_CENTER);
      else
        Gui::drawTextAligned(font20, c2, 160 + linegape * (1 + i / 6), cellColor, (m_multisearch.Entries[i / 6].on == ON) ? "On" : "OFF", ALIGNED_CENTER);
    }
    else if ((i % 6) == 2)
    {
      static const char *const modeNames[] = {"==", "!=", ">", "StateB", "<", "StateA", "A..B", "SAME", "DIFF", "+ +", "- -", "PTR", "  ","~PTR"};
      // if (M_ENTRY.type != SEARCH_TYPE_POINTER)
      Gui::drawTextAligned(font20, c3, 160 + linegape * (1 + i / 6), cellColor, modeNames[m_multisearch.Entries[i / 6].mode], ALIGNED_CENTER);
    }
    else if ((i % 6) == 3)
    {
      static const char *const typeNames[] = {"u8", "s8", "u16", "s16", "u32", "s32", "u64", "s64", "flt", "dbl", "pointer"};
      // if (M_ENTRY.mode != SEARCH_MODE_POINTER)
      Gui::drawTextAligned(font20, c4, 160 + linegape * (1 + i / 6), cellColor, typeNames[m_multisearch.Entries[i / 6].type], ALIGNED_CENTER);
    }
    else if ((i % 6) == 4)
    {
      if ((M_ENTRY.mode != SEARCH_MODE_POINTER) && (M_ENTRY.mode != SEARCH_MODE_NOT_POINTER))
        Gui::drawTextAligned(font20, c5, 160 + linegape * (1 + i / 6), cellColor, _getValueDisplayString(m_multisearch.Entries[i / 6].value1, m_multisearch.Entries[i / 6].type).c_str(), ALIGNED_CENTER);
    }
    else if ((i % 6) == 5)
    {
      if ((m_multisearch.Entries[i / 6].mode == SEARCH_MODE_RANGE) && (M_ENTRY.type != SEARCH_TYPE_POINTER))
        Gui::drawTextAligned(font20, c6, 160 + linegape * (1 + i / 6), cellColor, _getValueDisplayString(m_multisearch.Entries[i / 6].value2, m_multisearch.Entries[i / 6].type).c_str(), ALIGNED_CENTER);
    }
  }
  // if (m_selectedEntry == 1)
  //   Gui::drawRectangled(Gui::g_framebuffer_width / 2 - 155, 345, 310, 90, currTheme.highlightColor);
  // if (m_searchType != SEARCH_TYPE_NONE && m_searchMode != SEARCH_MODE_NONE && m_searchRegion != SEARCH_REGION_NONE)
  // {
  //   Gui::drawRectangled(Gui::g_framebuffer_width / 2 - 150, 350, 300, 80, currTheme.selectedColor);
  //   Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, 375, currTheme.backgroundColor, "Search Now!", ALIGNED_CENTER);
  // }
  // else
  // {
  //   Gui::drawRectangled(Gui::g_framebuffer_width / 2 - 150, 350, 300, 80, currTheme.selectedButtonColor);
  //   Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, 375, currTheme.separatorColor, "Search Now!", ALIGNED_CENTER);
  // }
  Gui::drawTextAligned(font14, Gui::g_framebuffer_width / 2, 520, currTheme.textColor, "Set the value(s) you want to search for. Put the cursor on the target and press \uE0A6 + \uE0B3 to mark it as target. Press \uE0B3 to start the search\n"
                                                                                       "Each line you enable will be used to narrow down the target. Pointer is 64bit values that falls in the range of either main or heap.\n"
                                                                                       "Move the cursor to the field you want to modify. Press \uE0A4 \uE0A5 to modify. Press \uE0A0 to edit numeric Values.\n"
                                                                                       "Use \uE0A6 + \uE0A0 to edit label. Use \uE0A2 to toggle on/off. Use \uE0A6 + \uE0A2 to toggle Hex mode. Use \uE0A3 to jump cursor to value1.\n"
                                                                                       "Press \uE0A7 to jump to target value1. Press \uE0A1 to exit this screen.",
                       ALIGNED_CENTER);
  //  "Press quick set keys to change the search mode \uE0AD SAME \uE0AC DIFF \uE0AB ++ \uE0AE -- \uE0B3 A..B \uE0B4 ==/!=\n"
  //  "If you search type is floating point \uE0A5 negate the number. \uE0C4 cycle float type \uE0C5 presets \uE0A3 cycle integer type",
}
// new dev
void GuiCheats::drawSEARCH_pickjump()
{
  std::stringstream ss;
  if (m_searchMenuLocation != SEARCH_pickjump)
    return;
  Gui::drawRectangled(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, Gui::makeColor(0x00, 0x00, 0x00, 0xA0));
  // Gui::drawRectangle(50, 50, Gui::g_framebuffer_width - 100, Gui::g_framebuffer_height - 100, currTheme.backgroundColor);
  Gui::drawRectangle(0, 50, Gui::g_framebuffer_width, Gui::g_framebuffer_height , currTheme.backgroundColor);
  Gui::drawRectangle(100, 135, Gui::g_framebuffer_width - 200, 1, currTheme.textColor);
  {
      // static const char *const regionNames[] = {"HEAP", "MAIN", "HEAP + MAIN", "RAM", "  "};
      ss.str("");
      ss << "\uE132   Pick Source for JumpBack";
      // ss << "   [ " << regionNames[m_searchRegion] << " ]";
      ss << " line = " << m_selectedJumpSource + m_fromto32_offset + 1 << " / " << m_fromto32_size;
  }
  Gui::drawText(font24, 120, 70, currTheme.textColor, ss.str().c_str());
  ss.str("");
  color_t cellColor;
  // Gui::drawTextAligned(font20, c0, labelline, currTheme.textColor, "LABEL", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, c1, labelline, currTheme.textColor, "OFFSET", ALIGNED_CENTER);
  // Gui::drawTextAligned(font20, c2, labelline, currTheme.textColor, "On/OFF", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, c3, labelline, currTheme.textColor, "Source", ALIGNED_CENTER);
  // Gui::drawTextAligned(font20, c4, labelline, currTheme.textColor, "TYPE", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, c5, labelline, currTheme.textColor, "Target", ALIGNED_CENTER);
  // Gui::drawTextAligned(font20, c6, labelline, currTheme.textColor, "VALUE 2", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 50, Gui::g_framebuffer_height - 50, currTheme.textColor, "\uE0E4 Page UP   \uE0E5 Page Down   \uE0E6+\uE0E2 Rescan pointers      \uE0E3 Pick Source     \uE0E1 Exit", ALIGNED_RIGHT);
  u64 address = m_EditorBaseAddr - (m_EditorBaseAddr % 16) - 0x20 + (m_selectedEntry - 1 - (m_selectedEntry / 5)) * 4 + m_addressmod;
  for (u8 i = 0; i < 15; i++) // 15 Row 
  {
    if (m_selectedJumpSource == i)
      cellColor = currTheme.selectedColor;
    else
      cellColor = currTheme.textColor;
    if (i + m_fromto32_offset >= m_fromto32_size)
      break;

    ss.str("");
    ss << std::uppercase << std::hex << std::setfill('0') << std::setw(3) << address - (m_fromto32[i + m_fromto32_offset].to + m_heapBaseAddr)  ;
    Gui::drawTextAligned(font20, c1, 160 + linegape * (1 + i), cellColor, ss.str().c_str(), ALIGNED_CENTER);

    ss.str("");
    if (m_fromto32[i + m_fromto32_offset].from == 0)
    {
      u32 temp_offset = get_main_offset32(m_fromto32[i + m_fromto32_offset].to);
      if (m_selectedJumpSource == i) m_selectedJumpSource_offset = temp_offset;
      ss << "Main+" << std::uppercase << std::hex << std::setfill('0') << std::setw(8) << temp_offset;
    }
    else
      ss << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << m_heapBaseAddr + m_fromto32[i + m_fromto32_offset].from;
    Gui::drawTextAligned(font20, c3, 160 + linegape * (1 + i), cellColor, ss.str().c_str(), ALIGNED_CENTER);

    ss.str("");
    ss << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << m_heapBaseAddr + m_fromto32[i + m_fromto32_offset].to;
    Gui::drawTextAligned(font20, c5, 160 + linegape * (1 + i), cellColor, ss.str().c_str(), ALIGNED_CENTER);
  }
}
//
void GuiCheats::MTsearchMemoryAddressesPrimary(Debugger *debugger, searchValue_t searchValue1, searchValue_t searchValue2, searchType_t searchType, searchMode_t searchMode, searchRegion_t searchRegion, MemoryDump **displayDump, std::vector<MemoryInfo> memInfos)
{
  (*displayDump) = new MemoryDump(EDIZON_DIR "/memdump1.dat", DumpType::ADDR, true);
  (*displayDump)->setBaseAddresses(m_addressSpaceBaseAddr, m_heapBaseAddr, m_mainBaseAddr, m_heapSize, m_mainSize);
  m_use_range = (searchMode == SEARCH_MODE_RANGE);
  (*displayDump)->setSearchParams(searchType, searchMode, searchRegion, searchValue1, searchValue2, m_use_range);
  MemoryDump *helperDump = new MemoryDump(EDIZON_DIR "/memdump1a.dat", DumpType::HELPER, true); // has address, size, count for fetching buffer from memory
  MemoryDump *newdataDump = new MemoryDump(EDIZON_DIR "/datadump2.dat", DumpType::DATA, true);
  MemoryDump *newstringDump = new MemoryDump(EDIZON_DIR "/stringdump.csv", DumpType::DATA, true); // to del when not needed
  helperinfo_t helperinfo;
  helperinfo.count = 0;
  bool ledOn = false;
  time_t unixTime1 = time(NULL);
  printf("%s%lx\n", "Start Time primary search", unixTime1);
  printf("value1=%lx value2=%lx typesize=%d\n", searchValue1._u64, searchValue2._u64, dataTypeSizes[searchType]);
  for (MemoryInfo meminfo : memInfos)
  {
    if (searchRegion == SEARCH_REGION_HEAP && meminfo.type != MemType_Heap)
      continue;
    else if (searchRegion == SEARCH_REGION_MAIN &&
             (((meminfo.type != MemType_CodeWritable && meminfo.type != MemType_CodeMutable && meminfo.type != MemType_CodeStatic) || !(meminfo.addr < m_mainend && meminfo.addr >= m_mainBaseAddr))))
      continue;
    else if (searchRegion == SEARCH_REGION_HEAP_AND_MAIN &&
             (((meminfo.type != MemType_Heap && meminfo.type != MemType_CodeWritable && meminfo.type != MemType_CodeMutable)) || !((meminfo.addr < m_heapEnd && meminfo.addr >= m_heapBaseAddr) || (meminfo.addr < m_mainend && meminfo.addr >= m_mainBaseAddr))))
      continue;
    else if ( (meminfo.perm & Perm_Rw) != Perm_Rw) //searchRegion == SEARCH_REGION_RAM &&
      continue;
    setLedState(ledOn);
    ledOn = !ledOn;
    u64 offset = 0;
    u64 bufferSize = MAX_BUFFER_SIZE; // consider to increase from 10k to 1M (not a big problem)
    u8 *buffer = new u8[MAX_BUFFER_SIZE + m_multisearch.size];
    while (offset < meminfo.size)
    {
      if (meminfo.size - offset < bufferSize)
        bufferSize = meminfo.size - offset;
      debugger->readMemory(buffer, bufferSize + m_multisearch.size, meminfo.addr + offset);
      searchValue_t realValue = {0};
      u32 inc_i;
      if (Config::getConfig()->extra_value)
      {
        inc_i = M_ALIGNMENT;
      }
      else if (searchMode == SEARCH_MODE_POINTER)
        inc_i = 4;
      else
        inc_i = dataTypeSizes[searchType];
      for (u32 i = 0; i < bufferSize; i += inc_i)
      {
        if (_check_extra_not_OK(buffer, i))
        {
          continue; // if not match let's continue
        }
        u64 address = meminfo.addr + offset + m_multisearch.target_offset + i;
        memset(&realValue, 0, 8);
        if (searchMode == SEARCH_MODE_POINTER && m_32bitmode)
          memcpy(&realValue, buffer + m_multisearch.target_offset + i, 4);
        else
          memcpy(&realValue, buffer + m_multisearch.target_offset + i, dataTypeSizes[searchType]);
        if (Config::getConfig()->exclude_ptr_candidates && searchMode != SEARCH_MODE_POINTER)
        {
          searchValue_t ptr_address;
          memcpy(&ptr_address, buffer + m_multisearch.target_offset + i - m_multisearch.target_offset % 8, 8);
          if (((ptr_address._u64 >= m_mainBaseAddr) && (ptr_address._u64 <= (m_mainend))) || ((ptr_address._u64 >= m_heapBaseAddr) && (ptr_address._u64 <= (m_heapEnd))))
            continue;
        }
        switch (searchMode)
        {
        case SEARCH_MODE_EQ:
          if (realValue._s64 == searchValue1._s64)
          {
            (*displayDump)->addData((u8 *)&address, sizeof(u64));
            helperinfo.count++;
          }
          break;
        case SEARCH_MODE_NEQ:
          if (realValue._s64 != searchValue1._s64)
          {
            (*displayDump)->addData((u8 *)&address, sizeof(u64));
            helperinfo.count++;
          }
          break;
        case SEARCH_MODE_GT:
          if (searchType & (SEARCH_TYPE_SIGNED_8BIT | SEARCH_TYPE_SIGNED_16BIT | SEARCH_TYPE_SIGNED_32BIT | SEARCH_TYPE_SIGNED_64BIT | SEARCH_TYPE_FLOAT_32BIT | SEARCH_TYPE_FLOAT_64BIT))
          {
            if (realValue._s64 > searchValue1._s64)
            {
              (*displayDump)->addData((u8 *)&address, sizeof(u64));
              helperinfo.count++;
            }
          }
          else
          {
            if (realValue._u64 > searchValue1._u64)
            {
              (*displayDump)->addData((u8 *)&address, sizeof(u64));
              helperinfo.count++;
            }
          }
          break;
        case SEARCH_MODE_DIFFA:
          if (searchType & (SEARCH_TYPE_SIGNED_8BIT | SEARCH_TYPE_SIGNED_16BIT | SEARCH_TYPE_SIGNED_32BIT | SEARCH_TYPE_SIGNED_64BIT | SEARCH_TYPE_FLOAT_32BIT | SEARCH_TYPE_FLOAT_64BIT))
          {
            if (realValue._s64 >= searchValue1._s64)
            {
              (*displayDump)->addData((u8 *)&address, sizeof(u64));
              helperinfo.count++;
            }
          }
          else
          {
            if (realValue._u64 >= searchValue1._u64)
            {
              (*displayDump)->addData((u8 *)&address, sizeof(u64));
              helperinfo.count++;
            }
          }
          break;
        case SEARCH_MODE_LT:
          if (searchType & (SEARCH_TYPE_SIGNED_8BIT | SEARCH_TYPE_SIGNED_16BIT | SEARCH_TYPE_SIGNED_32BIT | SEARCH_TYPE_SIGNED_64BIT | SEARCH_TYPE_FLOAT_32BIT | SEARCH_TYPE_FLOAT_64BIT))
          {
            if (realValue._s64 < searchValue1._s64)
            {
              (*displayDump)->addData((u8 *)&address, sizeof(u64));
              helperinfo.count++;
            }
          }
          else
          {
            if (realValue._u64 < searchValue1._u64)
            {
              (*displayDump)->addData((u8 *)&address, sizeof(u64));
              helperinfo.count++;
            }
          }
          break;
        case SEARCH_MODE_SAMEA:
          if (searchType & (SEARCH_TYPE_SIGNED_8BIT | SEARCH_TYPE_SIGNED_16BIT | SEARCH_TYPE_SIGNED_32BIT | SEARCH_TYPE_SIGNED_64BIT | SEARCH_TYPE_FLOAT_32BIT | SEARCH_TYPE_FLOAT_64BIT))
          {
            if (realValue._s64 <= searchValue1._s64)
            {
              (*displayDump)->addData((u8 *)&address, sizeof(u64));
              helperinfo.count++;
            }
          }
          else
          {
            if (realValue._u64 <= searchValue1._u64)
            {
              (*displayDump)->addData((u8 *)&address, sizeof(u64));
              helperinfo.count++;
            }
          }
          break;
        case SEARCH_MODE_RANGE:
          if (realValue._s64 >= searchValue1._s64 && realValue._s64 <= searchValue2._s64)
          {
            (*displayDump)->addData((u8 *)&address, sizeof(u64));
            newdataDump->addData((u8 *)&realValue, sizeof(u64));
            helperinfo.count++;
          }
          break;
        case SEARCH_MODE_POINTER: //m_heapBaseAddr, m_mainBaseAddr, m_heapSize, m_mainSize
          if ((realValue._u64 != 0))
            if (((realValue._u64 >= m_mainBaseAddr) && (realValue._u64 <= (m_mainend))) || ((realValue._u64 >= m_heapBaseAddr) && (realValue._u64 <= (m_heapEnd))))
            {
              if ((m_forwarddump) && (address > realValue._u64) && (meminfo.type == MemType_Heap))
                break;
              (*displayDump)->addData((u8 *)&address, sizeof(u64));
              newdataDump->addData((u8 *)&realValue, sizeof(u64));
              helperinfo.count++;
            }
          break;
        case SEARCH_MODE_NONE:
        case SEARCH_MODE_SAME:
        case SEARCH_MODE_DIFF:
        case SEARCH_MODE_INC:
        case SEARCH_MODE_DEC:
        case SEARCH_MODE_NOT_POINTER:
          printf("search mode non !");
          break;
        }
      }
      if (helperinfo.count != 0)
      {
        helperinfo.address = meminfo.addr + offset;
        helperinfo.size = bufferSize;
        helperDump->addData((u8 *)&helperinfo, sizeof(helperinfo));
        helperinfo.count = 0;
      } // must be after write
      offset += bufferSize;
    }
    delete[] buffer;
  }
  setLedState(false);
  time_t unixTime2 = time(NULL);
  printf("%s%lx\n", "Stop Time ", unixTime2);
  printf("%s%ld\n", "Stop Time ", unixTime2 - unixTime1);
  (*displayDump)->flushBuffer();
  newdataDump->flushBuffer();
  helperDump->flushBuffer();
  delete helperDump;
  delete newdataDump;
  newstringDump->flushBuffer(); // temp
  delete newstringDump;         //
}
// end new dev
void GuiCheats::EditExtraSearchValues_input(u32 kdown, u32 kheld)
{
#define M_ENTRY m_multisearch.Entries[m_selectedEntry / 6]
#define M_ENTRY_TOGGLE       \
  if (M_ENTRY.on == OFF)     \
    M_ENTRY.on = ON;         \
  else if (M_ENTRY.on == ON) \
    M_ENTRY.on = OFF;
  std::stringstream ss;
  if (kdown & KEY_B && !(kheld & KEY_ZL))
  {
    GuiCheats::save_multisearch_setup();
    m_selectedEntry = m_selectedEntrySave;
    m_searchMenuLocation = SEARCH_NONE;
  }
  else if (kdown & KEY_ZR && !(kheld & KEY_ZL))
  {
    m_selectedEntry = m_multisearch.target * 6 + 4;
  }
  else if (kdown & KEY_B && (kheld & KEY_ZL))
  {
    GuiCheats::save_multisearch_setup();
  }
  else if (kdown & KEY_PLUS && (kheld & KEY_ZL))
  {
    M_TARGET.on = ON;
    m_multisearch.target = m_selectedEntry / 6;
    M_TARGET.on = TARGET;
  }
  else if (kdown & KEY_PLUS && !(kheld & KEY_ZL))
  {
    // M_TARGET.on = OFF;
    // m_multisearch.target = m_selectedEntry / 6;
    // M_TARGET.on = TARGET;
    GuiCheats::save_multisearch_setup();
    m_searchType = M_TARGET.type;
    m_searchMode = M_TARGET.mode;
    m_searchValue[0] = M_TARGET.value1;
    m_searchValue[1] = M_TARGET.value2; 
    // START SEARCH
    {
      if (m_searched)
      {
        if (m_memoryDump1 == nullptr)
          m_memoryDump->setSearchParams(m_searchType, m_searchMode, m_searchRegion, m_searchValue[0], m_searchValue[1], m_use_range);
        else
          m_memoryDump1->setSearchParams(m_searchType, m_searchMode, m_searchRegion, m_searchValue[0], m_searchValue[1], m_use_range);
        (new Snackbar("Already did one search for this session, relaunch to do another"))->show();
      }
      else
      {
        m_searched = true;
        (new MessageBox("Traversing title memory.\n \nThis may take a while...", MessageBox::NONE))->show();
        requestDraw();
        overclockSystem(true);
        if (m_searchMode == SEARCH_MODE_POINTER)
          m_searchType = SEARCH_TYPE_UNSIGNED_64BIT;
        if (m_memoryDump1 != nullptr)
        {
          updatebookmark(true, false, true);
          m_memoryDump = m_memoryDumpBookmark;
        }
        else if (m_searchMode == SEARCH_MODE_SAME || m_searchMode == SEARCH_MODE_DIFF || m_searchMode == SEARCH_MODE_INC || m_searchMode == SEARCH_MODE_DEC || m_searchMode == SEARCH_MODE_DIFFA || m_searchMode == SEARCH_MODE_SAMEA)
        {
          if (m_memoryDump->size() == 0)
          {
            delete m_memoryDump;
            m_use_range = false;
            if (Config::getConfig()->enabletargetedscan && m_targetmemInfos.size() != 0)
              GuiCheats::searchMemoryValuesPrimary(m_debugger, m_searchType, m_searchMode, m_searchRegion, &m_memoryDump, m_targetmemInfos);
            else
              GuiCheats::searchMemoryValuesPrimary(m_debugger, m_searchType, m_searchMode, m_searchRegion, &m_memoryDump, m_memoryInfo);
            printf("%s%lx\n", "Dump Size = ", m_memoryDump->size());
          }
          else if (m_memoryDump->getDumpInfo().dumpType == DumpType::DATA)
          {
            printf("%s%lx\n", "Dump Size = ", m_memoryDump->size());
            if (Config::getConfig()->enabletargetedscan && m_targetmemInfos.size() != 0)
              GuiCheats::searchMemoryValuesSecondary(m_debugger, m_searchType, m_searchMode, m_searchRegion, &m_memoryDump, m_targetmemInfos);
            else
              GuiCheats::searchMemoryValuesSecondary(m_debugger, m_searchType, m_searchMode, m_searchRegion, &m_memoryDump, m_memoryInfo);
            delete m_memoryDump;
            std::string s = m_edizon_dir + "/memdump1.dat";
            REPLACEFILE(EDIZON_DIR "/memdump3.dat", s.c_str());
            m_memoryDump = new MemoryDump(EDIZON_DIR "/memdump1.dat", DumpType::ADDR, false);
          }
          else if (m_memoryDump->getDumpInfo().dumpType == DumpType::ADDR)
          {
            if (m_searchMode == SEARCH_MODE_DIFFA || m_searchMode == SEARCH_MODE_SAMEA)
            {
              if (Config::getConfig()->enabletargetedscan && m_targetmemInfos.size() != 0)
                GuiCheats::searchMemoryValuesTertiary(m_debugger, m_searchValue[0], m_searchValue[1], m_searchType, m_searchMode, m_searchRegion, m_use_range, &m_memoryDump, m_targetmemInfos);
              else
                GuiCheats::searchMemoryValuesTertiary(m_debugger, m_searchValue[0], m_searchValue[1], m_searchType, m_searchMode, m_searchRegion, m_use_range, &m_memoryDump, m_memoryInfo);
              delete m_memoryDump;
              std::string s = m_edizon_dir + "/memdump1.dat";
              REPLACEFILE(EDIZON_DIR "/memdump3.dat", s.c_str());
              s = m_edizon_dir + "/memdump1a.dat";
              REPLACEFILE(EDIZON_DIR "/memdump3a.dat", s.c_str());
              m_memoryDump = new MemoryDump(EDIZON_DIR "/memdump1.dat", DumpType::ADDR, false);
              s = m_edizon_dir + "/datadump2.dat";
              REPLACEFILE(EDIZON_DIR "/datadump4.dat", s.c_str());
              REPLACEFILE(EDIZON_DIR "/datadumpAa.dat", EDIZON_DIR "/datadumpA.dat")
              REPLACEFILE(EDIZON_DIR "/datadumpBa.dat", EDIZON_DIR "/datadumpB.dat");
            }
            else
            {
              m_nothingchanged = false;
              GuiCheats::searchMemoryAddressesSecondary2(m_debugger, m_searchValue[0], m_searchValue[1], m_searchType, m_searchMode, &m_memoryDump);
              if (m_nothingchanged == false)
              {
                std::string s = m_edizon_dir + "/memdump1a.dat";
                REPLACEFILE(EDIZON_DIR "/memdump3a.dat", s.c_str());
              }
            }
          }
        }
        else
        {
          if (m_memoryDump->size() == 0)
          {
            delete m_memoryDump;
            if (Config::getConfig()->enabletargetedscan && m_targetmemInfos.size() != 0)
              GuiCheats::MTsearchMemoryAddressesPrimary(m_debugger, m_searchValue[0], m_searchValue[1], m_searchType, m_searchMode, m_searchRegion, &m_memoryDump, m_targetmemInfos);
            else
              GuiCheats::MTsearchMemoryAddressesPrimary(m_debugger, m_searchValue[0], m_searchValue[1], m_searchType, m_searchMode, m_searchRegion, &m_memoryDump, m_memoryInfo);
          }
          else
          {
            m_nothingchanged = false;
            GuiCheats::searchMemoryAddressesSecondary(m_debugger, m_searchValue[0], m_searchValue[1], m_searchType, m_searchMode, m_use_range, &m_memoryDump);
            if (m_nothingchanged == false)
            {
              std::string s = m_edizon_dir + "/memdump1a.dat";
              REPLACEFILE(EDIZON_DIR "/memdump3a.dat", s.c_str());
            }
          }
        }
        overclockSystem(false);
        Gui::g_currMessageBox->hide();
        m_searchMenuLocation = SEARCH_NONE;
      }
    }
  }
  else if (kdown & KEY_MINUS && !(kheld & KEY_ZL))
  {
    for (u8 i = 0; i < 10; i++)
    {
      m_multisearch.Entries[i].offset = i * 0x8;
      m_multisearch.Entries[i].on = OFF;
      m_multisearch.Entries[i].type = SEARCH_TYPE_UNSIGNED_64BIT;
      m_multisearch.Entries[i].mode = SEARCH_MODE_POINTER;
      m_multisearch.Entries[i].value1._u64 = 0;
      m_multisearch.Entries[i].value2._u64 = 0;
      ss.str("");
      ss << "" << std::dec << (u16)i + 1;
      strcpy(m_multisearch.Entries[i].label, ss.str().c_str());
    }
    m_multisearch.Entries[0].on = TARGET;
    m_multisearch.target = 0;
  }
  else if (kdown & KEY_MINUS && (kheld & KEY_ZL))
  {
    m_multisearch.Entries[0].on = TARGET;
    m_multisearch.target = 0;
    u8 i = 0;
    m_multisearch.Entries[i].offset = i * 0x8;
    m_multisearch.Entries[i].on = OFF;
    m_multisearch.Entries[i].type = SEARCH_TYPE_UNSIGNED_32BIT;
    m_multisearch.Entries[i].mode = SEARCH_MODE_EQ;
    m_multisearch.Entries[i].value1._u64 = 0;
    m_multisearch.Entries[i].value2._u64 = 0;
    ss.str("");
    ss << "Item " << std::dec << (u16)i + 1;

    strcpy(m_multisearch.Entries[i].label, ss.str().c_str());
    i = 1;
    m_multisearch.Entries[i].offset = i * 0x8;
    m_multisearch.Entries[i].on = OFF;
    m_multisearch.Entries[i].type = SEARCH_TYPE_FLOAT_32BIT;
    m_multisearch.Entries[i].mode = SEARCH_MODE_RANGE;
    m_multisearch.Entries[i].value1._f32 = 0.1;
    m_multisearch.Entries[i].value2._f32 = 1000;
    ss.str("");
    ss << "Range 32+";

    strcpy(m_multisearch.Entries[i].label, ss.str().c_str());
    i = 2;
    m_multisearch.Entries[i].offset = i * 0x8;
    m_multisearch.Entries[i].on = OFF;
    m_multisearch.Entries[i].type = SEARCH_TYPE_FLOAT_32BIT;
    m_multisearch.Entries[i].mode = SEARCH_MODE_RANGE;
    m_multisearch.Entries[i].value1._f32 = -0.1;
    m_multisearch.Entries[i].value2._f32 = -1000;
    ss.str("");
    ss << "Range 32-";

    strcpy(m_multisearch.Entries[i].label, ss.str().c_str());
    i = 3;
    m_multisearch.Entries[i].offset = i * 0x8;
    m_multisearch.Entries[i].on = OFF;
    m_multisearch.Entries[i].type = SEARCH_TYPE_FLOAT_64BIT;
    m_multisearch.Entries[i].mode = SEARCH_MODE_EQ;
    m_multisearch.Entries[i].value1._f64 = 0;
    m_multisearch.Entries[i].value2._f64 = 0;
    ss.str("");
    ss << "Double";

    strcpy(m_multisearch.Entries[i].label, ss.str().c_str());
    i = 4;
    m_multisearch.Entries[i].offset = i * 0x8;
    m_multisearch.Entries[i].on = OFF;
    m_multisearch.Entries[i].type = SEARCH_TYPE_FLOAT_64BIT;
    m_multisearch.Entries[i].mode = SEARCH_MODE_RANGE;
    m_multisearch.Entries[i].value1._f64 = 0.1;
    m_multisearch.Entries[i].value2._f64 = 1000;
    ss.str("");
    ss << "Range 64+";

    strcpy(m_multisearch.Entries[i].label, ss.str().c_str());
    i = 5;
    m_multisearch.Entries[i].offset = i * 0x8;
    m_multisearch.Entries[i].on = OFF;
    m_multisearch.Entries[i].type = SEARCH_TYPE_FLOAT_64BIT;
    m_multisearch.Entries[i].mode = SEARCH_MODE_RANGE;
    m_multisearch.Entries[i].value1._f64 = -0.1;
    m_multisearch.Entries[i].value2._f64 = -1000;
    ss.str("");
    ss << "Range 64-";

    strcpy(m_multisearch.Entries[i].label, ss.str().c_str());

    i = 6;
    m_multisearch.Entries[i].offset = i * 0x8;
    m_multisearch.Entries[i].on = OFF;
    m_multisearch.Entries[i].type = SEARCH_TYPE_UNSIGNED_64BIT;
    m_multisearch.Entries[i].mode = SEARCH_MODE_RANGE;
    m_multisearch.Entries[i].value1._u64 = m_heapBaseAddr;
    m_multisearch.Entries[i].value2._u64 = m_heapEnd;
    ss.str("");
    ss << "Heap " << std::dec << (u16)i + 1;
    strcpy(m_multisearch.Entries[i].label, ss.str().c_str());

    i = 7;
    m_multisearch.Entries[i].offset = i * 0x8;
    m_multisearch.Entries[i].on = OFF;
    m_multisearch.Entries[i].type = SEARCH_TYPE_UNSIGNED_64BIT;
    m_multisearch.Entries[i].mode = SEARCH_MODE_RANGE;
    m_multisearch.Entries[i].value1._u64 = m_mainBaseAddr;
    m_multisearch.Entries[i].value2._u64 = m_mainend;
    ss.str("");
    ss << "Main " << std::dec << (u16)i + 1;
    strcpy(m_multisearch.Entries[i].label, ss.str().c_str());
  }
  else if (kdown & KEY_X && !(kheld & KEY_ZL))
  {
    M_ENTRY_TOGGLE
    // m_multisearch.Entries[m_selectedEntry / 6].on = !m_multisearch.Entries[m_selectedEntry / 6].on;
  }
  else if (kdown & KEY_R && !(kheld & KEY_ZL))
  {
    switch (m_selectedEntry % 6)
    {
    case 0:
      m_multisearch.Entries[m_selectedEntry / 6].offset++;
      break;
    case 1:
      M_ENTRY_TOGGLE
      // m_multisearch.Entries[m_selectedEntry / 6].on = !m_multisearch.Entries[m_selectedEntry / 6].on;
      break;
    case 2:
      if (M_ENTRY.mode == SEARCH_MODE_EQ)
        M_ENTRY.mode = SEARCH_MODE_NOT_POINTER;
      else
        M_ENTRY.mode = SEARCH_MODE_EQ;
      break;
    case 3:
      if (M_ENTRY.type == SEARCH_TYPE_UNSIGNED_32BIT)
      {
        M_ENTRY.type = SEARCH_TYPE_UNSIGNED_64BIT;
      }
      else if (M_ENTRY.type == SEARCH_TYPE_UNSIGNED_64BIT)
      {
        M_ENTRY.type = SEARCH_TYPE_UNSIGNED_8BIT;
      }
      else if (M_ENTRY.type == SEARCH_TYPE_UNSIGNED_8BIT)
      {
        M_ENTRY.type = SEARCH_TYPE_UNSIGNED_16BIT;
      }
      else
        M_ENTRY.type = SEARCH_TYPE_UNSIGNED_32BIT;
      break;
    case 4:
      switch (M_ENTRY.type)
      {
      case SEARCH_TYPE_FLOAT_32BIT:
        M_ENTRY.value1._f32++;
        break;
      case SEARCH_TYPE_FLOAT_64BIT:
        M_ENTRY.value1._f64++;
        break;
      default:
        M_ENTRY.value1._u64++;
      }
      break;
    case 5:
      switch (M_ENTRY.type)
      {
      case SEARCH_TYPE_FLOAT_32BIT:
        M_ENTRY.value2._f32++;
        break;
      case SEARCH_TYPE_FLOAT_64BIT:
        M_ENTRY.value2._f64++;
        break;
      default:
        M_ENTRY.value2._u64++;
      }
      break;
    }
  }
  else if (kdown & KEY_L && !(kheld & KEY_ZL))
  {
    switch (m_selectedEntry % 6)
    {
    case 0:
      m_multisearch.Entries[m_selectedEntry / 6].offset--;
      break;
    case 1:
      M_ENTRY_TOGGLE
      // m_multisearch.Entries[m_selectedEntry / 6].on = !m_multisearch.Entries[m_selectedEntry / 6].on;
      break;
    case 2:
      if (M_ENTRY.mode == SEARCH_MODE_POINTER)
        M_ENTRY.mode = SEARCH_MODE_RANGE;
      else
      {
        M_ENTRY.mode = SEARCH_MODE_POINTER;
        M_ENTRY.type = SEARCH_TYPE_UNSIGNED_64BIT;
      }
      break;
    case 3:
      if (m_multisearch.Entries[m_selectedEntry / 6].type == SEARCH_TYPE_FLOAT_32BIT)
      {
        m_multisearch.Entries[m_selectedEntry / 6].type = SEARCH_TYPE_FLOAT_64BIT;
      }
      else
      {
        m_multisearch.Entries[m_selectedEntry / 6].type = SEARCH_TYPE_FLOAT_32BIT;
      }
      break;
    case 4:
      switch (M_ENTRY.type)
      {
      case SEARCH_TYPE_FLOAT_32BIT:
        M_ENTRY.value1._f32--;
        break;
      case SEARCH_TYPE_FLOAT_64BIT:
        M_ENTRY.value1._f64--;
        break;
      default:
        M_ENTRY.value1._u64--;
      }
      break;
    case 5:
      switch (M_ENTRY.type)
      {
      case SEARCH_TYPE_FLOAT_32BIT:
        M_ENTRY.value2._f32--;
        break;
      case SEARCH_TYPE_FLOAT_64BIT:
        M_ENTRY.value2._f64--;
        break;
      default:
        M_ENTRY.value2._u64--;
      }
      break;
    }
  }
  else if (kdown & KEY_Y && !(kheld & KEY_ZL)) // Jump cursor to value1
  {
    m_selectedEntry = m_selectedEntry - m_selectedEntry %6 + 4;
  }
  else if (kdown & KEY_Y && (kheld & KEY_ZL)) // Jump cursor to offset
  {
    m_selectedEntry = m_selectedEntry - m_selectedEntry %6 ;
  }
  else if (kdown & KEY_X && (kheld & KEY_ZL)) // Toggle HEX mode
  {
    if (m_searchValueFormat == FORMAT_HEX)
      m_searchValueFormat = FORMAT_DEC;
    else
      m_searchValueFormat = FORMAT_HEX;
  }
  else if (kdown & KEY_A && !(kheld & KEY_ZL))
  {
    switch (m_selectedEntry % 6)
    {
    case 0:
    {
      searchValue_t offset = {0};
      offset._u16 = M_ENTRY.offset;
      if (m_searchValueFormat != FORMAT_HEX)
      {
        m_searchValueFormat = FORMAT_HEX;
        M_ENTRY.offset = _get_entry(offset, SEARCH_TYPE_UNSIGNED_16BIT)._u16;
        m_searchValueFormat = FORMAT_DEC;
      }
      else
      {
        M_ENTRY.offset = _get_entry(offset, SEARCH_TYPE_UNSIGNED_16BIT)._u16;
      }
    }
    break;
    case 1:
    case 2:
    case 3:
    case 4:
      M_ENTRY.value1 = _get_entry(M_ENTRY.value1, M_ENTRY.type);
      break;
    case 5:
      M_ENTRY.value2 = _get_entry(M_ENTRY.value2, M_ENTRY.type);
      break;
    }
  }
  else if (kdown & KEY_A && (kheld & KEY_ZL))
  {
    ss.str("");
    ss << M_ENTRY.label;
    if (!Gui::requestKeyboardInput("Enter Label", "Enter Label to this item .", ss.str().c_str(), SwkbdType_QWERTY, M_ENTRY.label, 14))
    {
      ss.str("");
      ss << "Item " << std::dec << (m_selectedEntry / 6) + 1;
      strcpy(M_ENTRY.label, ss.str().c_str());
    }
  }
  else if (kdown & KEY_UP && !(kheld & KEY_ZL))
  {
    if (m_selectedEntry > 5)
      m_selectedEntry -= 6;
  }
  else if (kdown & KEY_DOWN && !(kheld & KEY_ZL))
  {
    if (m_selectedEntry < 54)
      m_selectedEntry += 6;
  }
  else if (kdown & KEY_LEFT && !(kheld & KEY_ZL))
  {
    if (m_selectedEntry % 6 > 0)
      m_selectedEntry--;
  }
  else if (kdown & KEY_RIGHT && !(kheld & KEY_ZL))
  {
    if (m_selectedEntry % 6 < 5)
      m_selectedEntry++;
  }
  if (M_ENTRY.mode == SEARCH_MODE_POINTER || M_ENTRY.mode == SEARCH_MODE_NOT_POINTER)
  {
    if (m_selectedEntry % 6 > 2)
    {
      m_selectedEntry = m_selectedEntry - m_selectedEntry % 6 + 2;
    }
  }
  else if (M_ENTRY.mode != SEARCH_MODE_RANGE)
  {
    if (m_selectedEntry % 6 > 4)
    {
      m_selectedEntry = m_selectedEntry - m_selectedEntry % 6 + 4;
    }
  }
}
void GuiCheats::editor_input(u32 kdown, u32 kheld) //ME2 Key input for memory explorer
{
  if (kdown & KEY_B && kheld & KEY_ZL)
  {
    m_selectedEntry = m_selectedEntrySave;
    m_searchMenuLocation = SEARCH_NONE;
    m_addressmod = 0;
  }
  else if (kdown & KEY_RSTICK && !(kheld & KEY_ZL))
  {
    u64 address = m_EditorBaseAddr - (m_EditorBaseAddr % 16) - 0x20 + (m_selectedEntry - 1 - (m_selectedEntry / 5)) * 4 + m_addressmod;
    searchValue_t value = {0};
    u8 *ram_buffer = new u8[M_ENTRY_MAX * sizeof(u64)];
    u64 startaddress = address - 0x20 - (address % 16);
    m_debugger->readMemory(ram_buffer, M_ENTRY_MAX * sizeof(u64), startaddress);
    for (int i = 0; i < M_ENTRY_MAX; i++)
    {
      if (i == (int)(4 + (address / sizeof(u64)) % 2))
      {
        M_ENTRYi.on = TARGET;
        M_ENTRYi.type = m_searchType;
        M_ENTRYi.mode = SEARCH_MODE_EQ;
        M_ENTRYi.offset = address % 16 + 0x20;
        memcpy(&value, ram_buffer + M_ENTRYi.offset, dataTypeSizes[m_searchType]);
        M_ENTRYi.value1._s64 = value._s64;
      }
      else
      {
        M_ENTRYi.on = ON;
        M_ENTRYi.type = SEARCH_TYPE_UNSIGNED_64BIT;
        value = {0};
        memcpy(&value, ram_buffer + i * sizeof(u64), sizeof(u64));
        if (((value._u64 >= m_mainBaseAddr) && (value._u64 <= (m_mainend))) || ((value._u64 >= m_heapBaseAddr) && (value._u64 <= (m_heapEnd))))
        {
          M_ENTRYi.mode = SEARCH_MODE_POINTER;
          M_ENTRYi.offset = i * sizeof(u64);
        }
        else
        {
          value = {0};
          memcpy(&value, ram_buffer + i * sizeof(u64), sizeof(u32));
          if (value._f32 > 0.001 && value._f32 < 1000000)
          {
            M_ENTRYi.mode = SEARCH_MODE_RANGE;
            M_ENTRYi.type = SEARCH_TYPE_FLOAT_32BIT;
            M_ENTRYi.offset = i * sizeof(u64);
            M_ENTRYi.value1._f32 = 0.001;
            M_ENTRYi.value2._f32 = 1000000;
          }
          else if (value._f32 < -0.001 && value._f32 > -1000000)
          {
            M_ENTRYi.mode = SEARCH_MODE_RANGE;
            M_ENTRYi.type = SEARCH_TYPE_FLOAT_32BIT;
            M_ENTRYi.offset = i * sizeof(u64);
            M_ENTRYi.value1._f32 = -0.001;
            M_ENTRYi.value2._f32 = -1000000;
          }
          else
          {
            value = {0};
            memcpy(&value, ram_buffer + i * sizeof(u64) + sizeof(u32), sizeof(u32));
            if (value._f32 > 0.001 && value._f32 < 1000000)
            {
              M_ENTRYi.mode = SEARCH_MODE_RANGE;
              M_ENTRYi.type = SEARCH_TYPE_FLOAT_32BIT;
              M_ENTRYi.offset = i * sizeof(u64) + sizeof(u32);
              M_ENTRYi.value1._f32 = 0.001;
              M_ENTRYi.value2._f32 = 1000000;
            }
            else if (value._f32 < -0.001 && value._f32 > -1000000)
            {
              M_ENTRYi.mode = SEARCH_MODE_RANGE;
              M_ENTRYi.type = SEARCH_TYPE_FLOAT_32BIT;
              M_ENTRYi.offset = i * sizeof(u64) + sizeof(u32);
              M_ENTRYi.value1._f32 = -0.001;
              M_ENTRYi.value2._f32 = -1000000;
            }
            else
            {
              M_ENTRYi.mode = SEARCH_MODE_NOT_POINTER;
              M_ENTRYi.offset = i * sizeof(u64);
            }
          }
        }
      }
    }
    delete [] ram_buffer;
    (new Snackbar("Multi search setup created!"))->show();
    GuiCheats::save_multisearch_setup();
  }
  else if (kdown & KEY_B && !(kheld & KEY_ZL))
  {
    // if (m_jump_stack_index > 0)
    // {
    //   m_jump_stack_index--;
    //   // m_EditorBaseAddr = m_jump_stack[m_jump_stack_index];
    //   m_selectedEntry = (m_EditorBaseAddr % 16) / 4 + 11;
    // }
    // else
    // {
    //   (new Snackbar("Jump Stack empty!"))->show();
    // }
    {
      if (m_EditorBaseAddr >= m_mainBaseAddr && m_EditorBaseAddr <= m_mainend)
      {
        (new Snackbar("Already at Main!"))->show();
      }
      else
      {
        u64 address = m_EditorBaseAddr - (m_EditorBaseAddr % 16) - 0x20 + (m_selectedEntry - 1 - (m_selectedEntry / 5)) * 4 + m_addressmod;
        GuiCheats::prep_pointersearch(m_debugger, m_memoryInfo);
        GuiCheats::prep_backjump_stack(address);
        m_searchMenuLocation = SEARCH_pickjump;
      }
      // pick key need to do this
      // {
      //   u64 address = m_EditorBaseAddr - (m_EditorBaseAddr % 16) - 0x20 + (m_selectedEntry - 1 - (m_selectedEntry / 5)) * 4 + m_addressmod;
      //   m_bookmark.pointer.offset[m_z] = m_fromto32(m_pick).to - address + m_heapBaseAddr;
      //   m_z++;
      //   m_jump_stack[m_z].from = m_fromto32(m_pick).from + m_heapBaseAddr;
      //   m_jump_stack[m_z].to = m_fromto32(m_pick).to + m_heapBaseAddr;
      //   m_EditorBaseAddr = m_jump_stack[m_z].from;
      //   m_selectedEntry = (m_EditorBaseAddr % 16) / 4 + 11;
      //   m_searchType = SEARCH_TYPE_UNSIGNED_32BIT;
      //   m_addressmod = 0;
      //   place head here;
      // }
    }
  }
  else if (kdown & KEY_R && !(kheld & KEY_ZL))
  {
    if (m_z > m_bookmark.pointer.depth - m_depth_count)
    {
      // if (m_jump_stack[m_z].to !=0)
      m_EditorBaseAddr = m_jump_stack[m_z].to;
      m_selectedEntry = (m_EditorBaseAddr % 16) / 4 + 11;
      m_addressmod = 0;
      m_searchType = SEARCH_TYPE_UNSIGNED_32BIT;
      m_z--;
    }
    else
    {
      m_EditorBaseAddr = m_jump_stack[m_z + 1].to + m_bookmark.pointer.offset[m_z];
      m_selectedEntry = (m_EditorBaseAddr % 16) / 4 + 11;
      m_addressmod = m_EditorBaseAddr % 4;
      m_searchType = m_bookmark.type;
      if (m_addressmod % 2)
        m_searchType = SEARCH_TYPE_UNSIGNED_8BIT;
      else if (m_addressmod == 2)
        m_searchType = SEARCH_TYPE_UNSIGNED_16BIT;
      else 
        m_searchType = SEARCH_TYPE_UNSIGNED_32BIT;
    }
  }
  else if (kdown & KEY_L && !(kheld & KEY_ZL))
  {
    if (m_z < m_bookmark.pointer.depth)
    {
      m_z++;
      m_EditorBaseAddr = m_jump_stack[m_z].from;
      m_selectedEntry = (m_EditorBaseAddr % 16) / 4 + 11;
      m_searchType = SEARCH_TYPE_UNSIGNED_32BIT;
      m_addressmod = 0;
    }
  }
  else if (kdown & KEY_UP)
  {
    if (m_selectedEntry > 4)
      m_selectedEntry -= 5;
    else
    {
      m_EditorBaseAddr -= 0x10;
    }
  }
  else if (kdown & KEY_DOWN)
  {
    if (m_selectedEntry < 35)
      m_selectedEntry += 5;
    else
    {
      m_EditorBaseAddr += 0x10;
    }
  }
  else if (kdown & KEY_LEFT)
  {
    if (m_addressmod >= dataTypeSizes[m_searchType] && dataTypeSizes[m_searchType] < 4)
      m_addressmod -= dataTypeSizes[m_searchType];
    else
    {
      if (m_selectedEntry % 5 > 1)
      {
        m_selectedEntry--;
        if(dataTypeSizes[m_searchType] < 4)
          m_addressmod = 4 - dataTypeSizes[m_searchType];
      }
    }
  }
  else if (kdown & KEY_RIGHT)
  {
    if (m_addressmod + dataTypeSizes[m_searchType] < 4)
      m_addressmod += dataTypeSizes[m_searchType];
    else
    {
      if (m_selectedEntry % 5 < 4)
      {
        m_selectedEntry++;
        m_addressmod = 0;
      }
    }
  }
  else if (kdown & KEY_PLUS) // Add bookmark
  {
    u64 address = m_EditorBaseAddr - (m_EditorBaseAddr % 16) - 0x20 + (m_selectedEntry - 1 - (m_selectedEntry / 5)) * 4 + m_addressmod;
    bookmark_t bookmark;
    if (address >= m_heapBaseAddr && address < m_heapEnd)
    {
      bookmark.offset = address - m_heapBaseAddr;
      bookmark.heap = true;
    }
    else if (address >= m_mainBaseAddr && address < m_mainend)
    {
      bookmark.offset = address - m_mainBaseAddr;
      bookmark.heap = false;
    }
    bookmark.type = m_searchType;
    Gui::requestKeyboardInput("Enter Label", "Enter Label to add to bookmark .", "", SwkbdType_QWERTY, bookmark.label, 18);
    m_AttributeDumpBookmark->addData((u8 *)&bookmark, sizeof(bookmark_t));
    m_memoryDumpBookmark->addData((u8 *)&address, sizeof(u64));
    if (m_bookmark.pointer.depth > 0)
    {
      u64 address = m_EditorBaseAddr - (m_EditorBaseAddr % 16) - 0x20 + (m_selectedEntry - 1 - (m_selectedEntry / 5)) * 4 + m_addressmod;
      if (m_z == m_bookmark.pointer.depth)
        m_bookmark.pointer.offset[m_z] = address - ((m_bookmark.heap) ? m_heapBaseAddr : m_mainBaseAddr);
      else
        m_bookmark.pointer.offset[m_z] = address - m_jump_stack[m_z + 1].to;
      bookmark.pointer.depth = m_bookmark.pointer.depth - m_z;
      {
        for (int z = m_bookmark.pointer.depth; z >= m_z; z--)
        {
          bookmark.pointer.offset[z-m_z] = m_bookmark.pointer.offset[z];
        }
        // memcpy(&(bookmark.pointer), &(m_bookmark.pointer[m_bookmark.pointer.depth - m_depth_count]), (m_depth_count + 2) * 8);
        m_AttributeDumpBookmark->addData((u8 *)&bookmark, sizeof(bookmark_t));
        m_memoryDumpBookmark->addData((u8 *)&address, sizeof(u64));
      }
    }
    MemoryDump *BMDump = new MemoryDump(EDIZON_DIR "/BMDump.dat", DumpType::HELPER, false);
    BMDump->addData((u8 *)&bookmark.label, 18);
    BMDump->addData((u8 *)&address, sizeof(u64));
    delete BMDump;
    m_AttributeDumpBookmark->flushBuffer();
    m_memoryDumpBookmark->flushBuffer();
    (new Snackbar("Address added to bookmark!"))->show();
    printf("%s\n", "PLUS key pressed1");
  }
  else if (kdown & KEY_ZR && kheld & KEY_ZL) // Page Up
  {
    m_EditorBaseAddr -= 0x80;
  }
  else if (kdown & KEY_ZR) // Page down
  {
    m_EditorBaseAddr += 0x80;
  }
  else if (kdown & KEY_R && kheld & KEY_ZL) // change type
  {
    m_addressmod = 0;
    if (m_searchType < SEARCH_TYPE_FLOAT_64BIT)
    {
      u8 i = static_cast<u8>(m_searchType) + 1;
      m_searchType = static_cast<searchType_t>(i);
    }
  }
  else if (kdown & KEY_L && kheld & KEY_ZL) // Chang type
  {
    m_addressmod = 0;
    if (m_searchType > SEARCH_TYPE_UNSIGNED_8BIT)
    {
      u8 i = static_cast<u8>(m_searchType) - 1;
      m_searchType = static_cast<searchType_t>(i);
    }
  }
  else if (kdown & KEY_L && !(kheld & KEY_ZL))
  {
  }
  else if (kdown & KEY_X) // Hex mode toggle
  {
    if (m_searchValueFormat == FORMAT_DEC)
      m_searchValueFormat = FORMAT_HEX;
    else
      m_searchValueFormat = FORMAT_DEC;
  }
  else if (kdown & KEY_Y && !(kheld & KEY_ZL)) // Goto
  {
    u64 address = m_EditorBaseAddr - (m_EditorBaseAddr % 16) - 0x20 + (m_selectedEntry - 1 - (m_selectedEntry / 5)) * 4 + m_addressmod;
    u64 pointed_address;
    m_debugger->readMemory(&pointed_address, sizeof(u64), address);
    if (m_z == m_bookmark.pointer.depth)
      m_bookmark.pointer.offset[m_z] = address - ((m_bookmark.heap) ? m_heapBaseAddr : m_mainBaseAddr);
    else
      m_bookmark.pointer.offset[m_z] = address - m_jump_stack[m_z + 1].to;
    if ((pointed_address >= m_mainBaseAddr && pointed_address <= m_mainend) || (pointed_address >= m_heapBaseAddr && pointed_address <= m_heapEnd))
    {
      if (m_z == 0)
      {
        if (m_bookmark.pointer.depth < MAX_POINTER_DEPTH) // expand pointer chain
        {
          // if (m_bookmark.pointer.depth == 0)
          //   m_bookmark.heap = true;
          m_bookmark.pointer.depth++;
          for (int z = m_bookmark.pointer.depth; z > 0; z--)
          {
            m_bookmark.pointer.offset[z]=m_bookmark.pointer.offset[z-1];
          }
          m_bookmark.pointer.offset[0] = 0;
          m_z = 1;
          // m_bookmark.pointer.offset[1] = address - m_jump_stack[2].to;
        }
        else
        {
          // m_bookmark.pointer.offset[m_z] = address - m_jump_stack[m_z + 1].to;
        }
      }
      else
      {
        // m_bookmark.pointer.offset[m_z] = address - m_jump_stack[m_z + 1].to;
        // m_jump_stack[m_z].from = address;
        // m_jump_stack[m_z].to = pointed_address;
      }
    }
    else
    { // change target
      if (m_z == 0)
      {
        // m_bookmark.pointer.offset[0] = address - m_jump_stack[1].to;
      }
      else
      {
        // m_bookmark.pointer.offset[m_z] = address - m_jump_stack[m_z + 1].to;
      }
    }

      // if (m_jump_stack_index + 1 < MAX_JUMP_STACK)
      // {
      //   m_jump_stack[m_jump_stack_index] = address;
      //   m_jump_stack_index++; 
      //   m_jump_stack_max = m_jump_stack_index;
      //   m_EditorBaseAddr = pointed_address;
      //   m_selectedEntry = (m_EditorBaseAddr % 16) / 4 + 11;
      //   m_addressmod = m_EditorBaseAddr % 4;
      //   if (m_addressmod % dataTypeSizes[m_searchType] != 0)
      //     m_addressmod = 0;
      // }
      // else
      // {
      //   (new Snackbar("Jump Stack full!"))->show();
      // }

    
    // std::stringstream ss;
    // ss << "0x" << std::uppercase << std::hex << address;
    // char input[16];
    // if (Gui::requestKeyboardInput("Enter Address", "Enter Address to add to bookmark .", ss.str(), SwkbdType_QWERTY, input, 18))
    // {
    //   address = static_cast<u64>(std::stoul(input, nullptr, 16));
    //   bookmark_t bookmark;
    //   bookmark.type = m_searchType;
    //   Gui::requestKeyboardInput("Enter Label", "Enter Label to add to bookmark .", "", SwkbdType_QWERTY, bookmark.label, 18);
    //   m_AttributeDumpBookmark->addData((u8 *)&bookmark, sizeof(bookmark_t));
    //   m_AttributeDumpBookmark->flushBuffer();
    //   (new Snackbar("Address added to bookmark!"))->show();
    //   m_memoryDumpBookmark->addData((u8 *)&address, sizeof(u64));
    //   m_memoryDumpBookmark->flushBuffer();
    // }
  }
  else if (kdown & KEY_Y && (kheld & KEY_ZL))
  {
    u64 address = m_EditorBaseAddr - (m_EditorBaseAddr % 16) - 0x20 + (m_selectedEntry - 1 - (m_selectedEntry / 5)) * 4 + m_addressmod;
    // u64 pointed_address = address;
    std::stringstream ss;
    ss << "0x" << std::uppercase << std::hex << address;
    char input[19];
    if (Gui::requestKeyboardInput("Enter Address", "Enter Address to Jump to .", ss.str(), SwkbdType_QWERTY, input, 18))
    {
      address = static_cast<u64>(std::stoul(input, nullptr, 16)); // this line has problem?
      bookmark_t bookmark;
      bookmark.type = m_searchType;
      if (!Gui::requestKeyboardInput("Enter Label", "Enter Label to add to bookmark .", "", SwkbdType_QWERTY, bookmark.label, 18))
        strcpy(bookmark.label, "jump address");
      m_AttributeDumpBookmark->addData((u8 *)&bookmark, sizeof(bookmark_t));
      m_AttributeDumpBookmark->flushBuffer();
      (new Snackbar("Address added to bookmark!"))->show();
      m_memoryDumpBookmark->addData((u8 *)&address, sizeof(u64));
      m_memoryDumpBookmark->flushBuffer();
      if ((address >= m_mainBaseAddr && address <= m_mainend) || (address >= m_heapBaseAddr && address <= m_heapEnd))
      {
        // printf("valid %lx\n", address);
        // (new Snackbar("Address valid!"))->show();
        // if (m_jump_stack_index < MAX_JUMP_STACK)
        // {
        //   m_jump_stack[m_jump_stack_index] = m_EditorBaseAddr;
        //   m_jump_stack_index++;
        //   m_EditorBaseAddr = address;
        // }
        // else
        // {
        //   (new Snackbar("Jump Stack full!"))->show();
        // }
      }
      else
      {
        // printf("not valid %lx\n", address);
        (new Snackbar("Address not valid!"))->show();
      }
    }
  }
  else if (kdown & KEY_A)
  {
    u64 address = m_EditorBaseAddr - (m_EditorBaseAddr % 16) - 0x20 + (m_selectedEntry - 1 - (m_selectedEntry / 5)) * 4 + m_addressmod;
    char input[19];
    char initialString[21];
    strcpy(initialString, _getAddressDisplayString(address, m_debugger, m_searchType).c_str());
    if (Gui::requestKeyboardInput("Enter value", "Enter a value that should get written at this .", initialString, m_searchValueFormat == FORMAT_DEC ? SwkbdType_NumPad : SwkbdType_QWERTY, input, 18))
    {
      if (m_searchValueFormat == FORMAT_HEX)
      {
        auto value = static_cast<u64>(std::stoul(input, nullptr, 16));
        m_debugger->writeMemory(&value, dataTypeSizes[m_searchType], address);
      }
      else if (m_searchType == SEARCH_TYPE_FLOAT_32BIT)
      {
        auto value = static_cast<float>(std::atof(input));
        m_debugger->writeMemory(&value, sizeof(value), address);
      }
      else if (m_searchType == SEARCH_TYPE_FLOAT_64BIT)
      {
        auto value = std::atof(input);
        m_debugger->writeMemory(&value, sizeof(value), address);
      }
      else if (m_searchType != SEARCH_TYPE_NONE)
      {
        auto value = std::atol(input);
        m_debugger->writeMemory((void *)&value, dataTypeSizes[m_searchType], address);
      }
    }
  }
}

void GuiCheats::drawSearchRAMMenu()
{
  static u32 cursorBlinkCnt = 0;
  u32 strWidth = 0;
  std::stringstream ss;

  if ((m_searchMenuLocation == SEARCH_NONE) || (m_searchMenuLocation == SEARCH_POINTER) || (m_searchMenuLocation == SEARCH_editRAM)
   || (m_searchMenuLocation == SEARCH_editRAM2) || (m_searchMenuLocation == SEARCH_editExtraSearchValues))
    return;

  Gui::drawRectangled(0, 0, Gui::g_framebuffer_width, Gui::g_framebuffer_height, Gui::makeColor(0x00, 0x00, 0x00, 0xA0));

  Gui::drawRectangle(50, 50, Gui::g_framebuffer_width - 100, Gui::g_framebuffer_height - 100, currTheme.backgroundColor);
  Gui::drawRectangle(100, 135, Gui::g_framebuffer_width - 200, 1, currTheme.textColor);
  {
      static const char *const regionNames[] = {"HEAP", "MAIN", "HEAP + MAIN", "RAM", "  "};
      static const char *const modeNames[] = {"==", "!=", ">", "StateB", "<", "StateA", "A..B", "SAME", "DIFF", "+ +", "- -", "PTR", "  "};
      ss.str("");
      if (m_memoryDump1 != nullptr)
        ss << "\uE132   Search Bookmark";
      else
        ss << "\uE132   Search Memory";
      ss << "   [ " << dataTypes[m_searchType] << " ]";
      ss << "   [ " << modeNames[m_searchMode] << " ]";
      ss << "   [ " << regionNames[m_searchRegion] << " ]";
      if (m_use_range)
      ss << "   [ using range ]";
  }
  Gui::drawText(font24, 120, 70, currTheme.textColor, ss.str().c_str());
  ss.str("");

  Gui::drawTextAligned(font20, 100, 160, currTheme.textColor, "\uE149 \uE0A4", ALIGNED_LEFT);
  Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 100, 160, currTheme.textColor, "\uE0A5 \uE14A", ALIGNED_RIGHT);

  Gui::drawTextAligned(font20, 260, 160, m_searchMenuLocation == SEARCH_TYPE ? currTheme.selectedColor : currTheme.textColor, "TYPE", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, 510, 160, m_searchMenuLocation == SEARCH_MODE ? currTheme.selectedColor : currTheme.textColor, "MODE", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, 760, 160, m_searchMenuLocation == SEARCH_REGION ? currTheme.selectedColor : currTheme.textColor, "REGION", ALIGNED_CENTER);
  Gui::drawTextAligned(font20, 1010, 160, m_searchMenuLocation == SEARCH_VALUE ? currTheme.selectedColor : currTheme.textColor, "VALUE", ALIGNED_CENTER);

  static const char *const typeNames[] = {"u8", "s8", "u16", "s16", "u32", "s32", "u64", "s64", "flt", "dbl", "void*"};
  static const char *const modeNames[] = {"==", "!=", ">", "StateB", "<", "StateA", "A..B", "SAME", "DIFF", "+ +", "- -", "PTR"};
  static const char *const modeNames1[] = {"==", "!=", ">", "StateA", "<", "", "A..B", "", "Unknown", "? +", "? -", "PTR"};
  static const char *const regionNames[] = {"HEAP", "MAIN", "HEAP + MAIN", "RAM"};

  switch (m_searchMenuLocation) // search menu
  {
  case SEARCH_TYPE:
    for (u8 i = 0; i < 11; i++)
    {
      if (m_selectedEntry == i)
        Gui::drawRectangled(356 + (i / 2) * 100, 220 + (i % 2) * 100, 90, 90, m_searchType == static_cast<searchType_t>(i) ? currTheme.selectedColor : currTheme.highlightColor);

      Gui::drawRectangled(361 + (i / 2) * 100, 225 + (i % 2) * 100, 80, 80, currTheme.separatorColor);
      Gui::drawTextAligned(font20, 400 + (i / 2) * 100, 250 + (i % 2) * 100, currTheme.textColor, typeNames[i], ALIGNED_CENTER);
    }

    Gui::drawTextAligned(font14, Gui::g_framebuffer_width / 2, 500, currTheme.textColor, "Set the data type of the value you’re searching here. The prefix [u] means unsigned (positive integers), [s] means \n"
                                                                                         "signed (positive and negative integers), [flt] is for floating point numbers (rational numbers), [dbl] is for double (bigger \n"
                                                                                         "rational numbers) and [void*] stands for pointer (link to another memory ) which is useful for creating cheats. The \n"
                                                                                         "number that follows is the number of bits used in memory which determines the maximum value. Choose the data type that \n"
                                                                                         "best fits for the type of data you’re looking for.",
                         ALIGNED_CENTER);

    Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 100, Gui::g_framebuffer_height - 100, currTheme.textColor, "\uE0E1 Back     \uE0E0 OK", ALIGNED_RIGHT);

    break;
  case SEARCH_MODE:
    for (u8 i = 0; i < 12; i++)
    {
      if (m_selectedEntry == i)
        Gui::drawRectangled(356 + (i / 2) * 100, 220 + (i % 2) * 100, 90, 90, m_searchMode == static_cast<searchMode_t>(i) ? currTheme.selectedColor : currTheme.highlightColor);

      Gui::drawRectangled(361 + (i / 2) * 100, 225 + (i % 2) * 100, 80, 80, currTheme.separatorColor);
      Gui::drawTextAligned(font20, 400 + (i / 2) * 100, 250 + (i % 2) * 100, currTheme.textColor, (m_memoryDump->size() == 0) ? modeNames1[i] : modeNames[i], ALIGNED_CENTER);
    }

    if (m_memoryDump->size() == 0)
      Gui::drawTextAligned(font14, Gui::g_framebuffer_width / 2, 500, currTheme.textColor, "Set the mode you want to use for finding values. With these modes EdiZon will search for known values that are equal to [==], \n"
                                                                                           "not equal to [!=], greater than [>] or less than [<] that you input. \n"
                                                                                           "For unknown value [A : B] allows you to set a (min : max) range of values, Choose [Unknown] if to search find values that \n"
                                                                                           "will be different, [? +] and [? -] for value that will increased or decreased \n"
                                                                                           "[StateA] let you capture state A to be compared with state B in next search. ",
                           ALIGNED_CENTER);
    else
      Gui::drawTextAligned(font14, Gui::g_framebuffer_width / 2, 500, currTheme.textColor, "Set the mode you want to use for finding values. With these modes EdiZon will search for known values that are equal to [==], \n"
                                                                                           "not equal to [!=], greater than [>] or less than [<] that you input. \n"
                                                                                           "For unknown value [A : B] allows you to set a (min : max) range of values, SAME and DIFF search allows you to find values that \n"
                                                                                           "stayed the same or changed since the previous search, [+ +] and [- -] checks for values that increased or decreased since the \n"
                                                                                           "previous search. [StateA] and [StateB] let you search for value that are mutually exclusive between two states. ",
                           ALIGNED_CENTER);

    Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 100, Gui::g_framebuffer_height - 100, currTheme.textColor, "\uE0E1 Back     \uE0E0 OK", ALIGNED_RIGHT);
    break;
  case SEARCH_REGION:
    for (u8 i = 0; i < 4; i++)
    {
      if (m_selectedEntry == i)
        Gui::drawRectangled((Gui::g_framebuffer_width / 2) - 155, 215 + i * 70, 310, 70, m_searchRegion == static_cast<searchRegion_t>(i) ? currTheme.selectedColor : currTheme.highlightColor);

      Gui::drawRectangled((Gui::g_framebuffer_width / 2) - 150, 220 + i * 70, 300, 60, currTheme.separatorColor);
      Gui::drawTextAligned(font20, (Gui::g_framebuffer_width / 2), 235 + i * 70, currTheme.textColor, regionNames[i], ALIGNED_CENTER);
    }

    Gui::drawTextAligned(font14, Gui::g_framebuffer_width / 2, 500, currTheme.textColor, "Set the memory region you want to search in. HEAP contains dynamically allocated values and will be where the majority of \n"
                                                                                         "values worth changing will be found. MAIN contains global variables and instructions for game operation. You may find some \n"
                                                                                         "values here but it’s mainly for finding pointers to HEAP values or changing game code. RAM will search the entirety of the Games \n"
                                                                                         "used memory including memory shared memory and resources. Should only be used as a final resort as this will be extremely slow. \n",
                         ALIGNED_CENTER);

    Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 100, Gui::g_framebuffer_height - 100, currTheme.textColor, "\uE0E1 Back     \uE0E0 OK", ALIGNED_RIGHT);
    break;
  case SEARCH_VALUE:
    if (m_memoryDump1 != nullptr)
      Gui::drawTextAligned(font14, Gui::g_framebuffer_width / 2, 500, currTheme.textColor, "Bookmark search let you narrow down your bookmark list by searching. Only search of type == or A..B supported.", ALIGNED_CENTER);
    else
      Gui::drawTextAligned(font14, Gui::g_framebuffer_width / 2, 500, currTheme.textColor, "Set the value you want to search for. The value(s) you enter here will depend on what options you've chosen in the \n"
                                                                                           "first three sections. Either it's the exact integer you want to search for, a floating point number or even two values that \n"
                                                                                           "will be used as range. Use quick set keys to change the search mode \uE0AD SAME \uE0AC DIFF \uE0AB ++ \uE0AE -- \uE0B3 A..B \uE0B4 ==/!=\n"
                                                                                           "If you search type is floating point \uE0A5 negate the number. \uE0C4 cycle float type \uE0C5 presets \uE0A3 cycle integer type",
                           ALIGNED_CENTER);

    //Gui::drawRectangle(300, 250, Gui::g_framebuffer_width - 600, 80, currTheme.separatorColor);
    if (m_searchMode == SEARCH_MODE_SAME || m_searchMode == SEARCH_MODE_DIFF || m_searchMode == SEARCH_MODE_INC || m_searchMode == SEARCH_MODE_DEC || m_searchMode == SEARCH_MODE_DIFFA || m_searchMode == SEARCH_MODE_SAMEA) {m_selectedEntry = 1;};
    if (!(m_searchMode == SEARCH_MODE_SAME || m_searchMode == SEARCH_MODE_DIFF || m_searchMode == SEARCH_MODE_INC || m_searchMode == SEARCH_MODE_DEC || m_searchMode == SEARCH_MODE_DIFFA || m_searchMode == SEARCH_MODE_SAMEA) || m_use_range)
    {
      Gui::drawRectangle(300, 327, Gui::g_framebuffer_width - 600, 3, currTheme.textColor);
      if (m_searchValueFormat == FORMAT_DEC)
        ss << _getValueDisplayString(m_searchValue[0], m_searchType);
      else if (m_searchValueFormat == FORMAT_HEX)
      {
        switch (dataTypeSizes[m_searchType])
        {
        case 1:
          ss << "0x" << std::uppercase << std::hex << (m_searchValue[0]._u16 & 0x00FF);
          break;
        case 2:
          ss << "0x" << std::uppercase << std::hex << m_searchValue[0]._u16;
          break;
        default:
        case 4:
          ss << "0x" << std::uppercase << std::hex << m_searchValue[0]._u32;
          break;
        case 8:
          ss << "0x" << std::uppercase << std::hex << m_searchValue[0]._u64;
          break;
        }
      }

      Gui::getTextDimensions(font20, ss.str().c_str(), &strWidth, nullptr);
      Gui::drawTextAligned(font20, 310, 285, currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);

      //	Start Mod for search Range display
      if (cursorBlinkCnt++ % 20 > 10 && m_selectedEntry == 0 && (m_searchValueIndex == 0))
        Gui::drawRectangled(312 + strWidth, 285, 3, 35, currTheme.highlightColor);

      if (m_searchMode == SEARCH_MODE_RANGE || (m_use_range && (m_searchMode == SEARCH_MODE_SAME || m_searchMode == SEARCH_MODE_DIFF || m_searchMode == SEARCH_MODE_INC || m_searchMode == SEARCH_MODE_DEC || m_searchMode == SEARCH_MODE_DIFFA || m_searchMode == SEARCH_MODE_SAMEA)) )
      {
        ss.str("");
        if (m_searchValueFormat == FORMAT_DEC)
          ss << _getValueDisplayString(m_searchValue[1], m_searchType);
        else if (m_searchValueFormat == FORMAT_HEX)
          ss << "0x" << std::uppercase << std::hex << m_searchValue[1]._u64;
        Gui::getTextDimensions(font20, ss.str().c_str(), &strWidth, nullptr);
        Gui::drawTextAligned(font20, 650, 285, currTheme.textColor, ss.str().c_str(), ALIGNED_LEFT);
      }

      if (cursorBlinkCnt++ % 20 > 10 && m_selectedEntry == 0 && (m_searchValueIndex == 1))
        Gui::drawRectangled(652 + strWidth, 285, 3, 35, currTheme.highlightColor);
      //	End Mod

      if (m_searchValueFormat == FORMAT_DEC)
        Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 100, Gui::g_framebuffer_height - 100, currTheme.textColor, "\uE0EA \uE0EF \uE0F0 Quick Set Search Mode    \uE0E6 -   \uE0E7 +   \uE0E2 Hexadecimal view  \uE0E1 Back  \uE0E0 OK", ALIGNED_RIGHT);
      else if (m_searchValueFormat == FORMAT_HEX)
        Gui::drawTextAligned(font20, Gui::g_framebuffer_width - 100, Gui::g_framebuffer_height - 100, currTheme.textColor, "\uE0AD SAME \uE0AC DIFF \uE0AB ++ \uE0AE -- \uE0EF A..B \uE0F0 == \uE0E6 - \uE0E7 + \uE0E2 Decimal view     \uE0E1 Back     \uE0E0 OK", ALIGNED_RIGHT);
    };
    if (m_selectedEntry == 1)
      Gui::drawRectangled(Gui::g_framebuffer_width / 2 - 155, 345, 310, 90, currTheme.highlightColor);

    if (m_searchType != SEARCH_TYPE_NONE && m_searchMode != SEARCH_MODE_NONE && m_searchRegion != SEARCH_REGION_NONE)
    {
      Gui::drawRectangled(Gui::g_framebuffer_width / 2 - 150, 350, 300, 80, currTheme.selectedColor);
      Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, 375, currTheme.backgroundColor, "Search Now!", ALIGNED_CENTER);
    }
    else
    {
      Gui::drawRectangled(Gui::g_framebuffer_width / 2 - 150, 350, 300, 80, currTheme.selectedButtonColor);
      Gui::drawTextAligned(font20, Gui::g_framebuffer_width / 2, 375, currTheme.separatorColor, "Search Now!", ALIGNED_CENTER);
    }

    {
      std::string s = m_edizon_dir + "/datadump2.datB";
      if (access(s.c_str(), F_OK) == 0)
      {
        ss.str("");
        ss << "\uE0AF Store ";
        ss << m_store_extension;
        Gui::drawTextAligned(font20, Gui::g_framebuffer_width * 4 / 5, 375, currTheme.textColor, ss.str().c_str(), ALIGNED_CENTER);
      }
    };
    break;
  case SEARCH_NONE:
  case SEARCH_editRAM:
  case SEARCH_editRAM2:
  case SEARCH_POINTER:
  case SEARCH_editExtraSearchValues:
  case SEARCH_pickjump:
    break;
  }
}
void GuiCheats::easymode_input(u32 kdown, u32 kheld)
{
  if (kdown & KEY_B)
  {
    Gui::g_requestExit = true;
  }
  // else if (kdown & KEY_L)
  // {
  //   Gui::g_nextGui = GUI_MEMORY_EDITOR;
  // }
  else if (kdown & KEY_MINUS)
  {
    Gui::g_nextGui = GUI_FIRST_RUN;
  }
  else if (kdown & KEY_UP)
  {
    if (m_selectedEntry > 0)
      m_selectedEntry--;
    if (m_selectedEntry + 1 == cheatListOffset && cheatListOffset > 0)
      cheatListOffset-=8;
  }
  else if (kdown & KEY_DOWN) //
  {
    if (m_selectedEntry < (m_cheatCnt - 1))
      m_selectedEntry++;
    if (m_selectedEntry == (cheatListOffset + 8) && cheatListOffset < (m_cheatCnt - 8))
      cheatListOffset+=8;
  }
  else if (kdown & KEY_ZR)
  {
    cheatListOffset += 8;
    m_selectedEntry += 8;
    if (cheatListOffset >= m_cheatCnt)
    {
      cheatListOffset -= 8;
      m_selectedEntry -= 8;
    }
    if (m_selectedEntry + 1 > m_cheatCnt)
      m_selectedEntry = m_cheatCnt - 1;
  }
  else if (kdown & KEY_ZL)
  {
    if (cheatListOffset >= 8)
    {
      cheatListOffset -= 8;
      m_selectedEntry -= 8;
    }
    else
    {
      cheatListOffset = 0;
      m_selectedEntry = 0;
    }
    
  }
  else if (kdown & KEY_A)
  {
    if (m_cheatCnt == 0)
      return;
    // count total opcode
    u32 opcodecount = m_cheats[m_selectedEntry].definition.num_opcodes;
    for (u32 i = 0; i < m_cheatCnt; i++)
    {
      if (m_cheats[i].enabled)
        opcodecount += m_cheats[i].definition.num_opcodes;
    }
    if (opcodecount > 0x400)
    {
      (new Snackbar("Total opcode count would exceed 1024!"))->show();
      return;
    }
    dmntchtToggleCheat(m_cheats[m_selectedEntry].cheat_id);
    u64 cheatCnt = 0;
    dmntchtGetCheatCount(&cheatCnt);
    if (cheatCnt > 0)
    {
      delete[] m_cheats;
      m_cheats = new DmntCheatEntry[cheatCnt];
      dmntchtGetCheats(m_cheats, cheatCnt, 0, &m_cheatCnt);
    }
  }
}
void GuiCheats::pickjump_input(u32 kdown, u32 kheld)
{
  if (kdown & KEY_Y)
  {
    m_searchMenuLocation = SEARCH_editRAM2;
    u64 m_pick = m_selectedJumpSource + m_fromto32_offset;
      // Need to get main
    // u64 address = (m_EditorBaseAddr - (m_EditorBaseAddr % 16) - 0x20 + (m_selectedEntry - 1 - (m_selectedEntry / 5)) * 4 + m_addressmod);
    if (m_z == 0)
      m_bookmark.pointer.offset[m_z] =  (m_EditorBaseAddr - (m_EditorBaseAddr % 16) - 0x20 + (m_selectedEntry - 1 - (m_selectedEntry / 5)) * 4 + m_addressmod) - (m_fromto32[m_pick].to + m_heapBaseAddr) ;
    else
    {
      m_bookmark.pointer.offset[m_z] = m_jump_stack[m_z].from - (m_fromto32[m_pick].to + m_heapBaseAddr);
      // printf("m_jump_stack[m_z].from %lx - (m_fromto32[m_pick].to + m_heapBaseAddr) %lx \n",m_jump_stack[m_z].from, (m_fromto32[m_pick].to + m_heapBaseAddr));
    }
    m_z++;
    if (m_fromto32[m_pick].from == 0)
    {
      m_jump_stack[m_z].from = m_selectedJumpSource_offset + m_mainBaseAddr; //get_main_offset32(m_fromto32[m_pick].to)
    }
    else
      m_jump_stack[m_z].from = m_fromto32[m_pick].from + m_heapBaseAddr;
    m_jump_stack[m_z].to = m_fromto32[m_pick].to + m_heapBaseAddr;
    m_EditorBaseAddr = m_jump_stack[m_z].from;
    m_selectedEntry = (m_EditorBaseAddr % 16) / 4 + 11;
    m_searchType = SEARCH_TYPE_UNSIGNED_32BIT;
    m_addressmod = 0;
    m_bookmark.pointer.depth = m_z;
    m_bookmark.pointer.offset[m_z] = m_jump_stack[m_z].from - ((m_bookmark.heap) ? m_heapBaseAddr : m_mainBaseAddr);
    //   place head here;
  }
  else if (kdown & KEY_UP)
  {
    if (m_selectedJumpSource > 0)
      m_selectedJumpSource--;
    // if (m_selectedJumpSource + 1 == m_fromto32_offset && m_fromto32_offset > 0)
    //   m_fromto32_offset-=15;
  }
  else if (kdown & KEY_DOWN) //
  {
    if ((m_selectedJumpSource < 14) && (m_selectedJumpSource + m_fromto32_offset + 1 < m_fromto32_size))
      m_selectedJumpSource++;
    // if (m_selectedJumpSource == (m_fromto32_offset + 15) && m_fromto32_offset < (m_fromto32_size - 15))
    //   m_fromto32_offset+=15;
  }
  else if (kdown & KEY_R)
  {
    if ( m_fromto32_offset  + 15 < m_fromto32_size )
      m_fromto32_offset += 15;
    if (m_selectedJumpSource + m_fromto32_offset >= m_fromto32_size)
      m_selectedJumpSource = m_fromto32_size - m_fromto32_offset -1;
  }
  else if (kdown & KEY_L)
  {
    if (m_fromto32_offset >= 15)
      m_fromto32_offset -= 15;
  }
  else if (kdown & KEY_X && (kheld & KEY_ZL))  
  {
    m_redo_prep_pointersearch = true;
    u64 address = m_EditorBaseAddr - (m_EditorBaseAddr % 16) - 0x20 + (m_selectedEntry - 1 - (m_selectedEntry / 5)) * 4 + m_addressmod;
    GuiCheats::prep_pointersearch(m_debugger, m_memoryInfo);
    GuiCheats::prep_backjump_stack(address);
  }
  else if (kdown & KEY_B)
  {
    m_searchMenuLocation = SEARCH_editRAM2;
  }
}
// WIP ***************

void GuiCheats::onInput(u32 kdown)
{
  u32 kheld = hidKeysHeld(CONTROLLER_PLAYER_1) | hidKeysHeld(CONTROLLER_HANDHELD);
  if (m_searchMenuLocation == SEARCH_editRAM2)
  {
    editor_input(kdown, kheld);
    return;
  };
  if (m_searchMenuLocation == SEARCH_pickjump)
  {
    pickjump_input(kdown, kheld);
    return;
  };
  if (m_searchMenuLocation == SEARCH_editExtraSearchValues)
  {
    EditExtraSearchValues_input(kdown, kheld);
    return;
  };
  if (Config::getConfig()->easymode)
  {
    easymode_input(kdown, kheld);
    return;
  }
  if (m_editCheat)
  {
    // printf("kdown = %x, kheld = %x\n", kdown, kheld);
    u32 keycode = 0x80000000 | kdown;
    if (buttonStr(keycode) != "")
    {
      // edit cheat
      if ((m_cheats[m_selectedEntry].definition.opcodes[0] & 0xF0000000) == 0x80000000)
      {
        m_cheats[m_selectedEntry].definition.opcodes[0] = keycode;
      }
      else
      {
        if (m_cheats[m_selectedEntry].definition.num_opcodes < 0x100 + 2)
        {
          m_cheats[m_selectedEntry].definition.opcodes[m_cheats[m_selectedEntry].definition.num_opcodes + 1] = 0x20000000;

          for (u32 i = m_cheats[m_selectedEntry].definition.num_opcodes; i > 0; i--)
          {
            m_cheats[m_selectedEntry].definition.opcodes[i] = m_cheats[m_selectedEntry].definition.opcodes[i - 1];
          }
          m_cheats[m_selectedEntry].definition.num_opcodes += 2;
          m_cheats[m_selectedEntry].definition.opcodes[0] = keycode;
        }
      }
      // insert cheat
      for (u32 i = m_selectedEntry; i < m_cheatCnt; i++)
      {
        dmntchtRemoveCheat(m_cheats[i].cheat_id);
      }
      for (u32 i = m_selectedEntry; i < m_cheatCnt; i++)
      {
        u32 outid;
        dmntchtAddCheat(&(m_cheats[i].definition), m_cheats[i].enabled, &outid);
      }
    };
    m_editCheat = false;
    return;
  }
  if (kdown & KEY_B)
  {
    u8 saveE = m_selectedEntry;
    m_selectedEntry = 0;

    if (m_searchMenuLocation == SEARCH_NONE)
    {
      // Gui::g_nextGui = GUI_MAIN;
      PSsaveSTATE();
      if (kheld & KEY_R)
      {
        Config::getConfig()->option_once = true;
        Config::writeConfig();
      }
      if (kheld & KEY_ZL)
      {
        unfreeze();
        if (!m_debugger -> m_dmnt)
        {
          m_debugger->detatch();
          dmntchtForceOpenCheatProcess();
          printf("force open called\n");
        }
        else
        {
          dmntchtForceCloseCheatProcess();
          m_debugger->m_dmnt = false;
        }
        printf("dmnt toggled \n");
        if (autoexitcheck()) Gui::g_requestExit = true;
        return;
      };
      if (m_memoryDump1 != nullptr)
        m_memoryDump1->setSearchParams(m_searchType, m_searchMode, m_searchRegion, m_searchValue[0], m_searchValue[1], m_use_range);
      else
        m_memoryDump->setSearchParams(m_searchType, m_searchMode, m_searchRegion, m_searchValue[0], m_searchValue[1], m_use_range);
      Gui::g_requestExit = true;
      return;
    }
    else if ((m_searchMenuLocation == SEARCH_POINTER) || (m_showpointermenu))
    {
      m_searchMenuLocation = SEARCH_NONE;
      m_abort = true;
      m_showpointermenu = false;
      printf("abort pressed .. \n");
    }
    else if (m_searchMenuLocation == SEARCH_editRAM)
    {
      m_selectedEntry = m_selectedEntrySave;
      m_searchMenuLocation = SEARCH_NONE;
    }
    else if (m_searchMenuLocation == SEARCH_TYPE)
    {
      if (m_searchType != SEARCH_TYPE_NONE && m_memoryDump->size() == 0)
      {
        m_searchType = SEARCH_TYPE_NONE;
        m_selectedEntry = saveE;
      }
      else
        m_searchMenuLocation = SEARCH_NONE;
    }
    else if (m_searchMenuLocation == SEARCH_MODE)
    {
      if (m_searchMode != SEARCH_MODE_NONE)
      {
        m_searchMode = SEARCH_MODE_NONE;
        m_selectedEntry = saveE;
      }
      else
        m_searchMenuLocation = SEARCH_NONE;
    }
    else if (m_searchMenuLocation == SEARCH_REGION)
    {
      if (m_searchRegion != SEARCH_REGION_NONE && m_memoryDump->size() == 0)
        m_searchRegion = SEARCH_REGION_NONE;
      else
        m_searchMenuLocation = SEARCH_NONE;
    }
    else if (m_searchMenuLocation == SEARCH_VALUE)
      m_searchMenuLocation = SEARCH_NONE;
  }

  if (m_debugger->getRunningApplicationPID() == 0)
    return;
  // BM2
  if (m_searchMenuLocation == SEARCH_POINTER)
  {
    if (kdown & KEY_Y)
    {
      printf("starting PC dump\n");
      m_searchType = SEARCH_TYPE_UNSIGNED_64BIT;
      m_searchRegion = SEARCH_REGION_HEAP_AND_MAIN;
      Gui::beginDraw();
      Gui::drawRectangle(70, 420, 1150, 65, currTheme.backgroundColor);
      Gui::drawTextAligned(font20, 70, 420, currTheme.textColor, "Making Dump for pointersearcher SE", ALIGNED_LEFT);
      Gui::endDraw();
      GuiCheats::searchMemoryAddressesPrimary2(m_debugger, m_searchValue[0], m_searchValue[1], m_searchType, m_searchMode, m_searchRegion, &m_memoryDump, m_memoryInfo);
      (new Snackbar("Dump for pointersearcher SE completed"))->show();
      // PCdump();
    }

    if ((kdown & KEY_PLUS) && !(kheld & KEY_ZL))
    {
      m_abort = false;
      // (new Snackbar("Starting pointer search"))->show();
      // m_searchMenuLocation = SEARCH_NONE;
      printf("starting pointer search from plus %lx \n", m_EditorBaseAddr);
      // m_AttributeDumpBookmark->getData((m_selectedEntry + m_addresslist_offset) * sizeof(bookmark_t), &m_bookmark, sizeof(bookmark_t));
      m_abort = false;
      m_Time1 = time(NULL);
      if (m_pointersearch_canresume)
        resumepointersearch2();
      else
        startpointersearch2(m_EditorBaseAddr);
      char st[100];
      snprintf(st, 100, "Done pointer search found %ld pointer in %ld seconds", m_pointer_found, time(NULL) - m_Time1);
      printf("done pointer search \n");
      printf("Time taken =%ld\n", time(NULL) - m_Time1);
      (new Snackbar(st))->show();
    }
    if ((kdown & KEY_PLUS) && (kheld & KEY_ZL))
    {
      m_pointersearch_canresume = false;
      delete m_PointerSearch;
      printf("set resume to false\n");
    }

    if (kdown & KEY_UP)
    {
      if (m_selectedEntry > 0)
        m_selectedEntry--;
    }
    if (kdown & KEY_DOWN)
    {
      if (m_selectedEntry < 5)
        m_selectedEntry++;
    }
    if (kdown & KEY_R)
    {
      if (m_selectedEntry == 0 && m_max_depth < MAX_POINTER_DEPTH)
      {
        m_max_depth++;
      }
      else if (m_selectedEntry == 1 && m_max_range < MAX_POINTER_RANGE)
      {
        m_max_range += 0x100;
      }
      else if (m_selectedEntry == 2 && m_max_source < MAX_NUM_SOURCE_POINTER)
      {
        m_max_source += 10;
      }
      else if (m_selectedEntry == 4)
      {
        m_forwarddump = !m_forwarddump;
      }
      else if (m_selectedEntry == 5 && m_numoffset < MAX_NUM_POINTER_OFFSET)
      {
        m_numoffset++;
      };
    }
    if (kdown & KEY_L)
    {
      if (m_selectedEntry == 0 && m_max_depth > 2)
      {
        m_max_depth--;
      }
      else if (m_selectedEntry == 1 && m_max_range > 0x100)
      {
        m_max_range -= 0x100;
      }
      else if (m_selectedEntry == 2 && m_max_source > 10)
      {
        m_max_source -= 10;
      }
      else if (m_selectedEntry == 5 && m_numoffset > 1)
      {
        m_numoffset--;
      };
    }
  }

  if (m_searchMenuLocation == SEARCH_NONE)
  {
    if (kdown & KEY_UP)
    {
      if (m_selectedEntry > 0)
        m_selectedEntry--;

      if (m_menuLocation == CHEATS)
        if (m_selectedEntry+1 == cheatListOffset && cheatListOffset > 0)
          cheatListOffset-=8;
    }

    if (kdown & KEY_DOWN) //
    {
      if (m_menuLocation == CANDIDATES)
      {
        if (m_selectedEntry < 7 && m_selectedEntry + m_addresslist_offset < ((m_memoryDump->size() / sizeof(u64)) - 1))
          m_selectedEntry++;
      }
      else
      {
        if (m_selectedEntry < (m_cheatCnt - 1))
          m_selectedEntry++;

        if (m_selectedEntry == (cheatListOffset + 8) && cheatListOffset < (m_cheatCnt - 8))
          cheatListOffset+=8;
      }
    }
    // start mod
    if ((kdown & KEY_RSTICK) && m_menuLocation == CHEATS && !(kheld & KEY_ZL))
    {
      m_editCheat = true;
      while ((hidKeysHeld(CONTROLLER_PLAYER_1) | hidKeysHeld(CONTROLLER_HANDHELD)) != 0)
      {
        hidScanInput();
      }
    }
    if ((kdown & KEY_RSTICK) && m_menuLocation == CHEATS && (kheld & KEY_ZL))
    { // remove condition key
      if ((m_cheats[m_selectedEntry].definition.opcodes[0] & 0xF0000000) == 0x80000000 && (m_cheats[m_selectedEntry].definition.opcodes[m_cheats[m_selectedEntry].definition.num_opcodes - 1] & 0xF0000000) == 0x20000000)
      {
        for (u32 i = 0; i < m_cheats[m_selectedEntry].definition.num_opcodes - 1; i++)
        {
          m_cheats[m_selectedEntry].definition.opcodes[i] = m_cheats[m_selectedEntry].definition.opcodes[i + 1];
        }
        m_cheats[m_selectedEntry].definition.num_opcodes -= 2;
      }
      // insert cheat
      for (u32 i = m_selectedEntry; i < m_cheatCnt; i++)
      {
        dmntchtRemoveCheat(m_cheats[i].cheat_id);
      }
      for (u32 i = m_selectedEntry; i < m_cheatCnt; i++)
      {
        u32 outid;
        dmntchtAddCheat(&(m_cheats[i].definition), m_cheats[i].enabled, &outid);
      }
    }
    if ((kdown & KEY_LSTICK) && m_menuLocation == CHEATS && !(kheld & KEY_ZL))
    {
      // Edit cheats
      // WIP
      // if (m_cheats[m_selectedEntry].definition.opcodes[0])

      for (u64 i = 0; i < m_cheatCnt; i++)
        if (m_cheatDelete[i])
        {
          m_cheatDelete[i] = false;
          dmntchtRemoveCheat(m_cheats[i].cheat_id);
        }
      reloadcheats();

      // m_menuLocation = CANDIDATES;
      // m_searchType = SEARCH_TYPE_UNSIGNED_64BIT;
      // m_searchMode = SEARCH_MODE_POINTER;
      // m_searchRegion = SEARCH_REGION_HEAP_AND_MAIN;
      // m_searchMenuLocation = SEARCH_VALUE;
      // m_selectedEntry = 1;
      // m_searchValue[0]._u64 = 0x1000000000;
      // m_searchValue[1]._u64 = 0x8000000000;
    }
    if ((kdown & KEY_X) && (kheld & KEY_ZL))
    {
      m_menuLocation = CANDIDATES;
      m_searchType = SEARCH_TYPE_UNSIGNED_64BIT;
      m_searchMode = SEARCH_MODE_POINTER;
      m_searchRegion = SEARCH_REGION_HEAP_AND_MAIN;
      m_searchMenuLocation = SEARCH_VALUE;
      m_selectedEntry = 1;
      m_searchValue[0]._u64 = 0x1000000000;
      m_searchValue[1]._u64 = 0x8000000000;
      Config::getConfig()->exclude_ptr_candidates = false;
    }
    if ((kdown & KEY_LSTICK) && m_menuLocation == CHEATS && (kheld & KEY_ZL))
    {
      dumpcodetofile();
      (new Snackbar("Writing change to file"))->show();
    }

    if ((kdown & KEY_PLUS) && m_menuLocation == CHEATS && (m_cheatCnt > 0) && (m_memoryDump1 != nullptr) && !(kheld & KEY_ZL))
    {
      // printf("start adding cheat to bookmark\n");
      // m_cheatCnt
      DmntCheatDefinition cheat = m_cheats[m_selectedEntry].definition;
      //bookmark_t bookmark;
      memcpy(&bookmark.label, &cheat.readable_name, sizeof(bookmark.label));
      bookmark.pointer.depth = 0;
      bookmark.deleted = false;
      bool success = false;
      u64 offset[MAX_POINTER_DEPTH + 1] = {0};
      u64 depth = 0;
      u64 address;
      bool no7 = true;

      for (u8 i = 0; i < cheat.num_opcodes; i++)
      {
        u8 opcode = (cheat.opcodes[i] >> 28) & 0xF;
        u8 Register = (cheat.opcodes[i] >> 16) & 0xF;
        u8 FSA = (cheat.opcodes[i] >> 12) & 0xF;
        u8 T = (cheat.opcodes[i] >> 24) & 0xF;
        u8 M = (cheat.opcodes[i] >> 20) & 0xF;

        printf("code %x opcode %d register %d FSA %d %x \n", cheat.opcodes[i], opcode, Register, FSA, cheat.opcodes[i + 1]);

        if (depth > MAX_POINTER_DEPTH)
        {
          (new Snackbar("this code is bigger than space catered on the bookmark !!"))->show();
          printf("!!!!!!!!!!!!!!!!!!!!!!!this code is bigger than space catered on the bookmark !! \n");
          break;
        }

        if (opcode == 0)
        { //static case
          i++;
          bookmark.offset = cheat.opcodes[i];
          switch (T)
          {
          case 1:
            bookmark.type = SEARCH_TYPE_UNSIGNED_8BIT;
            i++;
            break;
          case 2:
            bookmark.type = SEARCH_TYPE_UNSIGNED_16BIT;
            i++;
            break;
          case 4:
            bookmark.type = SEARCH_TYPE_UNSIGNED_32BIT;
            i++;
            break;
          case 8:
            bookmark.type = SEARCH_TYPE_UNSIGNED_64BIT;
            i += 2;
            break;
          default:
            printf("cheat code processing error, wrong width value\n");
            bookmark.type = SEARCH_TYPE_UNSIGNED_32BIT;
            i++;
            break;
          };
          if (M == 1)
          {
            bookmark.heap = true;
            address = m_heapBaseAddr + bookmark.offset;
          }
          else
          {
            bookmark.heap = false;
            address = m_mainBaseAddr + bookmark.offset;
          }

          m_memoryDumpBookmark->addData((u8 *)&address, sizeof(u64));
          m_AttributeDumpBookmark->addData((u8 *)&bookmark, sizeof(bookmark_t));
          break;
        }
        if (depth == 0)
        {
          if (opcode == 5 && FSA == 0)
          {
            i++;
            if (M == 0)
              offset[depth] = cheat.opcodes[i];
            else
              offset[depth] = m_heapBaseAddr - m_mainBaseAddr + cheat.opcodes[i];
            depth++;
          }
          continue;
        }
        if (opcode == 5 && FSA == 1)
        {
          i++;
          offset[depth] = cheat.opcodes[i];
          depth++;
          continue;
        }
        if (opcode == 7 && FSA == 0)
        {
          i++;
          offset[depth] = cheat.opcodes[i];
          // success = true;
          no7 = false;
          continue;
          // break;
        }
        if (opcode == 6)
        {
          if (no7)
          {
            offset[depth] = 0;
          }
          switch (T)
          {
          case 1:
            bookmark.type = SEARCH_TYPE_UNSIGNED_8BIT;
            break;
          case 2:
            bookmark.type = SEARCH_TYPE_UNSIGNED_16BIT;
            break;
          case 4:
            bookmark.type = SEARCH_TYPE_UNSIGNED_32BIT;
            break;
          case 8:
            bookmark.type = SEARCH_TYPE_UNSIGNED_64BIT;
            break;
          default:
            printf("cheat code processing error, wrong width value\n");
            bookmark.type = SEARCH_TYPE_UNSIGNED_32BIT;
            break;
          }
          success = true;
          break;
        }
      }

      if (success)
      {
        // compute address
        printf("success ! \n");
        bookmark.pointer.depth = depth;
        u64 nextaddress = m_mainBaseAddr;
        printf("main[%lx]", nextaddress);
        u8 i = 0;
        for (int z = depth; z >= 0; z--)
        {
          // bookmark_t bm;
          bookmark.pointer.offset[z] = offset[i];
          printf("+%lx z=%d ", bookmark.pointer.offset[z], z);
          nextaddress += bookmark.pointer.offset[z];
          printf("[%lx]", nextaddress);
          // m_memoryDumpBookmark->addData((u8 *)&nextaddress, sizeof(u64));
          // m_AttributeDumpBookmark->addData((u8 *)&bm, sizeof(bookmark_t));
          MemoryInfo meminfo = m_debugger->queryMemory(nextaddress);
          if (meminfo.perm == Perm_Rw)
          {
            address = nextaddress;
            if (m_32bitmode)
              m_debugger->readMemory(&nextaddress, sizeof(u32), nextaddress);
            else
              m_debugger->readMemory(&nextaddress, sizeof(u64), nextaddress);
          }
          else
          {
            printf("*access denied*\n");
            success = false;
            // break;
          }
          printf("(%lx)", nextaddress);
          i++;
        }
        printf("\n");
      }
      if (success)
      {
        m_memoryDumpBookmark->addData((u8 *)&address, sizeof(u64));
        m_AttributeDumpBookmark->addData((u8 *)&bookmark, sizeof(bookmark_t));
        (new Snackbar("Adding pointer chain from cheat to bookmark"))->show();
      }
      else
      {
        if (bookmark.pointer.depth > 2) // depth of 2 means only one pointer hit high chance of wrong positive
        {
          (new MessageBox("Extracted pointer chain is broken on current memory state\n \n If the game is in correct state\n \n would you like to try to rebase the chain?", MessageBox::YES_NO))
              ->setSelectionAction([&](u8 selection) {
                if (selection)
                {
                  searchValue_t value;
                  while (!getinput("Enter the value at this memory", "You must know what type is the value and set it correctly in the search memory type setting", "", &value))
                  {
                  }
                  printf("value = %ld\n", value._u64);
                  rebasepointer(value); //bookmark);
                }
                Gui::g_currMessageBox->hide();
              })
              ->show();
        }
        else
          (new Snackbar("Not able to extract pointer chain from cheat"))->show();
      }

      // pointercheck(); //disable for now;
    }
    // end mod

    if (m_memoryDump->size() > 0)
    {
      if (kdown & KEY_LEFT)
        if (m_cheatCnt > 0)
        {
          m_menuLocation = CHEATS;
          if (m_memoryDump1 == nullptr)
          {
            m_selectedEntrySaveSR = m_selectedEntry;
            m_addresslist_offsetSaveSR = m_addresslist_offset;
          }
          else
          {
            m_selectedEntrySaveBM = m_selectedEntry;
            m_addresslist_offsetSaveBM = m_addresslist_offset;
          }

          m_selectedEntry = m_selectedEntrySaveCL;
          // cheatListOffset = 0;
        }

      if (kdown & KEY_RIGHT)
      {
        m_selectedEntrySaveCL = m_selectedEntry;
        m_menuLocation = CANDIDATES;
        if (m_memoryDump1 == nullptr)
        {
          m_selectedEntry = m_selectedEntrySaveSR;
          m_addresslist_offset = m_addresslist_offsetSaveSR;
        }
        else
        {
          m_selectedEntry = m_selectedEntrySaveBM;
          m_addresslist_offset = m_addresslist_offsetSaveBM;
        }

        // m_selectedEntry = 0;
        // cheatListOffset = 0;
      }
    }

    if (m_menuLocation == CANDIDATES)
    { /* Candidates menu */
      if (m_memoryDump->size() > 0)
      {
        if (kdown & KEY_X && m_memoryDump->getDumpInfo().dumpType == DumpType::ADDR && !(kheld & KEY_ZL))
        {
          if (!(m_debugger->m_dmnt))
          {
            m_debugger->detatch();
            dmntchtForceOpenCheatProcess();
            printf("force open called\n");
          }
          u64 address = 0;
          m_memoryDump->getData((m_selectedEntry + m_addresslist_offset) * sizeof(u64), &address, sizeof(u64));

          if (!_isAddressFrozen(address))
          {
            u64 outValue;
            if (m_memoryDump1 == nullptr)
            {
              if (R_SUCCEEDED(dmntchtEnableFrozenAddress(address, dataTypeSizes[m_searchType], &outValue)))
              {
                (new Snackbar("Froze variable!"))->show();
                m_frozenAddresses.insert({address, outValue});
              }
              else
                (new Snackbar("Failed to freeze variable!"))->show();
            }
            else
            {
              bookmark_t bookmark;
              m_AttributeDumpBookmark->getData((m_selectedEntry + m_addresslist_offset) * sizeof(bookmark_t), &bookmark, sizeof(bookmark_t));
              if (R_SUCCEEDED(dmntchtEnableFrozenAddress(address, dataTypeSizes[bookmark.type], &outValue)))
              {
                (new Snackbar("Froze variable!"))->show();
                m_frozenAddresses.insert({address, outValue});
              }
              else
                (new Snackbar("Failed to freeze variable!"))->show();
            }
          }
          else
          {
            if (R_SUCCEEDED(dmntchtDisableFrozenAddress(address)))
            {
              (new Snackbar("Unfroze variable!"))->show();
              m_frozenAddresses.erase(address);
            }
            else
              (new Snackbar("Failed to unfreeze variable!"))->show();
          }
        }
        // add bookmark
        if ((kdown & KEY_PLUS) && (kheld & KEY_ZL))
        {
          if (m_memoryDump1 != nullptr)
          {
            m_memoryDump = m_memoryDump1;
            m_memoryDump1 = nullptr;

            updatebookmark(true, true, false);

            m_memoryDump1 = m_memoryDump;
            m_memoryDump = m_memoryDumpBookmark;

            if (m_memoryDump->size() > 0)
            {
              if (m_memoryDump->size() / sizeof(u64) - 1 < m_selectedEntry)
                m_selectedEntry = m_memoryDump->size() / sizeof(u64) - 1;
            }
            else
            {
              m_menuLocation = CHEATS;
              m_selectedEntry = 0;
            };
          }
        }

        if (kdown & KEY_PLUS && m_memoryDump->getDumpInfo().dumpType == DumpType::ADDR && !(kheld & KEY_ZL))
        {
          if (m_memoryDump1 != nullptr)
          { //Bookmark case
            bookmark_t bookmark;
            m_AttributeDumpBookmark->getData((m_selectedEntry + m_addresslist_offset) * sizeof(bookmark_t), &bookmark, sizeof(bookmark_t));
            if (Gui::requestKeyboardInput("Enter Label", "Enter Label to add to bookmark .", bookmark.label, SwkbdType_QWERTY, bookmark.label, 18))
              m_AttributeDumpBookmark->putData((m_selectedEntry + m_addresslist_offset) * sizeof(bookmark_t), &bookmark, sizeof(bookmark_t));
          }
          else
          {
            u64 address = 0;
            m_memoryDump->getData((m_selectedEntry + m_addresslist_offset) * sizeof(u64), &address, sizeof(u64));

            bookmark_t bookmark;
            bookmark.type = m_searchType;
            Gui::requestKeyboardInput("Enter Label", "Enter Label to add to bookmark .", "", SwkbdType_QWERTY, bookmark.label, 18);

            if (address >= m_heapBaseAddr && address < m_heapEnd)
            {
              bookmark.offset = address - m_heapBaseAddr;
              bookmark.heap = true;
            }
            else if (address >= m_mainBaseAddr && address < m_mainend)
            {
              bookmark.offset = address - m_mainBaseAddr;
              bookmark.heap = false;
            }

            m_AttributeDumpBookmark->addData((u8 *)&bookmark, sizeof(bookmark_t));
            m_AttributeDumpBookmark->flushBuffer();

            m_memoryDumpBookmark->addData((u8 *)&address, sizeof(u64));
            m_memoryDumpBookmark->flushBuffer();
            MemoryDump *BMDump = new MemoryDump(EDIZON_DIR "/BMDump.dat", DumpType::HELPER, false);
            BMDump->addData((u8 *)&bookmark.label, 18);
            BMDump->addData((u8 *)&address, sizeof(u64));
            delete BMDump;
            (new Snackbar("Address added to bookmark!"))->show(); // prompt for label
            printf("%s\n", "PLUS key pressed2");
          }
        }
        // add bookmark end
        // show memory editor
        // BM1
        if (kdown & KEY_RSTICK && (kheld & KEY_ZL) && m_memoryDump->getDumpInfo().dumpType == DumpType::ADDR)
        {
          m_memoryDump->getData((m_selectedEntry + m_addresslist_offset) * sizeof(u64), &m_EditorBaseAddr, sizeof(u64));
          m_BookmarkAddr = m_EditorBaseAddr;
          m_AttributeDumpBookmark->getData((m_selectedEntry + m_addresslist_offset) * sizeof(bookmark_t), &m_bookmark, sizeof(bookmark_t));
          m_searchMenuLocation = SEARCH_editRAM;
          m_selectedEntrySave = m_selectedEntry;
          m_selectedEntry = (m_EditorBaseAddr % 16) / 4 + 11;
        }
        if (kdown & KEY_RSTICK && !(kheld & KEY_ZL) && m_memoryDump->getDumpInfo().dumpType == DumpType::ADDR)
        {
          m_memoryDump->getData((m_selectedEntry + m_addresslist_offset) * sizeof(u64), &m_EditorBaseAddr, sizeof(u64));
          m_BookmarkAddr = m_EditorBaseAddr;
          if (m_memoryDump1 != nullptr)
            m_AttributeDumpBookmark->getData((m_selectedEntry + m_addresslist_offset) * sizeof(bookmark_t), &m_bookmark, sizeof(bookmark_t));
          else
            m_bookmark.pointer.depth = 0;
          m_searchMenuLocation = SEARCH_editRAM2;
          m_selectedEntrySave = m_selectedEntry;
          m_selectedEntry = (m_EditorBaseAddr % 16) / 4 + 11;
          m_addressmod = m_EditorBaseAddr % 4;
          if (m_addressmod % dataTypeSizes[m_searchType] != 0)
            m_addressmod = 0;
          m_z = 0;
          m_bookmark.heap = false;
          if (m_memoryDump1 != nullptr)
          {
            m_searchType = m_bookmark.type;
            // populate stack if on pointer
            if (m_bookmark.pointer.depth > 0)
            {
              u64 nextaddress = m_mainBaseAddr;
              for (int z = m_bookmark.pointer.depth; z > 0; z--)
              {
                nextaddress += m_bookmark.pointer.offset[z];
                m_jump_stack[z].from = nextaddress;
                MemoryInfo meminfo = m_debugger->queryMemory(nextaddress);
                if (meminfo.perm == Perm_Rw)
                  m_debugger->readMemory(&nextaddress, ((m_32bitmode) ? sizeof(u32) : sizeof(u64)), nextaddress);
                else
                {
                  m_z = z ;
                  break;
                }
                m_jump_stack[z].to = nextaddress;
                m_z = z - 1;
              }
            }
            //ME3 data init for memory explorer
          }
          printf("bookmarkdepth =%ld m_z=%d\n", m_bookmark.pointer.depth, m_z);
        }

        if ((kdown & KEY_LSTICK) && (m_memoryDump->getDumpInfo().dumpType == DumpType::ADDR) && (m_memoryDump1 != nullptr))
        {
          printf("\nstart scan range select ....................\n");
          m_memoryDump->getData((m_selectedEntry + m_addresslist_offset) * sizeof(u64), &m_EditorBaseAddr, sizeof(u64));
          MemoryInfo meminfo = {0};
          u64 t_start = m_EditorBaseAddr - m_EditorBaseAddr % M_ALIGNMENT - (Config::getConfig()->extraMB + 1) * 1024 * 1024;
          u64 t_end = m_EditorBaseAddr - m_EditorBaseAddr % M_ALIGNMENT + (Config::getConfig()->extraMB + 1) * 1024 * 1024;
          if (t_end > m_heapEnd && m_heapEnd != 0)
            t_end = m_heapEnd;
          meminfo = m_debugger->queryMemory(t_start);
          u64 seg_endp = meminfo.addr + meminfo.size;
          meminfo.addr = t_start;
          if (seg_endp > t_end)
            meminfo.size = t_end - t_start;
          else
            meminfo.size = seg_endp - t_start;
          m_targetmemInfos.clear();
          m_targetmemInfos.push_back(meminfo);
          while (seg_endp < t_end)
          {
            meminfo = m_debugger->queryMemory(seg_endp);
            seg_endp = meminfo.addr + meminfo.size;
            if (seg_endp > t_end)
              meminfo.size = t_end - meminfo.addr ;
            m_targetmemInfos.push_back(meminfo);
          }
          save_meminfos();
          // MemoryDump *scaninfo = new MemoryDump((m_edizon_dir + "/scaninfo.dat").c_str(), DumpType::UNDEFINED, true);
          // scaninfo->setBaseAddresses(m_addressSpaceBaseAddr, m_heapBaseAddr, m_mainBaseAddr, m_heapSize, m_mainSize);
          // for (MemoryInfo meminfo : m_targetmemInfos)
          // {
          //   scaninfo->addData((u8 *)&meminfo, sizeof(meminfo));
          // }
          // scaninfo->flushBuffer();
          // delete scaninfo;

          // m_AttributeDumpBookmark->getData((m_selectedEntry + m_addresslist_offset) * sizeof(bookmark_t), &m_bookmark, sizeof(bookmark_t));
          // m_abort = false;
          // m_Time1 = time(NULL);
          // m_searchValue[0]._u64 = m_EditorBaseAddr - 0x800;
          // m_searchValue[1]._u64 = m_EditorBaseAddr;
          // if (m_pointersearch_canresume)
          //   resumepointersearch2();
          // else
          //   startpointersearch2(m_EditorBaseAddr);
          // printf("done pointer search \n");
          // printf("Time taken =%ld\n", time(NULL) - m_Time1);
        }
        // end
        if ((kdown & KEY_A) && (kheld & KEY_ZL))
        {
          if (m_memoryDump1 != nullptr)
          { // in bookmark mode
            m_memoryDump->getData((m_selectedEntry + m_addresslist_offset) * sizeof(u64), &m_EditorBaseAddr, sizeof(u64));
            // m_searchMenuLocation = SEARCH_POINTER;
            // m_showpointermenu = true;
            if (m_menuLocation == CANDIDATES && m_memoryDumpBookmark->size() != 0)
            {
              if (!(m_debugger->m_dmnt))
              {
                m_debugger->detatch();
                dmntchtForceOpenCheatProcess();
                printf("force open called\n");
              }
              bookmark_t bookmark;
              m_AttributeDumpBookmark->getData((m_selectedEntry + m_addresslist_offset) * sizeof(bookmark_t), &bookmark, sizeof(bookmark_t));
              // printf("m_selectedEntry + m_addresslist_offset %ld\n", m_selectedEntry + m_addresslist_offset);
              // u64 index = m_selectedEntry + m_addresslist_offset;
              if (bookmark.pointer.depth > 0)
              {
                addcodetofile(m_selectedEntry + m_addresslist_offset);
              }
              else
              {
                addstaticcodetofile(m_selectedEntry + m_addresslist_offset);
              }
              m_searchMenuLocation = SEARCH_NONE;
              (new Snackbar("Coded added to cheat file, reload to take effect"))->show();
            }
            // pointercheck();
            // (new Snackbar("Searching pointer "))->show();
          }
        }
        else if (kdown & KEY_A && m_memoryDump->getDumpInfo().dumpType == DumpType::ADDR)
        {
          searchType_t savetype = m_searchType;
          if (m_memoryDump1 != nullptr)
          {
            bookmark_t bookmark;
            m_AttributeDumpBookmark->getData((m_selectedEntry + m_addresslist_offset) * sizeof(bookmark_t), &bookmark, sizeof(bookmark_t));
            m_searchType = bookmark.type;
          }

          u64 address = 0;
          m_memoryDump->getData((m_selectedEntry + m_addresslist_offset) * sizeof(u64), &address, sizeof(u64));

          if (m_selectedEntry < 8)
          {
            char input[19];
            char initialString[21];
            // start mod address content edit Hex option

            //
            strcpy(initialString, _getAddressDisplayString(address, m_debugger, m_searchType).c_str());
            if (Gui::requestKeyboardInput("Enter value", "Enter a value that should get written at this .", initialString, m_searchValueFormat == FORMAT_DEC ? SwkbdType_NumPad : SwkbdType_QWERTY, input, 18))
            {
              if (m_searchValueFormat == FORMAT_HEX)
              {
                auto value = static_cast<u64>(std::stoul(input, nullptr, 16));
                m_debugger->writeMemory(&value, sizeof(value), address);
              }
              else if (m_searchType == SEARCH_TYPE_FLOAT_32BIT)
              {
                auto value = static_cast<float>(std::atof(input));
                m_debugger->writeMemory(&value, sizeof(value), address);
              }
              else if (m_searchType == SEARCH_TYPE_FLOAT_64BIT)
              {
                auto value = std::atof(input);
                m_debugger->writeMemory(&value, sizeof(value), address);
              }
              else if (m_searchType != SEARCH_TYPE_NONE)
              {
                auto value = std::atol(input);
                m_debugger->writeMemory((void *)&value, dataTypeSizes[m_searchType], address);
              }
            }
          }
          else if ((m_memoryDump->size() / sizeof(u64)) < 25)
          {
            std::vector<std::string> options;
            options.clear();

            std::stringstream ss;
            for (u32 i = 7; i < (m_memoryDump->size() / sizeof(u64)); i++)
            { //TODO: i?
              m_memoryDump->getData(m_selectedEntry * sizeof(u64), &address, sizeof(u64));
              ss.str("");
              ss << "0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << address;

              ss << " (" << _getAddressDisplayString(address, m_debugger, m_searchType);

              options.push_back(ss.str());
              printf("%s\n", ss.str().c_str());
            }

            (new ListSelector("Edit value at ", "\uE0E0 Edit value     \uE0E1 Back", options))->setInputAction([&](u32 k, u16 selectedItem) {
                                                                                                if (k & KEY_A)
                                                                                                {
                                                                                                  char input[16];
                                                                                                  char initialString[21];
                                                                                                  u64 selectedAddress;

                                                                                                  m_memoryDump->getData((selectedItem + 7) * sizeof(u64), &selectedAddress, sizeof(u64));

                                                                                                  strcpy(initialString, _getAddressDisplayString(selectedAddress, m_debugger, m_searchType).c_str());

                                                                                                  if (Gui::requestKeyboardInput("Enter value", "Enter a value for which the game's memory should be searched.", initialString, SwkbdType::SwkbdType_NumPad, input, 15))
                                                                                                  {
                                                                                                    u64 value = atol(input);
                                                                                                    if (value > dataTypeMaxValues[m_searchType] || value < dataTypeMinValues[m_searchType])
                                                                                                    {
                                                                                                      (new Snackbar("Entered value isn't inside the range of this data type. Please enter a different value."))->show();
                                                                                                      return;
                                                                                                    }

                                                                                                    m_memoryDump->getData((m_selectedEntry) * sizeof(u64), &selectedAddress, sizeof(u64));
                                                                                                    m_debugger->pokeMemory(dataTypeSizes[m_searchType], selectedAddress, value);
                                                                                                  }
                                                                                                }
                                                                                              })
                ->show();
          }
          else
            (new Snackbar("Too many addresses! Narrow down the selection before editing."))->show();

          m_searchType = savetype; // restore value
        }
      }
    }
    else
    { /* Cheats menu */
      if (kdown & KEY_A)
      {
        if (m_cheatCnt == 0)
          return;

        // count total opcode
        u32 opcodecount = m_cheats[m_selectedEntry].definition.num_opcodes;
        for (u32 i = 0; i < m_cheatCnt; i++)
        {
          if (m_cheats[i].enabled)
            opcodecount += m_cheats[i].definition.num_opcodes;
        }
        if (opcodecount > 0x400)
        {
          (new Snackbar("Total opcode count would exceed 1024!"))->show();
          return;
        }

        dmntchtToggleCheat(m_cheats[m_selectedEntry].cheat_id);
        u64 cheatCnt = 0;

        dmntchtGetCheatCount(&cheatCnt);
        if (cheatCnt > 0)
        {
          delete[] m_cheats;
          m_cheats = new DmntCheatEntry[cheatCnt];
          dmntchtGetCheats(m_cheats, cheatCnt, 0, &m_cheatCnt);
        }
      }
    }

    if ((kdown & KEY_MINUS) && (kheld & KEY_ZL))
    {
      if (m_memoryDump1 != nullptr)
      {
        m_memoryDump = m_memoryDump1;
        m_memoryDump1 = nullptr;

        updatebookmark(true, false, false);

        m_memoryDump1 = m_memoryDump;
        m_memoryDump = m_memoryDumpBookmark;

        if (m_memoryDump->size() > 0)
        {
          if (m_memoryDump->size() / sizeof(u64) - 1 < m_selectedEntry)
            m_selectedEntry = m_memoryDump->size() / sizeof(u64) - 1;
        }
        else
        {
          // m_selectedEntrySave = 0;
          m_selectedEntrySaveBM = 0;
          if (m_menuLocation != CHEATS)
          {
            m_menuLocation = CHEATS;
            m_selectedEntry = m_selectedEntrySaveCL;
          }
        };
      }
    }

    if ((kdown & KEY_MINUS) && !(kheld & KEY_ZL))
    {
      //make sure not using bookmark m_searchType
      if (m_menuLocation == CANDIDATES)
      {
        if (m_memoryDump1 != nullptr)
        { //Bookmark case
          bookmark_t bookmark;
          m_AttributeDumpBookmark->getData((m_selectedEntry + m_addresslist_offset) * sizeof(bookmark_t), &bookmark, sizeof(bookmark_t));
          bookmark.deleted = !bookmark.deleted;
          m_AttributeDumpBookmark->putData((m_selectedEntry + m_addresslist_offset) * sizeof(bookmark_t), &bookmark, sizeof(bookmark_t));
          // m_memoryDumpBookmark->flushBuffer();
          // m_memoryDump = m_memoryDump1;
          // m_memoryDump1 = nullptr;
        }
        else
        {

          // m_addresslist_offset = 0;
          m_selectedEntrySaveSR = 0;
          m_addresslist_offsetSaveSR = 0;
          // end mod
          if (m_memoryDump->size() == 0)
          {
            std::vector<std::string> options;

            if (m_frozenAddresses.size() == 0)
              return;

            std::stringstream ss;
            for (auto [addr, value] : m_frozenAddresses)
            {
              ss << "[ BASE + 0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << (addr - m_addressSpaceBaseAddr) << " ]  ";
              ss << "( " << std::dec << value << " )";
              options.push_back(ss.str());
              ss.str("");
            }

            (new ListSelector("Frozen Addresses", "\uE0E0 Unfreeze     \uE0E1 Back", options))->setInputAction([&](u32 k, u16 selectedItem) {
                                                                                                if (k & KEY_A)
                                                                                                {
                                                                                                  auto itr = m_frozenAddresses.begin();
                                                                                                  std::advance(itr, selectedItem);

                                                                                                  dmntchtDisableFrozenAddress(itr->first);
                                                                                                  m_frozenAddresses.erase(itr->first);
                                                                                                }
                                                                                              })
                ->show();
          }
          else
          {
            m_memoryDump->clear();
            m_use_range = false;
            removef(EDIZON_DIR "/memdump1.dat");
            removef(EDIZON_DIR "/memdump1a.dat");
            removef(EDIZON_DIR "/datadump2.dat");
            removef(EDIZON_DIR "/datadump2.datA");
            removef(EDIZON_DIR "/datadump2.datB");
            // removef(EDIZON_DIR "/memdump2.dat");
            // removef(EDIZON_DIR "/memdump3.dat");

            // m_searchType = SEARCH_TYPE_UNSIGNED_16BIT;
            // m_searchMode = SEARCH_MODE_RANGE;
            // m_searchRegion = SEARCH_REGION_MAIN;
            // m_searchValue[0]._u64 = 1;
            // m_searchValue[1]._u64 = 100;

            m_menuLocation = CHEATS;
          }
        }
      }
      else
      { // WIP working on cheat menu
        // m_cheatCnt
        m_cheatDelete[m_selectedEntry] = !m_cheatDelete[m_selectedEntry];
      }
    }
    // start mod KEY_PLUS
    // if (kdown & KEY_PLUS) {
    // printf("%s\n","PLUS key pressed");
    // printf("%s\n",titleNameStr.c_str());
    // printf("%s\n",tidStr.c_str());
    // printf("%s\n",buildIDStr.c_str());
    // Gui::g_nextGui = GUI_MAIN;
    // return;
    // }
    if ((kdown & KEY_R) && (kheld & KEY_ZL))
    {
      if (m_menuLocation == CANDIDATES && m_memoryDump1 != nullptr)
      {
        m_AttributeDumpBookmark->getData((m_selectedEntry + m_addresslist_offset) * sizeof(bookmark_t), &m_bookmark, sizeof(bookmark_t));
        if (m_bookmark.type < SEARCH_TYPE_FLOAT_64BIT)
        {
          u8 i = static_cast<u8>(m_bookmark.type) + 1;
          m_bookmark.type = static_cast<searchType_t>(i);
        }
        m_AttributeDumpBookmark->putData((m_selectedEntry + m_addresslist_offset) * sizeof(bookmark_t), &m_bookmark, sizeof(bookmark_t));
        m_AttributeDumpBookmark->flushBuffer();
      }
      else if (m_menuLocation == CANDIDATES && m_memoryDump1 == nullptr) // change type
      {
        if (m_searchType < SEARCH_TYPE_FLOAT_64BIT)
        {
          u8 i = static_cast<u8>(m_searchType) + 1;
          m_searchType = static_cast<searchType_t>(i);
        }
      }
    };

    if ((kdown & KEY_L) && (kheld & KEY_ZL))
    {
      if (m_menuLocation == CANDIDATES && m_memoryDump1 != nullptr)
      {
        m_AttributeDumpBookmark->getData((m_selectedEntry + m_addresslist_offset) * sizeof(bookmark_t), &m_bookmark, sizeof(bookmark_t));
        if (m_bookmark.type > SEARCH_TYPE_UNSIGNED_8BIT)
        {
          u8 i = static_cast<u8>(m_bookmark.type) - 1;
          m_bookmark.type = static_cast<searchType_t>(i);
        }
        m_AttributeDumpBookmark->putData((m_selectedEntry + m_addresslist_offset) * sizeof(bookmark_t), &m_bookmark, sizeof(bookmark_t));
        m_AttributeDumpBookmark->flushBuffer();
      }
      else if (m_menuLocation == CANDIDATES && m_memoryDump1 == nullptr) // Chang type
      {
        if (m_searchType > SEARCH_TYPE_UNSIGNED_8BIT)
        {
          u8 i = static_cast<u8>(m_searchType) - 1;
          m_searchType = static_cast<searchType_t>(i);
        }
      }
    };

    if ((kdown & KEY_R) && !(kheld & KEY_ZL))
    {
      if (m_searchValueFormat == FORMAT_HEX)
        m_searchValueFormat = FORMAT_DEC;
      else
        m_searchValueFormat = FORMAT_HEX;
      if (m_searchValueFormat == FORMAT_HEX)
        printf("%s\n", "HEX");
      printf("%s\n", "R key pressed");
    }
    if ((kdown & KEY_L) && !(kheld & KEY_ZL)) //toggle bookmark view bookmark : (m_memoryDump1 != nullptr)
    {
      if (m_memoryDump1 == nullptr)
      {
        // WIP
        // switch to bookmark
        // if (m_memoryDumpBookmark->size() == 0)
        // {
        //   updatebookmark(true, false);
        // }

        // if (m_memoryDumpBookmark->size() == 0)
        //   m_menuLocation = CHEATS;

        {
          m_memoryDump1 = m_memoryDump;
          m_memoryDump = m_memoryDumpBookmark;

          if (m_menuLocation == CANDIDATES)
          {
            m_selectedEntrySaveSR = m_selectedEntry;
            m_addresslist_offsetSaveSR = m_addresslist_offset;
            m_selectedEntry = m_selectedEntrySaveBM;
          }
          m_addresslist_offset = m_addresslist_offsetSaveBM;

          // if (m_memoryDump->size() == 0 && m_cheatCnt > 0)
          // {
          //   m_menuLocation = CHEATS;
          // };
          //consider to remove later
          if (m_searchType == SEARCH_TYPE_NONE)
            m_searchType = SEARCH_TYPE_UNSIGNED_32BIT; // to make sure not blank
          // end
          // (new Snackbar("Switch to bookmark List!"))->show();
          printf("%s\n", "Bookmark");
        }
      }
      else
      {
        m_memoryDump = m_memoryDump1;
        m_memoryDump1 = nullptr;

        if (m_menuLocation == CANDIDATES)
        {
          m_selectedEntrySaveBM = m_selectedEntry;
          m_addresslist_offsetSaveBM = m_addresslist_offset;
          m_selectedEntry = m_selectedEntrySaveSR;
        }
        m_addresslist_offset = m_addresslist_offsetSaveSR;

        // (new Snackbar("Switch to Normal List!"))->show();
      }

      if (m_memoryDumpBookmark->size() == 0 && m_menuLocation == CANDIDATES && m_cheatCnt > 0)
      {
        m_selectedEntry = m_selectedEntrySaveCL;
        m_menuLocation = CHEATS;
      }
      printf("%s\n", "L key pressed");
      // if (m_menuLocation == CANDIDATES)
      //   m_selectedEntry = 0;
    }

    if ((kdown & KEY_ZR) && !(kheld & KEY_ZL))
    {
      if (m_menuLocation == CHEATS)
      {
        cheatListOffset += 8;
        m_selectedEntry += 8;
        if (cheatListOffset >= m_cheatCnt)
        {
          cheatListOffset -= 8;
          m_selectedEntry -= 8;
        }
        if (m_selectedEntry + 1 > m_cheatCnt)
          m_selectedEntry = m_cheatCnt - 1;
      }
      else
      {
        m_addresslist_offset += 8;
        if (m_addresslist_offset >= (m_memoryDump->size() / sizeof(u64)))
          m_addresslist_offset -= 8;
        if (m_selectedEntry + m_addresslist_offset + 1 > (m_memoryDump->size() / sizeof(u64)))
          m_selectedEntry = (m_memoryDump->size() / sizeof(u64)) % 8 - 1;
        // printf("%s\n", "ZR key pressed");
      }
    }

    if ((kdown & KEY_ZR) && (kheld & KEY_ZL))
    {
      if (m_menuLocation == CHEATS)
      {
        if (cheatListOffset >= 8)
        {
          cheatListOffset -= 8;
          m_selectedEntry -= 8;
        }
        else
        {
          cheatListOffset = 0;
          m_selectedEntry = 0;
        }
      }
      else
      {
        if (m_addresslist_offset >= 8)
          m_addresslist_offset -= 8;
      }
      // printf("%s\n", "ZL key pressed");
    }

    // End Mod
    // hidScanInput();
    if ((kdown & KEY_Y) && (kheld & KEY_ZL))
    {
      if (m_searchMenuLocation == SEARCH_NONE)
      {
        if (m_memoryDump1 != nullptr)
        { // in bookmark mode
          m_memoryDump->getData((m_selectedEntry + m_addresslist_offset) * sizeof(u64), &m_EditorBaseAddr, sizeof(u64));
          m_AttributeDumpBookmark->getData((m_selectedEntry + m_addresslist_offset) * sizeof(bookmark_t), &m_bookmark, sizeof(bookmark_t));
          m_searchMenuLocation = SEARCH_POINTER;
          PSresumeSTATE();
          // m_showpointermenu = true;
        }
        else
        {
          m_searchMenuLocation = SEARCH_VALUE;
            m_selectedEntry = 1;
        }
      }
    }
    // if ((kdown & KEY_X) && (kheld & KEY_ZL))
    // {
    //   printf("resume \n");
    //   resumepointersearch2();
    // }
    if ((kdown & KEY_Y) && !(kheld & KEY_ZL))
    {
      if (m_searchMenuLocation == SEARCH_NONE)
      {
        if ((Config::getConfig()->extra_value) && (m_memoryDump->size() == 0))
        { // enter multi search
          m_searchMenuLocation = SEARCH_editExtraSearchValues;
          m_selectedEntrySave = m_selectedEntry;
          m_selectedEntry = 0;
          load_multisearch_setup();
        }
        else if (m_searchMode == SEARCH_MODE_NONE) 
        {
          m_searchMenuLocation = SEARCH_MODE;
          if (m_selectedEntry > 11)
            m_selectedEntry = 0;
          // m_selectedEntry = m_searchType == SEARCH_TYPE_NONE ? 0 : m_searchType;
        }
        else
          m_searchMenuLocation = SEARCH_VALUE;
        // auto toggle between stateA and stateB
        if (m_searchMode == SEARCH_MODE_DIFFA)
        {
          m_searchMode = SEARCH_MODE_SAMEA;
          m_selectedEntry = 1;
        }
        else if (m_searchMode == SEARCH_MODE_SAMEA)
        {
          m_searchMode = SEARCH_MODE_DIFFA;
          m_selectedEntry = 1;
        }
        // else
        //   m_selectedEntry = 0;

        // cheatListOffset = 0;
      }
      printf("%s\n", "Y key pressed");
      if (Title::g_titles[m_debugger->getRunningApplicationTID()] != nullptr)
        printf("%s\n", Title::g_titles[m_debugger->getRunningApplicationTID()]->getTitleName().c_str());
      printf("%s\n", tidStr.c_str());
      printf("%s\n", buildIDStr.c_str());
      GuiCheats::_writegameid();
      //make sure not using bookmark
      // if (m_memoryDump1 != nullptr)
      // {
      //   m_memoryDump = m_memoryDump1;
      //   m_memoryDump1 = nullptr;
      // }
      // m_addresslist_offset = 0;
      // end mod
    }
  }
  else
  {
    if ((m_searchMenuLocation == SEARCH_TYPE && m_searchType == SEARCH_TYPE_NONE) ||
        (m_searchMenuLocation == SEARCH_MODE && m_searchMode == SEARCH_MODE_NONE) ||
        (m_searchMenuLocation == SEARCH_REGION && m_searchRegion == SEARCH_REGION_NONE) ||
        (m_searchMenuLocation == SEARCH_VALUE) ||
        (m_searchMenuLocation == SEARCH_editRAM))
    {
      if (kdown & KEY_UP)
      {
        switch (m_searchMenuLocation)
        {
        case SEARCH_TYPE:
        // [[fallthrough]]
        case SEARCH_MODE:
          if (m_selectedEntry % 2 == 1)
            m_selectedEntry--;
          break;
        case SEARCH_REGION:
          if (m_selectedEntry > 0)
            m_selectedEntry--;
          break;
        case SEARCH_VALUE:
          m_selectedEntry = 0;
          break;
        case SEARCH_NONE:
        case SEARCH_POINTER:
          break;
        case SEARCH_editRAM2:
        case SEARCH_editRAM: // need UP
          if (m_selectedEntry > 4)
            m_selectedEntry -= 5;
          else
          {
            m_EditorBaseAddr -= 0x10;
          }

          break;
        case SEARCH_editExtraSearchValues:
        case SEARCH_pickjump:
          break;
        }
      }

      if (kdown & KEY_DOWN)
      {
        switch (m_searchMenuLocation)
        {
        case SEARCH_TYPE:
        // [[fallthrough]]
        case SEARCH_MODE:
          if ((m_selectedEntry + 1) < 12 && m_selectedEntry % 2 == 0)
            m_selectedEntry++;
          break;
        case SEARCH_REGION:
          if (m_selectedEntry < 3)
            m_selectedEntry++;
          break;
        case SEARCH_VALUE:
          if (m_searchType != SEARCH_TYPE_NONE && m_searchMode != SEARCH_MODE_NONE && m_searchRegion != SEARCH_REGION_NONE)
            m_selectedEntry = 1;
          break;
        case SEARCH_NONE:
        case SEARCH_POINTER:
          break;
        case SEARCH_editRAM2:
        case SEARCH_editRAM: // need DOWN
          if (m_selectedEntry < 35)
            m_selectedEntry += 5;
          else
          {
            m_EditorBaseAddr += 0x10;
          }

          break;
        case SEARCH_editExtraSearchValues:
        case SEARCH_pickjump:
          break;
        }
      }

      if (kdown & KEY_LEFT)
      {
        switch (m_searchMenuLocation)
        {
        case SEARCH_TYPE:
        // [[fallthrough]]
        case SEARCH_MODE:
          if (m_selectedEntry >= 2)
            m_selectedEntry -= 2;
          break;
        case SEARCH_REGION:
          break;
        case SEARCH_VALUE:
          if (m_searchValueIndex == 1)
            m_searchValueIndex--;
          break;
        case SEARCH_NONE:
        case SEARCH_POINTER:
          break;
        case SEARCH_editRAM2:
        case SEARCH_editRAM: // need LEFT
          if (m_selectedEntry % 5 > 1)
            m_selectedEntry--;
          break;
        case SEARCH_editExtraSearchValues:
        case SEARCH_pickjump:
          break;
        }
      }

      if (kdown & KEY_RIGHT)
      {
        switch (m_searchMenuLocation)
        {
        case SEARCH_TYPE:
        // [[fallthrough]]
        case SEARCH_MODE:
          if (m_selectedEntry <= 9)
            m_selectedEntry += 2;
          break;
        case SEARCH_REGION:
          break;
        case SEARCH_VALUE:
          if (m_searchValueIndex == 0 && (m_searchMode == SEARCH_MODE_RANGE || m_use_range))
            m_searchValueIndex++;
          break;
        case SEARCH_NONE:
        case SEARCH_POINTER:
          break;
        case SEARCH_editRAM2:
        case SEARCH_editRAM: // need RIGHT
          if (m_selectedEntry % 5 < 4)
            m_selectedEntry++;
          break;
        case SEARCH_editExtraSearchValues:
        case SEARCH_pickjump:
          break;
        }
      }

      if (m_searchMenuLocation == SEARCH_editRAM)
      {
        if (kdown & KEY_PLUS)
        {
          u64 address = m_EditorBaseAddr - (m_EditorBaseAddr % 16) - 0x20 + (m_selectedEntry - 1 - (m_selectedEntry / 5)) * 4 + m_addressmod;

          bookmark_t bookmark;
          if (address >= m_heapBaseAddr && address < m_heapEnd)
          {
            bookmark.offset = address - m_heapBaseAddr;
            bookmark.heap = true;
          }
          else if (address >= m_mainBaseAddr && address < m_mainend)
          {
            bookmark.offset = address - m_mainBaseAddr;
            bookmark.heap = false;
          }

          bookmark.type = m_searchType;
          Gui::requestKeyboardInput("Enter Label", "Enter Label to add to bookmark .", "", SwkbdType_QWERTY, bookmark.label, 18);
          m_AttributeDumpBookmark->addData((u8 *)&bookmark, sizeof(bookmark_t));
          m_memoryDumpBookmark->addData((u8 *)&address, sizeof(u64));
          if (m_bookmark.pointer.depth > 0)
          {
            s64 offset = address - m_BookmarkAddr + m_bookmark.pointer.offset[0];
            // printf("address = %lx m_EditorBaseAddr = %lx m_bookmark.pointer.offset[0] = %lx m_addressmod = %lx\n", address, m_EditorBaseAddr, m_bookmark.pointer.offset[0],m_addressmod);
            if (offset >= 0 && offset < (s64)m_max_range)
            {
              memcpy(&(bookmark.pointer), &(m_bookmark.pointer), (m_bookmark.pointer.depth + 2) * 8);
              bookmark.pointer.offset[0] = (u64)offset;
              m_AttributeDumpBookmark->addData((u8 *)&bookmark, sizeof(bookmark_t));
              m_memoryDumpBookmark->addData((u8 *)&address, sizeof(u64));
            }
          }
          m_AttributeDumpBookmark->flushBuffer();
          m_memoryDumpBookmark->flushBuffer();
          MemoryDump *BMDump = new MemoryDump(EDIZON_DIR "/BMDump.dat", DumpType::HELPER, false);
          BMDump->addData((u8 *)&bookmark.label, 18);
          BMDump->addData((u8 *)&address, sizeof(u64));
          delete BMDump;
          (new Snackbar("Address added to bookmark!"))->show();
          printf("%s\n", "PLUS key pressed3");
        }
        if (kdown & KEY_ZR)
        {
          m_EditorBaseAddr += 0x80;
        }
        if (kdown & KEY_ZL)
        {
          m_EditorBaseAddr -= 0x80;
        }
        if (kdown & KEY_R)
        {
          m_addressmod++;
          m_addressmod = m_addressmod % 4;
        }
        if (kdown & KEY_L)
        {
          m_addressmod--;
          m_addressmod = m_addressmod % 4;
        }
        if (kdown & KEY_X)
        {
          if (m_searchValueFormat == FORMAT_DEC)
            m_searchValueFormat = FORMAT_HEX;
          else
            m_searchValueFormat = FORMAT_DEC;
        }
        if (kdown & KEY_Y) // BM9
        {
          u64 address = m_EditorBaseAddr - (m_EditorBaseAddr % 16) - 0x20 + (m_selectedEntry - 1 - (m_selectedEntry / 5)) * 4 + m_addressmod;
          std::stringstream ss;
          ss << "0x" << std::uppercase << std::hex << address;
          char input[19];
          if (Gui::requestKeyboardInput("Enter Address", "Enter Address to add to bookmark .", ss.str(), SwkbdType_QWERTY, input, 18))
          {
            address = static_cast<u64>(std::stoul(input, nullptr, 16));

            bookmark_t bookmark;
            bookmark.type = m_searchType;
            Gui::requestKeyboardInput("Enter Label", "Enter Label to add to bookmark .", "", SwkbdType_QWERTY, bookmark.label, 18);
            m_AttributeDumpBookmark->addData((u8 *)&bookmark, sizeof(bookmark_t));
            m_AttributeDumpBookmark->flushBuffer();

            (new Snackbar("Address added to bookmark!"))->show();
            m_memoryDumpBookmark->addData((u8 *)&address, sizeof(u64));
            m_memoryDumpBookmark->flushBuffer();
          }
        }
      }

      // inc and dec search value
      if ((kdown & KEY_ZR) && (m_searchMenuLocation == SEARCH_VALUE))
      {
        switch (m_searchType)
        {
        case SEARCH_TYPE_FLOAT_32BIT:
          m_searchValue[m_searchValueIndex]._f32++;
          break;
        case SEARCH_TYPE_FLOAT_64BIT:
          m_searchValue[m_searchValueIndex]._f64++;
          break;
        default:
          m_searchValue[m_searchValueIndex]._u64++;
        }
        m_selectedEntry = 1;
      };
      if ((kdown & KEY_ZL) && (m_searchMenuLocation == SEARCH_VALUE))
      {
        switch (m_searchType)
        {
        case SEARCH_TYPE_FLOAT_32BIT:
          m_searchValue[m_searchValueIndex]._f32--;
          break;
        case SEARCH_TYPE_FLOAT_64BIT:
          m_searchValue[m_searchValueIndex]._f64--;
          break;
        default:
          m_searchValue[m_searchValueIndex]._u64--;
        }
        m_selectedEntry = 1;
      };
      if (m_searchMenuLocation == SEARCH_VALUE)
      {
        if (kdown & KEY_DUP)
        {
          m_searchMode = SEARCH_MODE_SAME;
          {
            std::string s = m_edizon_dir + "/datadump2.datB";
            if (access(s.c_str(), F_OK) == 0)
            {
              if (m_store_extension == "B")
                m_store_extension = "A";
              else
                m_store_extension = "B";
              memcpy(Config::getConfig()->store_extension, m_store_extension.c_str(), m_store_extension.size());
              Config::getConfig()->store_extension[m_store_extension.size()] = 0;
              Config::writeConfig();
            }
          };
        }
        else if (kdown & KEY_DDOWN)
        {
          m_searchMode = SEARCH_MODE_DIFF;
        }
        else if (kdown & KEY_DLEFT)
        {
          m_searchMode = SEARCH_MODE_DEC;
        }
        else if (kdown & KEY_DRIGHT)
        {
          m_searchMode = SEARCH_MODE_INC;
        }
        else if (kdown & KEY_PLUS)
        {
          m_searchMode = SEARCH_MODE_RANGE;
        }   
        else if (kdown & KEY_MINUS)
        {
          if (m_searchMode == SEARCH_MODE_RANGE)
          {
            m_searchMode = SEARCH_MODE_EQ;
            m_searchValue[0]._u64 = 0;
            m_selectedEntry = 1;
          }
          else if (m_searchMode == SEARCH_MODE_EQ)
          {
            m_searchMode = SEARCH_MODE_NEQ;
            m_selectedEntry = 1;
          }
          else
          {
            m_searchMode = SEARCH_MODE_EQ;
          }
          m_searchValueIndex = 0;  
        }
        else if (kdown & KEY_RSTICK)
        {
          m_searchMode = SEARCH_MODE_RANGE;
          m_searchType = SEARCH_TYPE_FLOAT_32BIT;
          m_searchValue[0]._f32 = 0.1;
          m_searchValue[1]._f32 = 2000; 
          m_searchRegion = SEARCH_REGION_HEAP_AND_MAIN;
          m_selectedEntry = 1;
        }
        else if (kdown & KEY_LSTICK)
        {
          if (m_searchType == SEARCH_TYPE_FLOAT_32BIT)
          {
            m_searchType = SEARCH_TYPE_FLOAT_64BIT;
          }
          else
            m_searchType = SEARCH_TYPE_FLOAT_32BIT;
          m_searchValueIndex = 0;
          m_selectedEntry = 0;
          m_searchValue[0]._u64 = 0;
          m_searchValue[1]._u64 = 0;
        }
        else if (kdown & KEY_Y)
        {
          if (m_searchType == SEARCH_TYPE_UNSIGNED_32BIT)
          {
            m_searchType = SEARCH_TYPE_UNSIGNED_8BIT;
          }
          else if (m_searchType == SEARCH_TYPE_UNSIGNED_8BIT)
          {
            m_searchType = SEARCH_TYPE_UNSIGNED_16BIT;
          }
          else
            m_searchType = SEARCH_TYPE_UNSIGNED_32BIT;

          m_searchMode = SEARCH_MODE_EQ;
          m_searchValueIndex = 0;
          m_selectedEntry = 0;
          m_searchValue[0]._u64 = 0;
          m_searchValue[1]._u64 = 0;
          m_searchRegion = SEARCH_REGION_HEAP_AND_MAIN;
        }
        else if (kdown & KEY_R)
        {
          if (m_searchType == SEARCH_TYPE_FLOAT_32BIT)
          {
            m_searchValue[0]._f32 = 0 - m_searchValue[0]._f32;
            m_searchValue[1]._f32 = 0 - m_searchValue[1]._f32;
          }
          else if (m_searchType == SEARCH_TYPE_FLOAT_64BIT)
          {
            m_searchValue[0]._f64 = 0 - m_searchValue[0]._f64;
            m_searchValue[1]._f64 = 0 - m_searchValue[1]._f64;
          }
        };
      }
      if (kdown & KEY_A)
      {
        if (m_searchMenuLocation == SEARCH_editRAM)
        { // BM3
          // EditRAM routine
          // to update to use L and R to select type and display it on the top line
          u64 address = m_EditorBaseAddr - (m_EditorBaseAddr % 16) - 0x20 + (m_selectedEntry - 1 - (m_selectedEntry / 5)) * 4 + m_addressmod;
          char input[19];
          char initialString[21];
          strcpy(initialString, _getAddressDisplayString(address, m_debugger, m_searchType).c_str());
          if (Gui::requestKeyboardInput("Enter value", "Enter a value that should get written at this .", initialString, m_searchValueFormat == FORMAT_DEC ? SwkbdType_NumPad : SwkbdType_QWERTY, input, 18))
          {
            if (m_searchValueFormat == FORMAT_HEX)
            {
              auto value = static_cast<u64>(std::stoul(input, nullptr, 16));
              m_debugger->writeMemory(&value, dataTypeSizes[m_searchType], address);
            }
            else if (m_searchType == SEARCH_TYPE_FLOAT_32BIT)
            {
              auto value = static_cast<float>(std::atof(input));
              m_debugger->writeMemory(&value, sizeof(value), address);
            }
            else if (m_searchType == SEARCH_TYPE_FLOAT_64BIT)
            {
              auto value = std::atof(input);
              m_debugger->writeMemory(&value, sizeof(value), address);
            }
            else if (m_searchType != SEARCH_TYPE_NONE)
            {
              auto value = std::atol(input);
              m_debugger->writeMemory((void *)&value, dataTypeSizes[m_searchType], address);
            }
          }
        }
        else if (m_searchMenuLocation == SEARCH_TYPE)
          m_searchType = static_cast<searchType_t>(m_selectedEntry);
        else if (m_searchMenuLocation == SEARCH_REGION)
          m_searchRegion = static_cast<searchRegion_t>(m_selectedEntry);
        else if (m_searchMenuLocation == SEARCH_MODE)
          m_searchMode = static_cast<searchMode_t>(m_selectedEntry);
        else if (m_searchMenuLocation == SEARCH_VALUE)
        {
          if (m_selectedEntry == 0)
          {
            m_selectedEntry = 1;
            char str[0x21];
            // Start Mod keep previous value
            // End Mod
            if ((m_searchValue[m_searchValueIndex]._u32 > 10) || (m_searchValueFormat == FORMAT_HEX))
            {
              Gui::requestKeyboardInput("Enter the value you want to search for", "Based on your previously chosen options, EdiZon will expect different input here.", _getValueDisplayString(m_searchValue[m_searchValueIndex], m_searchType), m_searchValueFormat == FORMAT_DEC ? SwkbdType_NumPad : SwkbdType_QWERTY, str, 0x20);
            }
            else
            {
              Gui::requestKeyboardInput("Enter the value you want to search for", "Based on your previously chosen options, EdiZon will expect different input here.", "", m_searchValueFormat == FORMAT_DEC ? SwkbdType_NumPad : SwkbdType_QWERTY, str, 0x20);
            }
            if (std::string(str) == "")
              return;
            m_searchValue[m_searchValueIndex]._u64 = 0; //hack to fix bug elsewhere
            if (m_searchValueFormat == FORMAT_HEX)
            {
              m_searchValue[m_searchValueIndex]._u64 = static_cast<u64>(std::stoul(str, nullptr, 16));
            }
            else
            {
              switch (m_searchType)
              {
              case SEARCH_TYPE_UNSIGNED_8BIT:
                m_searchValue[m_searchValueIndex]._u8 = static_cast<u8>(std::stoul(str, nullptr, 0));
                break;
              case SEARCH_TYPE_UNSIGNED_16BIT:
                m_searchValue[m_searchValueIndex]._u16 = static_cast<u16>(std::stoul(str, nullptr, 0));
                break;
              case SEARCH_TYPE_UNSIGNED_32BIT:
                m_searchValue[m_searchValueIndex]._u32 = static_cast<u32>(std::stoul(str, nullptr, 0));
                break;
              case SEARCH_TYPE_UNSIGNED_64BIT:
                m_searchValue[m_searchValueIndex]._u64 = static_cast<u64>(std::stoul(str, nullptr, 0));
                break;
              case SEARCH_TYPE_SIGNED_8BIT:
                m_searchValue[m_searchValueIndex]._s8 = static_cast<s8>(std::stol(str, nullptr, 0));
                break;
              case SEARCH_TYPE_SIGNED_16BIT:
                m_searchValue[m_searchValueIndex]._s16 = static_cast<s16>(std::stol(str, nullptr, 0));
                break;
              case SEARCH_TYPE_SIGNED_32BIT:
                m_searchValue[m_searchValueIndex]._s32 = static_cast<s32>(std::stol(str, nullptr, 0));
                break;
              case SEARCH_TYPE_SIGNED_64BIT:
                m_searchValue[m_searchValueIndex]._s64 = static_cast<s64>(std::stol(str, nullptr, 0));
                break;
              case SEARCH_TYPE_FLOAT_32BIT:
                m_searchValue[m_searchValueIndex]._f32 = static_cast<float>(std::stof(str));
                break;
              case SEARCH_TYPE_FLOAT_64BIT:
                m_searchValue[m_searchValueIndex]._f64 = static_cast<double>(std::stod(str));
                break;
              case SEARCH_TYPE_POINTER:
                m_searchValue[m_searchValueIndex]._u64 = static_cast<u64>(std::stol(str));
                break;
              case SEARCH_TYPE_NONE:
                break;
              }
            }
          }
          else if (m_selectedEntry == 1) // search
          {if (m_searched) 
          {
            if (m_memoryDump1 == nullptr)
              m_memoryDump->setSearchParams(m_searchType, m_searchMode, m_searchRegion, m_searchValue[0], m_searchValue[1], m_use_range);
            else
              m_memoryDump1->setSearchParams(m_searchType, m_searchMode, m_searchRegion, m_searchValue[0], m_searchValue[1], m_use_range);
            (new Snackbar("Already did one search for this session, relaunch to do another"))->show();
          }
          else
          {
            m_searched = true;
            (new MessageBox("Traversing title memory.\n \nThis may take a while...", MessageBox::NONE))->show();
            requestDraw();

            overclockSystem(true);

            if (m_searchMode == SEARCH_MODE_POINTER)
              m_searchType = SEARCH_TYPE_UNSIGNED_64BIT;
            if (m_memoryDump1 != nullptr)
            {
              updatebookmark(true,false,true);
              m_memoryDump = m_memoryDumpBookmark;
            }
            else if (m_searchMode == SEARCH_MODE_SAME || m_searchMode == SEARCH_MODE_DIFF || m_searchMode == SEARCH_MODE_INC || m_searchMode == SEARCH_MODE_DEC || m_searchMode == SEARCH_MODE_DIFFA || m_searchMode == SEARCH_MODE_SAMEA)
            {
              if (m_memoryDump->size() == 0)
              {
                delete m_memoryDump;
                m_use_range = false;
                if (Config::getConfig()->enabletargetedscan && m_targetmemInfos.size() != 0)
                  GuiCheats::searchMemoryValuesPrimary(m_debugger, m_searchType, m_searchMode, m_searchRegion, &m_memoryDump, m_targetmemInfos);
                else
                  GuiCheats::searchMemoryValuesPrimary(m_debugger, m_searchType, m_searchMode, m_searchRegion, &m_memoryDump, m_memoryInfo);
                printf("%s%lx\n", "Dump Size = ", m_memoryDump->size());
              }
              else if (m_memoryDump->getDumpInfo().dumpType == DumpType::DATA)
              {
                printf("%s%lx\n", "Dump Size = ", m_memoryDump->size());
                if (Config::getConfig()->enabletargetedscan && m_targetmemInfos.size() != 0)
                  GuiCheats::searchMemoryValuesSecondary(m_debugger, m_searchType, m_searchMode, m_searchRegion, &m_memoryDump, m_targetmemInfos);
                else
                  GuiCheats::searchMemoryValuesSecondary(m_debugger, m_searchType, m_searchMode, m_searchRegion, &m_memoryDump, m_memoryInfo);
                delete m_memoryDump;
                // remove(EDIZON_DIR "/memdump1.dat");
                // rename(EDIZON_DIR "/memdump3.dat", EDIZON_DIR "/memdump1.dat");
                // printf("%s\n", "renaming");
                std::string s = m_edizon_dir + "/memdump1.dat";
                REPLACEFILE(EDIZON_DIR "/memdump3.dat", s.c_str());
                m_memoryDump = new MemoryDump(EDIZON_DIR "/memdump1.dat", DumpType::ADDR, false);
              }
              else if (m_memoryDump->getDumpInfo().dumpType == DumpType::ADDR)
              {
                if (m_searchMode == SEARCH_MODE_DIFFA || m_searchMode == SEARCH_MODE_SAMEA)
                {
                  if (Config::getConfig()->enabletargetedscan && m_targetmemInfos.size() != 0)
                    GuiCheats::searchMemoryValuesTertiary(m_debugger, m_searchValue[0], m_searchValue[1], m_searchType, m_searchMode, m_searchRegion, m_use_range, &m_memoryDump, m_targetmemInfos);
                  else
                    GuiCheats::searchMemoryValuesTertiary(m_debugger, m_searchValue[0], m_searchValue[1], m_searchType, m_searchMode, m_searchRegion, m_use_range, &m_memoryDump, m_memoryInfo);
                  delete m_memoryDump;
                  // remove(EDIZON_DIR "/memdump1.dat");
                  // remove(EDIZON_DIR "/memdump1a.dat");
                  // rename(EDIZON_DIR "/memdump3.dat", EDIZON_DIR "/memdump1.dat");
                  // rename(EDIZON_DIR "/memdump3a.dat", EDIZON_DIR "/memdump1a.dat");
                  std::string s = m_edizon_dir + "/memdump1.dat";
                  REPLACEFILE(EDIZON_DIR "/memdump3.dat", s.c_str());
                  s = m_edizon_dir + "/memdump1a.dat";
                  REPLACEFILE(EDIZON_DIR "/memdump3a.dat", s.c_str());
                  m_memoryDump = new MemoryDump(EDIZON_DIR "/memdump1.dat", DumpType::ADDR, false);
                  // remove(EDIZON_DIR "/datadump2.dat");
                  // rename(EDIZON_DIR "/datadump4.dat", EDIZON_DIR "/datadump2.dat");
                  s = m_edizon_dir + "/datadump2.dat";
                  REPLACEFILE(EDIZON_DIR "/datadump4.dat",s.c_str() );
                  // // rename B to A
                  // remove(EDIZON_DIR "/datadumpA.dat");
                  // rename(EDIZON_DIR "/datadumpAa.dat", EDIZON_DIR "/datadumpA.dat");
                  REPLACEFILE(EDIZON_DIR "/datadumpAa.dat", EDIZON_DIR "/datadumpA.dat")
                  // remove(EDIZON_DIR "/datadumpB.dat");
                  // rename(EDIZON_DIR "/datadumpBa.dat", EDIZON_DIR "/datadumpB.dat");
                  REPLACEFILE(EDIZON_DIR "/datadumpBa.dat", EDIZON_DIR "/datadumpB.dat");
                }
                else
                {
                  m_nothingchanged = false;
                  GuiCheats::searchMemoryAddressesSecondary2(m_debugger, m_searchValue[0], m_searchValue[1], m_searchType, m_searchMode, &m_memoryDump);
                  if (m_nothingchanged == false)
                  {
                    // remove(EDIZON_DIR "/memdump1a.dat");                              // remove old helper
                    // rename(EDIZON_DIR "/memdump3a.dat", EDIZON_DIR "/memdump1a.dat"); // rename new helper to current helper
                    std::string s = m_edizon_dir + "/memdump1a.dat";
                    REPLACEFILE(EDIZON_DIR "/memdump3a.dat", s.c_str());
                  }
                  // else
                  // {
                  //   std::string s = m_edizon_dir + "/datadump2.dat";
                  //   REPLACEFILE(EDIZON_DIR "/predatadump2.dat", s.c_str());
                  // }
                }
              }
            }
            else
            {
              if (m_memoryDump->size() == 0)
              {
                delete m_memoryDump;
                if (Config::getConfig()->enabletargetedscan && m_targetmemInfos.size() != 0 )
                  GuiCheats::searchMemoryAddressesPrimary(m_debugger, m_searchValue[0], m_searchValue[1], m_searchType, m_searchMode, m_searchRegion, &m_memoryDump, m_targetmemInfos);
                else
                  GuiCheats::searchMemoryAddressesPrimary(m_debugger, m_searchValue[0], m_searchValue[1], m_searchType, m_searchMode, m_searchRegion, &m_memoryDump, m_memoryInfo);
              }
              else
              {
                m_nothingchanged = false;
                GuiCheats::searchMemoryAddressesSecondary(m_debugger, m_searchValue[0], m_searchValue[1], m_searchType, m_searchMode, m_use_range, &m_memoryDump);
                if (m_nothingchanged == false)
                {
                  // remove(EDIZON_DIR "/memdump1a.dat");                              // remove old helper
                  // rename(EDIZON_DIR "/memdump3a.dat", EDIZON_DIR "/memdump1a.dat"); // rename new helper to current helper
                  std::string s = m_edizon_dir + "/memdump1a.dat";
                  REPLACEFILE(EDIZON_DIR "/memdump3a.dat", s.c_str());
                }
              }
            }

            overclockSystem(false);

            Gui::g_currMessageBox->hide();

            m_searchMenuLocation = SEARCH_NONE;
            // m_searchMode = SEARCH_MODE_NONE;
          }
          }
        }
      }
    }

    if (kdown & KEY_X && !(kheld & KEY_ZL))
    {
      if (m_searchMenuLocation == SEARCH_VALUE)
      {
        if (m_searchValueFormat == FORMAT_DEC)
          m_searchValueFormat = FORMAT_HEX;
        else
          m_searchValueFormat = FORMAT_DEC;
      }
    }
    else if (kdown & KEY_X && (kheld & KEY_ZL))
    {
      // key available
    }

    if (kdown & KEY_L)
    {
      if (m_searchMenuLocation == SEARCH_VALUE)
      {
        m_searchMenuLocation = SEARCH_REGION;
        m_selectedEntry = m_searchRegion == SEARCH_REGION_NONE ? 0 : static_cast<u32>(m_searchRegion);
      }
      else if (m_searchMenuLocation == SEARCH_REGION)
      {
        m_searchMenuLocation = SEARCH_MODE;
        m_selectedEntry = m_searchMode == SEARCH_MODE_NONE ? 0 : static_cast<u32>(m_searchMode);
      }
      else if (m_searchMenuLocation == SEARCH_MODE)
      {
        m_searchMenuLocation = SEARCH_TYPE;
        m_selectedEntry = m_searchType == SEARCH_TYPE_NONE ? 0 : static_cast<u32>(m_searchType);
      }
    }

    if (kdown & KEY_R)
    {
      if (m_searchMenuLocation == SEARCH_TYPE)
      {
        m_searchMenuLocation = SEARCH_MODE;
        m_selectedEntry = m_searchMode == SEARCH_MODE_NONE ? 0 : static_cast<u32>(m_searchMode);
      }
      else if (m_searchMenuLocation == SEARCH_MODE)
      {
        m_searchMenuLocation = SEARCH_REGION;
        m_selectedEntry = m_searchRegion == SEARCH_REGION_NONE ? 0 : static_cast<u32>(m_searchRegion);
      }
      else if (m_searchMenuLocation == SEARCH_REGION)
      {
        m_searchMenuLocation = SEARCH_VALUE;
        m_selectedEntry = 0;
        m_searchValueIndex = 0;
      }
    }
  }
}

void GuiCheats::onTouch(touchPosition &touch)
{
}

void GuiCheats::onGesture(touchPosition startPosition, touchPosition endPosition, bool finish)
{
}
static bool _isAddressFrozen(uintptr_t address)
{
  DmntFrozenAddressEntry *es;
  u64 Cnt = 0;
  bool frozen = false;

  dmntchtGetFrozenAddressCount(&Cnt);

  if (Cnt != 0)
  {
    es = new DmntFrozenAddressEntry[Cnt];
    dmntchtGetFrozenAddresses(es, Cnt, 0, nullptr);

    for (u64 i = 0; i < Cnt; i++)
    {
      if (es[i].address == address)
      {
        frozen = true;
        break;
      }
    }
  }

  return frozen;
}

static std::string _getAddressDisplayString(u64 address, Debugger *debugger, searchType_t searchType)
{
  std::stringstream ss;

  searchValue_t searchValue;
  searchValue._u64 = debugger->peekMemory(address);
  // start mod for address content display
  u16 k = searchValue._u8;
  if (m_searchValueFormat == FORMAT_HEX)
  {
    switch (dataTypeSizes[searchType])
    {
    case 1:
      ss << "0x" << std::uppercase << std::hex << k;
      break;
    case 2:
      ss << "0x" << std::uppercase << std::hex << searchValue._u16;
      break;
    default:
    case 4:
      ss << "0x" << std::uppercase << std::hex << searchValue._u32;
      break;
    case 8:
      ss << "0x" << std::uppercase << std::hex << searchValue._u64;
      break;
    }
  }
  else
  {

    // end mod
    switch (searchType)
    {
    case SEARCH_TYPE_UNSIGNED_8BIT:
      ss << std::dec << static_cast<u64>(searchValue._u8);
      break;
    case SEARCH_TYPE_UNSIGNED_16BIT:
      ss << std::dec << static_cast<u64>(searchValue._u16);
      break;
    case SEARCH_TYPE_UNSIGNED_32BIT:
      ss << std::dec << static_cast<u64>(searchValue._u32);
      break;
    case SEARCH_TYPE_UNSIGNED_64BIT:
      ss << std::dec << static_cast<u64>(searchValue._u64);
      break;
    case SEARCH_TYPE_SIGNED_8BIT:
      ss << std::dec << static_cast<s64>(searchValue._s8);
      break;
    case SEARCH_TYPE_SIGNED_16BIT:
      ss << std::dec << static_cast<s64>(searchValue._s16);
      break;
    case SEARCH_TYPE_SIGNED_32BIT:
      ss << std::dec << static_cast<s64>(searchValue._s32);
      break;
    case SEARCH_TYPE_SIGNED_64BIT:
      ss << std::dec << static_cast<s64>(searchValue._s64);
      break;
    case SEARCH_TYPE_FLOAT_32BIT:
      ss << std::dec << searchValue._f32;
      break;
    case SEARCH_TYPE_FLOAT_64BIT:
      ss << std::dec << searchValue._f64;
      break;
    case SEARCH_TYPE_POINTER:
      ss << std::dec << searchValue._u64;
      break;
    case SEARCH_TYPE_NONE:
      break;
    }
  }

  return ss.str();
}
static searchValue_t _get_entry(searchValue_t value, searchType_t type)
{
  searchValue_t result={0};
  char input[19];
  char initialString[21];
  strcpy(initialString, _getValueDisplayString(value, type).c_str());
  if (Gui::requestKeyboardInput("Enter value", "Enter a value that should get written at this .", initialString, m_searchValueFormat == FORMAT_DEC ? SwkbdType_NumPad : SwkbdType_QWERTY, input, 18))
  {
    if (m_searchValueFormat == FORMAT_HEX)
    {
      result._u64 = static_cast<u64>(std::stoul(input, nullptr, 16));
    }
    else if (type == SEARCH_TYPE_FLOAT_32BIT)
    {
      result._f32 = static_cast<float>(std::atof(input));
    }
    else if (type == SEARCH_TYPE_FLOAT_64BIT)
    {
      result._f64 = std::atof(input);
    }
    else if (type != SEARCH_TYPE_NONE)
    {
      result._u64 = std::atol(input);
    }
    else
    {
      result = value;
    };
  }
  else
  {
    result = value;
  }
  return result;
} 
static std::string _getValueDisplayString(searchValue_t searchValue, searchType_t searchType)
{
  std::stringstream ss;

  if (m_searchValueFormat == FORMAT_HEX)
  {
    switch (dataTypeSizes[searchType])
    {
    case 1:
      ss << "0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(2) << (searchValue._u16 & 0x00FF);
      break;
    case 2:
      ss << "0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(4) << searchValue._u16;
      break;
    default:
    case 4:
      ss << "0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(8) << searchValue._u32;
      break;
    case 8:
      ss << "0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(16) << searchValue._u64;
      break;
    }
  }
  else
  {
    switch (searchType)
    {
    case SEARCH_TYPE_UNSIGNED_8BIT:
      ss << std::dec << static_cast<u64>(searchValue._u8);
      break;
    case SEARCH_TYPE_UNSIGNED_16BIT:
      ss << std::dec << static_cast<u64>(searchValue._u16);
      break;
    case SEARCH_TYPE_UNSIGNED_32BIT:
      ss << std::dec << static_cast<u64>(searchValue._u32);
      break;
    case SEARCH_TYPE_UNSIGNED_64BIT:
      ss << std::dec << static_cast<u64>(searchValue._u64);
      break;
    case SEARCH_TYPE_SIGNED_8BIT:
      ss << std::dec << static_cast<s64>(searchValue._s8);
      break;
    case SEARCH_TYPE_SIGNED_16BIT:
      ss << std::dec << static_cast<s64>(searchValue._s16);
      break;
    case SEARCH_TYPE_SIGNED_32BIT:
      ss << std::dec << static_cast<s64>(searchValue._s32);
      break;
    case SEARCH_TYPE_SIGNED_64BIT:
      ss << std::dec << static_cast<s64>(searchValue._s64);
      break;
    case SEARCH_TYPE_FLOAT_32BIT:
      // ss.precision(15);
      ss << std::dec << searchValue._f32;
      break;
    case SEARCH_TYPE_FLOAT_64BIT:
      // ss.precision(15);
      ss << std::dec << searchValue._f64;
      break;
    case SEARCH_TYPE_POINTER:
      ss << std::dec << searchValue._u64;
      break;
    case SEARCH_TYPE_NONE:
      break;
    }
  }

  return ss.str();
}
// read
void GuiCheats::searchMemoryAddressesPrimary(Debugger *debugger, searchValue_t searchValue1, searchValue_t searchValue2, searchType_t searchType, searchMode_t searchMode, searchRegion_t searchRegion, MemoryDump **displayDump, std::vector<MemoryInfo> memInfos)
{
  (*displayDump) = new MemoryDump(EDIZON_DIR "/memdump1.dat", DumpType::ADDR, true);
  (*displayDump)->setBaseAddresses(m_addressSpaceBaseAddr, m_heapBaseAddr, m_mainBaseAddr, m_heapSize, m_mainSize);
  m_use_range = (searchMode == SEARCH_MODE_RANGE);
  (*displayDump)->setSearchParams(searchType, searchMode, searchRegion, searchValue1, searchValue2, m_use_range);

  MemoryDump *helperDump = new MemoryDump(EDIZON_DIR "/memdump1a.dat", DumpType::HELPER, true); // has address, size, count for fetching buffer from memory
  MemoryDump *newdataDump = new MemoryDump(EDIZON_DIR "/datadump2.dat", DumpType::DATA, true);
  MemoryDump *newstringDump = new MemoryDump(EDIZON_DIR "/stringdump.csv", DumpType::DATA, true); // to del when not needed

  helperinfo_t helperinfo;
  helperinfo.count = 0;

  bool ledOn = false;

  time_t unixTime1 = time(NULL);
  printf("%s%lx\n", "Start Time primary search", unixTime1);
  // printf("main %lx main end %lx heap %lx heap end %lx \n",m_mainBaseAddr, m_mainBaseAddr+m_mainSize, m_heapBaseAddr, m_heapBaseAddr+m_heapSize);
  printf("value1=%lx value2=%lx typesize=%d\n", searchValue1._u64, searchValue2._u64, dataTypeSizes[searchType]);
  for (MemoryInfo meminfo : memInfos)
  {

    if (searchRegion == SEARCH_REGION_HEAP && meminfo.type != MemType_Heap)
      continue;
    else if (searchRegion == SEARCH_REGION_MAIN &&
             (((meminfo.type != MemType_CodeWritable && meminfo.type != MemType_CodeMutable && meminfo.type != MemType_CodeStatic) || !(meminfo.addr < m_mainend && meminfo.addr >= m_mainBaseAddr))))
      continue;
    else if (searchRegion == SEARCH_REGION_HEAP_AND_MAIN &&
             (((meminfo.type != MemType_Heap && meminfo.type != MemType_CodeWritable && meminfo.type != MemType_CodeMutable)) || !((meminfo.addr < m_heapEnd && meminfo.addr >= m_heapBaseAddr) || (meminfo.addr < m_mainend && meminfo.addr >= m_mainBaseAddr))))
      continue;
    else if ( (meminfo.perm & Perm_Rw) != Perm_Rw) //searchRegion == SEARCH_REGION_RAM &&
      continue;

    // printf("%s%p", "meminfo.addr, ", meminfo.addr);
    // printf("%s%p", ", meminfo.end, ", meminfo.addr + meminfo.size);
    // printf("%s%p", ", meminfo.size, ", meminfo.size);
    // printf("%s%lx", ", meminfo.type, ", meminfo.type);
    // printf("%s%lx", ", meminfo.attr, ", meminfo.attr);
    // printf("%s%lx", ", meminfo.perm, ", meminfo.perm);
    // printf("%s%lx", ", meminfo.device_refcount, ", meminfo.device_refcount);
    // printf("%s%lx\n", ", meminfo.ipc_refcount, ", meminfo.ipc_refcount);
    setLedState(ledOn);
    ledOn = !ledOn;

    u64 offset = 0;
    u64 bufferSize = MAX_BUFFER_SIZE; // consider to increase from 10k to 1M (not a big problem)
    u8 *buffer = new u8[bufferSize];
    while (offset < meminfo.size)
    {

      if (meminfo.size - offset < bufferSize)
        bufferSize = meminfo.size - offset;

      debugger->readMemory(buffer, bufferSize, meminfo.addr + offset);

      searchValue_t realValue = {0};
      u32 inc_i;
      if (searchMode == SEARCH_MODE_POINTER)
        inc_i = 4;
      else
        inc_i = dataTypeSizes[searchType];

      for (u32 i = 0; i < bufferSize; i += inc_i)
      {
        u64 address = meminfo.addr + offset + i;
        memset(&realValue, 0, 8);
        if (searchMode == SEARCH_MODE_POINTER && m_32bitmode)
          memcpy(&realValue, buffer + i, 4);
        else
          memcpy(&realValue, buffer + i, dataTypeSizes[searchType]);

        if (Config::getConfig()->exclude_ptr_candidates && searchMode != SEARCH_MODE_POINTER)
        {
          searchValue_t ptr_address;
          memcpy(&ptr_address, buffer + i - i % 8, 8);
          if (((ptr_address._u64 >= m_mainBaseAddr) && (ptr_address._u64 <= (m_mainend))) || ((ptr_address._u64 >= m_heapBaseAddr) && (ptr_address._u64 <= (m_heapEnd))))
            continue;
        }
        // if (_check_extra_not_OK(buffer, i)) continue; // if not match let's continue
        switch (searchMode)
        {
        case SEARCH_MODE_EQ:
          if (realValue._s64 == searchValue1._s64)
          {
            // if (Config::getConfig()->extra_value)
            // {
            //   if (!true) // extra value match
            //     break;
            // }
            (*displayDump)->addData((u8 *)&address, sizeof(u64));
            helperinfo.count++;
          }
          break;
        case SEARCH_MODE_NEQ:
          if (realValue._s64 != searchValue1._s64)
          {
            (*displayDump)->addData((u8 *)&address, sizeof(u64));
            helperinfo.count++;
          }
          break;
        case SEARCH_MODE_GT:
          if (searchType & (SEARCH_TYPE_SIGNED_8BIT | SEARCH_TYPE_SIGNED_16BIT | SEARCH_TYPE_SIGNED_32BIT | SEARCH_TYPE_SIGNED_64BIT | SEARCH_TYPE_FLOAT_32BIT | SEARCH_TYPE_FLOAT_64BIT))
          {
            if (realValue._s64 > searchValue1._s64)
            {
              (*displayDump)->addData((u8 *)&address, sizeof(u64));
              helperinfo.count++;
            }
          }
          else
          {
            if (realValue._u64 > searchValue1._u64)
            {
              (*displayDump)->addData((u8 *)&address, sizeof(u64));
              helperinfo.count++;
            }
          }
          break;
        case SEARCH_MODE_DIFFA:
          if (searchType & (SEARCH_TYPE_SIGNED_8BIT | SEARCH_TYPE_SIGNED_16BIT | SEARCH_TYPE_SIGNED_32BIT | SEARCH_TYPE_SIGNED_64BIT | SEARCH_TYPE_FLOAT_32BIT | SEARCH_TYPE_FLOAT_64BIT))
          {
            if (realValue._s64 >= searchValue1._s64)
            {
              (*displayDump)->addData((u8 *)&address, sizeof(u64));
              helperinfo.count++;
            }
          }
          else
          {
            if (realValue._u64 >= searchValue1._u64)
            {
              (*displayDump)->addData((u8 *)&address, sizeof(u64));
              helperinfo.count++;
            }
          }
          break;
        case SEARCH_MODE_LT:
          if (searchType & (SEARCH_TYPE_SIGNED_8BIT | SEARCH_TYPE_SIGNED_16BIT | SEARCH_TYPE_SIGNED_32BIT | SEARCH_TYPE_SIGNED_64BIT | SEARCH_TYPE_FLOAT_32BIT | SEARCH_TYPE_FLOAT_64BIT))
          {
            if (realValue._s64 < searchValue1._s64)
            {
              (*displayDump)->addData((u8 *)&address, sizeof(u64));
              helperinfo.count++;
            }
          }
          else
          {
            if (realValue._u64 < searchValue1._u64)
            {
              (*displayDump)->addData((u8 *)&address, sizeof(u64));
              helperinfo.count++;
            }
          }
          break;
        case SEARCH_MODE_SAMEA:
          if (searchType & (SEARCH_TYPE_SIGNED_8BIT | SEARCH_TYPE_SIGNED_16BIT | SEARCH_TYPE_SIGNED_32BIT | SEARCH_TYPE_SIGNED_64BIT | SEARCH_TYPE_FLOAT_32BIT | SEARCH_TYPE_FLOAT_64BIT))
          {
            if (realValue._s64 <= searchValue1._s64)
            {
              (*displayDump)->addData((u8 *)&address, sizeof(u64));
              helperinfo.count++;
            }
          }
          else
          {
            if (realValue._u64 <= searchValue1._u64)
            {
              (*displayDump)->addData((u8 *)&address, sizeof(u64));
              helperinfo.count++;
            }
          }
          break;
        case SEARCH_MODE_RANGE:
          if (realValue._s64 >= searchValue1._s64 && realValue._s64 <= searchValue2._s64)
          {
            (*displayDump)->addData((u8 *)&address, sizeof(u64));
            newdataDump->addData((u8 *)&realValue, sizeof(u64));
            helperinfo.count++;
          }
          break;
        case SEARCH_MODE_POINTER: //m_heapBaseAddr, m_mainBaseAddr, m_heapSize, m_mainSize
          if ((realValue._u64 != 0))
            if (((realValue._u64 >= m_mainBaseAddr) && (realValue._u64 <= (m_mainend))) || ((realValue._u64 >= m_heapBaseAddr) && (realValue._u64 <= (m_heapEnd))))
            // if ((realValue._u64 >= m_heapBaseAddr) && (realValue._u64 <= m_heapEnd))
            {
              if ((m_forwarddump) && (address > realValue._u64) && (meminfo.type == MemType_Heap))
                break;
              (*displayDump)->addData((u8 *)&address, sizeof(u64));
              newdataDump->addData((u8 *)&realValue, sizeof(u64));
              helperinfo.count++;
              // printf("%lx,%lx\n",address,realValue);
              // std::stringstream ss; // replace the printf
              // ss.str("");
              // ss << "0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << address;
              // ss << ",0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << realValue._u64;
              // char st[27];
              // snprintf(st, 27, "%s\n", ss.str().c_str());    //
              // newstringDump->addData((u8 *)&st, sizeof(st)); //
            }
          break;
        case SEARCH_MODE_NONE:
        case SEARCH_MODE_SAME:
        case SEARCH_MODE_DIFF:
        case SEARCH_MODE_INC:
        case SEARCH_MODE_DEC:
        case SEARCH_MODE_NOT_POINTER:
          printf("search mode non !");
          break;
        }
      }

      // helper info write must be before inc
      if (helperinfo.count != 0)
      {
        helperinfo.address = meminfo.addr + offset;
        helperinfo.size = bufferSize;
        helperDump->addData((u8 *)&helperinfo, sizeof(helperinfo));
        // printf("address 0x%lx ", helperinfo.address);
        // printf("size %ld ", helperinfo.size);
        // printf("count %ld type %d \n ", helperinfo.count, meminfo.type);
        helperinfo.count = 0;
      } // must be after write

      offset += bufferSize;
    }

    delete[] buffer;
  }

  setLedState(false);

  time_t unixTime2 = time(NULL);
  printf("%s%lx\n", "Stop Time ", unixTime2);
  printf("%s%ld\n", "Stop Time ", unixTime2 - unixTime1);

  (*displayDump)->flushBuffer();
  newdataDump->flushBuffer();
  helperDump->flushBuffer();
  delete helperDump;
  delete newdataDump;
  newstringDump->flushBuffer(); // temp
  delete newstringDump;         //
}
//

void GuiCheats::searchMemoryAddressesSecondary(Debugger *debugger, searchValue_t searchValue1, searchValue_t searchValue2, searchType_t searchType, searchMode_t searchMode, bool use_range, MemoryDump **displayDump)
{
  MemoryDump *newDump = new MemoryDump(EDIZON_DIR "/memdump2.dat", DumpType::ADDR, true);
  bool ledOn = false;
  //begin
  time_t unixTime1 = time(NULL);
  printf("%s%lx\n", "Start Time Secondary search", unixTime1);
  if (searchMode == SEARCH_MODE_RANGE)
  {
    m_use_range = true;
    use_range = true;
  };
  u64 offset = 0;
  u64 bufferSize = MAX_BUFFER_SIZE; // this is for file access going for 1M
  u8 *buffer = new u8[bufferSize];
  // helper init
  MemoryDump *helperDump = new MemoryDump(EDIZON_DIR "/memdump1a.dat", DumpType::HELPER, false);   // has address, size, count for fetching buffer from memory
  MemoryDump *newhelperDump = new MemoryDump(EDIZON_DIR "/memdump3a.dat", DumpType::HELPER, true); // has address, size, count for fetching buffer from memory
  MemoryDump *newdataDump = new MemoryDump(EDIZON_DIR "/datadump2.dat", DumpType::DATA, true);
  MemoryDump *debugdump1 = new MemoryDump(EDIZON_DIR "/debugdump1.dat", DumpType::HELPER, true);
  if (helperDump->size() == 0)
  {
    (new Snackbar("Helper file not found !"))->show();
    return;
  }
  else
  {
    // helper integrity check
    printf("start helper integrity check address secondary \n");
    u32 helpercount = 0;
    helperinfo_t helperinfo;
    for (u64 i = 0; i < helperDump->size(); i += sizeof(helperinfo))
    {
      helperDump->getData(i, &helperinfo, sizeof(helperinfo));
      helpercount += helperinfo.count;
    }
    if (helpercount != (*displayDump)->size() / sizeof(u64))
    {
      printf("Integrity problem with helper file helpercount = %d  memdumpsize = %ld \n", helpercount, (*displayDump)->size() / sizeof(u64));
      (new Snackbar("Helper integrity check failed !"))->show();
      return;
    }
    printf("end helper integrity check address secondary \n");
    // end helper integrity check

    std::stringstream Message;
    Message << "Traversing title memory.\n \nThis may take a while... secondary search\nTime " << (unixTime1 - time(NULL)) << "    total " << (*displayDump)->size();
    (new MessageBox(Message.str(), MessageBox::NONE))->show();
    requestDraw();
  }

  u8 *ram_buffer = new u8[bufferSize];
  u64 helper_offset = 0;
  helperinfo_t helperinfo;
  helperinfo_t newhelperinfo;
  newhelperinfo.count = 0;

  helperDump->getData(helper_offset, &helperinfo, sizeof(helperinfo)); // helper_offset+=sizeof(helperinfo)
  debugger->readMemory(ram_buffer, helperinfo.size, helperinfo.address);
  // helper init end
  while (offset < (*displayDump)->size())
  {
    if ((*displayDump)->size() - offset < bufferSize)
      bufferSize = (*displayDump)->size() - offset;
    (*displayDump)->getData(offset, buffer, bufferSize); // BM4

    for (u64 i = 0; i < bufferSize; i += sizeof(u64)) // for (size_t i = 0; i < (bufferSize / sizeof(u64)); i++)
    {
      if (helperinfo.count == 0)
      {
        if (newhelperinfo.count != 0)
        {
          newhelperinfo.address = helperinfo.address;
          newhelperinfo.size = helperinfo.size;
          newhelperDump->addData((u8 *)&newhelperinfo, sizeof(newhelperinfo));
          // printf("%s%lx\n", "newhelperinfo.address ", newhelperinfo.address);
          // printf("%s%lx\n", "newhelperinfo.size ", newhelperinfo.size);
          // printf("%s%lx\n", "newhelperinfo.count ", newhelperinfo.count);
          newhelperinfo.count = 0;
        }
        helper_offset += sizeof(helperinfo);
        helperDump->getData(helper_offset, &helperinfo, sizeof(helperinfo));
        debugger->readMemory(ram_buffer, helperinfo.size, helperinfo.address);
      }
      searchValue_t value = {0};
      // searchValue_t testing = {0}; // temp
      u64 address = 0;

      address = *reinterpret_cast<u64 *>(&buffer[i]); //(*displayDump)->getData(i * sizeof(u64), &address, sizeof(u64));

      memcpy(&value, ram_buffer + address - helperinfo.address, dataTypeSizes[searchType]); // extrat from buffer instead of making call
      helperinfo.count--;                                                                   // each fetch dec
      // testing = value;                                                                      // temp
      // debugger->readMemory(&value, dataTypeSizes[searchType], address);
      // if (testing._u64 != value._u64)
      // {
      //   printf("%s%lx\n", "helperinfo.address ", helperinfo.address);
      //   printf("%s%lx\n", "helperinfo.size ", helperinfo.size);
      //   printf("%s%lx\n", "helperinfo.count ", helperinfo.count);
      //   printf("%s%lx\n", "address ", address);
      //   printf("%s%lx\n", "testing._u64 ", testing._u64);
      //   printf("%s%lx\n", "value ", value);
      //   printf("%s%lx\n", " address - helperinfo.address ", address - helperinfo.address);
      //   printf("%s%lx\n", " * (ram_buffer + address - helperinfo.address) ", *(ram_buffer + address - helperinfo.address));
      //   printf("%s%lx\n", " * (&ram_buffer[ address - helperinfo.address]) ", *(&ram_buffer[address - helperinfo.address]));
      //   printf("%s%lx\n", "  (ram_buffer + address - helperinfo.address) ", (ram_buffer + address - helperinfo.address));
      //   printf("%s%lx\n", "  (&ram_buffer[ address - helperinfo.address]) ", (&ram_buffer[address - helperinfo.address]));
      //   printf("%s%lx\n", "  helperinfo.size - address + helperinfo.address ", helperinfo.size - address + helperinfo.address);
      //   // debugdump1->addData((u8 *)&ram_buffer, helperinfo.size);
      //   // debugger->readMemory(ram_buffer, 0x50, address);
      //   // debugdump2->addData((u8 *)&ram_buffer, 0x50);
      //   //
      //   // delete debugdump2;
      // }

      if (i % 50000 == 0)
      {
        setLedState(ledOn);
        ledOn = !ledOn;
      }

      switch (searchMode)
      {
      case SEARCH_MODE_EQ:
        if (value._s64 == searchValue1._s64)
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_NEQ:
        if (value._s64 != searchValue1._s64)
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_GT:
        if (value._s64 > searchValue1._s64)
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_DIFFA:
        if (value._s64 >= searchValue1._s64)
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_LT:
        if (value._s64 < searchValue1._s64)
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_SAMEA:
        if (value._s64 <= searchValue1._s64)
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_RANGE:
        if (value._s64 >= searchValue1._s64 && value._s64 <= searchValue2._s64)
        {
          newDump->addData((u8 *)&address, sizeof(u64)); // add here
          newdataDump->addData((u8 *)&value, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_POINTER: //m_heapBaseAddr, m_mainBaseAddr, m_heapSize, m_mainSize
        if ((value._u64 >= m_mainBaseAddr && value._u64 <= (m_mainend)) || (value._u64 >= m_heapBaseAddr && value._u64 <= (m_heapEnd)))
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newdataDump->addData((u8 *)&value, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_SAME:
        if (value._s64 == searchValue1._s64)
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_DIFF:
        if (value._s64 != searchValue1._s64)
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_INC:
        if (value._s64 > searchValue1._s64)
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_DEC:
        if (value._s64 < searchValue1._s64)
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_NONE:
      case SEARCH_MODE_NOT_POINTER:
        break;
      }
    }
    printf("%ld of %ld done \n", offset, (*displayDump)->size()); // maybe consider message box this info
    offset += bufferSize;
  }

  if (newhelperinfo.count != 0) // take care of the last one
  {
    newhelperinfo.address = helperinfo.address;
    newhelperinfo.size = helperinfo.size;
    newhelperDump->addData((u8 *)&newhelperinfo, sizeof(newhelperinfo));
    // printf("%s%lx\n", "newhelperinfo.address ", newhelperinfo.address);
    // printf("%s%lx\n", "newhelperinfo.size ", newhelperinfo.size);
    // printf("%s%lx\n", "newhelperinfo.count ", newhelperinfo.count);
    newhelperinfo.count = 0;
  }
  //end
  newDump->flushBuffer();
  newhelperDump->flushBuffer();
  newdataDump->flushBuffer();

  if (newDump->size() > 0)
  {
    // delete m_memoryDump;
    // remove(EDIZON_DIR "/memdump1.dat");
    // rename(EDIZON_DIR "/memdump2.dat", EDIZON_DIR "/memdump2.dat");
    (*displayDump)->clear();
    (*displayDump)->setSearchParams(searchType, searchMode, (*displayDump)->getDumpInfo().searchRegion, searchValue1, searchValue2, use_range);
    (*displayDump)->setDumpType(DumpType::ADDR);

    // begin copy
    offset = 0;
    bufferSize = MAX_BUFFER_SIZE;                 //0x1000000; // match what was created before
    printf("%s%lx\n", "bufferSize ", bufferSize); // printf
    while (offset < newDump->size())
    {
      if (newDump->size() - offset < bufferSize)
        bufferSize = newDump->size() - offset;
      newDump->getData(offset, buffer, bufferSize);
      (*displayDump)->addData(buffer, bufferSize);
      offset += bufferSize;
    }
    // end copy

    (*displayDump)->flushBuffer();
  }
  else
  {
    (new Snackbar("None of values changed to the entered one!"))->show();
    m_nothingchanged = true;
  }

  setLedState(false);
  delete newDump;
  delete newhelperDump;
  delete helperDump;
  delete debugdump1;
  delete newdataDump;
  delete[] buffer;
  delete[] ram_buffer;

  remove(EDIZON_DIR "/memdump2.dat");
  time_t unixTime2 = time(NULL);
  printf("%s%lx\n", "Stop Time secondary search ", unixTime2);
  printf("%s%ld\n", "Stop Time ", unixTime2 - unixTime1);
}

void GuiCheats::searchMemoryAddressesSecondary2(Debugger *debugger, searchValue_t searchValue1, searchValue_t searchValue2, searchType_t searchType, searchMode_t searchMode, MemoryDump **displayDump)
{
  MemoryDump *lastdataDump = new MemoryDump(EDIZON_DIR "/datadump2.dat", DumpType::DATA, false);
  if (lastdataDump->size() == 0)
  {
    (new Snackbar("No previous value found !"))->show();
    m_nothingchanged = true;
    delete lastdataDump;
    return;
  }
  else
  {
    delete lastdataDump;
  }
  MemoryDump *newDump = new MemoryDump(EDIZON_DIR "/memdump2.dat", DumpType::ADDR, true);
  bool ledOn = false;
  //begin
  time_t unixTime1 = time(NULL);
  printf("%s%lx\n", "Start Time Secondary search", unixTime1);
  if (searchMode == SEARCH_MODE_RANGE)
    m_use_range = true;
  u64 offset = 0;
  u64 bufferSize = MAX_BUFFER_SIZE; // this is for file access going for 1M
  u8 *buffer = new u8[bufferSize];
  u8 *predatabuffer = new u8[bufferSize];
  searchValue_t prevalue = {0};
  // helper init
  MemoryDump *helperDump = new MemoryDump(EDIZON_DIR "/memdump1a.dat", DumpType::HELPER, false);   // has address, size, count for fetching buffer from memory
  MemoryDump *newhelperDump = new MemoryDump(EDIZON_DIR "/memdump3a.dat", DumpType::HELPER, true); // has address, size, count for fetching buffer from memory
  std::string s = m_edizon_dir + "/datadump2.dat" + m_store_extension;
  REPLACEFILE(s.c_str(), EDIZON_DIR "/predatadump2.dat");
  MemoryDump *predataDump = new MemoryDump(EDIZON_DIR "/predatadump2.dat", DumpType::DATA, false);
  MemoryDump *newdataDump = new MemoryDump(EDIZON_DIR "/datadump2.dat", DumpType::DATA, true);
  // MemoryDump *debugdump1 = new MemoryDump(EDIZON_DIR "/debugdump1.dat", DumpType::HELPER, true);
  if (helperDump->size() == 0)
  {
    (new Snackbar("Helper file not found !"))->show();
    return;
  }
  else
  {
    // helper integrity check
    printf("start helper integrity check address secondary \n");
    u32 helpercount = 0;
    helperinfo_t helperinfo;
    for (u64 i = 0; i < helperDump->size(); i += sizeof(helperinfo))
    {
      helperDump->getData(i, &helperinfo, sizeof(helperinfo));
      helpercount += helperinfo.count;
    }
    if (helpercount != (*displayDump)->size() / sizeof(u64))
    {
      printf("Integrity problem with helper file helpercount = %d  memdumpsize = %ld \n", helpercount, (*displayDump)->size() / sizeof(u64));
      (new Snackbar("Helper integrity check failed !"))->show();
      return;
    }
    printf("end helper integrity check address secondary \n");
    // end helper integrity check

    std::stringstream Message;
    Message << "Traversing title memory.\n \nThis may take a while... secondary search\nTime " << (unixTime1 - time(NULL)) << "    total " << (*displayDump)->size();
    (new MessageBox(Message.str(), MessageBox::NONE))->show();
    requestDraw();
  }

  u8 *ram_buffer = new u8[bufferSize];
  u64 helper_offset = 0;
  helperinfo_t helperinfo;
  helperinfo_t newhelperinfo;
  newhelperinfo.count = 0;

  helperDump->getData(helper_offset, &helperinfo, sizeof(helperinfo)); // helper_offset+=sizeof(helperinfo)
  debugger->readMemory(ram_buffer, helperinfo.size, helperinfo.address);
  // helper init end
  while (offset < (*displayDump)->size())
  {
    if ((*displayDump)->size() - offset < bufferSize)
      bufferSize = (*displayDump)->size() - offset;
    (*displayDump)->getData(offset, buffer, bufferSize); // BM4
    predataDump->getData(offset, predatabuffer, bufferSize);

    for (u64 i = 0; i < bufferSize; i += sizeof(u64)) // for (size_t i = 0; i < (bufferSize / sizeof(u64)); i++)
    {
      if (helperinfo.count == 0)
      {
        if (newhelperinfo.count != 0)
        {
          newhelperinfo.address = helperinfo.address;
          newhelperinfo.size = helperinfo.size;
          newhelperDump->addData((u8 *)&newhelperinfo, sizeof(newhelperinfo));
          // printf("%s%lx\n", "newhelperinfo.address ", newhelperinfo.address);
          // printf("%s%lx\n", "newhelperinfo.size ", newhelperinfo.size);
          // printf("%s%lx\n", "newhelperinfo.count ", newhelperinfo.count);
          newhelperinfo.count = 0;
        }
        helper_offset += sizeof(helperinfo);
        helperDump->getData(helper_offset, &helperinfo, sizeof(helperinfo));
        debugger->readMemory(ram_buffer, helperinfo.size, helperinfo.address);
      }
      searchValue_t value = {0};
      // searchValue_t testing = {0}; // temp
      u64 address = 0;

      address = *reinterpret_cast<u64 *>(&buffer[i]); //(*displayDump)->getData(i * sizeof(u64), &address, sizeof(u64));
      prevalue._u64 = *reinterpret_cast<u64 *>(&predatabuffer[i]);

      memcpy(&value, ram_buffer + address - helperinfo.address, dataTypeSizes[searchType]); // extrat from buffer instead of making call
      helperinfo.count--;                                                                   // each fetch dec
      // testing = value;                                                                      // temp
      // debugger->readMemory(&value, dataTypeSizes[searchType], address);
      // if (testing._u64 != value._u64)
      // {
      //   printf("%s%lx\n", "helperinfo.address ", helperinfo.address);
      //   printf("%s%lx\n", "helperinfo.size ", helperinfo.size);
      //   printf("%s%lx\n", "helperinfo.count ", helperinfo.count);
      //   printf("%s%lx\n", "address ", address);
      //   printf("%s%lx\n", "testing._u64 ", testing._u64);
      //   printf("%s%lx\n", "value ", value);
      //   printf("%s%lx\n", " address - helperinfo.address ", address - helperinfo.address);
      //   printf("%s%lx\n", " * (ram_buffer + address - helperinfo.address) ", *(ram_buffer + address - helperinfo.address));
      //   printf("%s%lx\n", " * (&ram_buffer[ address - helperinfo.address]) ", *(&ram_buffer[address - helperinfo.address]));
      //   printf("%s%lx\n", "  (ram_buffer + address - helperinfo.address) ", (ram_buffer + address - helperinfo.address));
      //   printf("%s%lx\n", "  (&ram_buffer[ address - helperinfo.address]) ", (&ram_buffer[address - helperinfo.address]));
      //   printf("%s%lx\n", "  helperinfo.size - address + helperinfo.address ", helperinfo.size - address + helperinfo.address);
      //   // debugdump1->addData((u8 *)&ram_buffer, helperinfo.size);
      //   // debugger->readMemory(ram_buffer, 0x50, address);
      //   // debugdump2->addData((u8 *)&ram_buffer, 0x50);
      //   //
      //   // delete debugdump2;
      // }

      if (i % 50000 == 0)
      {
        setLedState(ledOn);
        ledOn = !ledOn;
      }

      switch (searchMode)
      {
      case SEARCH_MODE_EQ:
        if (value._s64 == searchValue1._s64)
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_NEQ:
        if (value._s64 != searchValue1._s64)
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_GT:
        if (value._s64 > searchValue1._s64)
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_DIFFA: //need to rewrite
        if (value._s64 != prevalue._s64)
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newdataDump->addData((u8 *)&value, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_LT:
        if (value._s64 < searchValue1._s64)
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_SAMEA: // need to rewrite
        if (value._s64 == prevalue._s64)
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newdataDump->addData((u8 *)&value, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_RANGE:
        if (value._s64 >= searchValue1._s64 && value._s64 <= searchValue2._s64)
        {
          newDump->addData((u8 *)&address, sizeof(u64)); // add here
          newdataDump->addData((u8 *)&value, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_POINTER: //m_heapBaseAddr, m_mainBaseAddr, m_heapSize, m_mainSize
        if ((value._u64 >= m_mainBaseAddr && value._u64 <= (m_mainend)) || (value._u64 >= m_heapBaseAddr && value._u64 <= (m_heapEnd)))
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newdataDump->addData((u8 *)&value, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_SAME:
        if ((value._s64 == prevalue._s64) && ((value._s64 >= searchValue1._s64 && value._s64 <= searchValue2._s64) || !m_use_range))
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newdataDump->addData((u8 *)&value, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_DIFF:
        if ((value._s64 != prevalue._s64) && ((value._s64 >= searchValue1._s64 && value._s64 <= searchValue2._s64) || !m_use_range))
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newdataDump->addData((u8 *)&value, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_INC:
        if ((value._s64 > prevalue._s64)&& ((value._s64 >= searchValue1._s64 && value._s64 <= searchValue2._s64) || !m_use_range))
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newdataDump->addData((u8 *)&value, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_DEC:
        if ((value._s64 < prevalue._s64)&& ((value._s64 >= searchValue1._s64 && value._s64 <= searchValue2._s64) || !m_use_range))
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newdataDump->addData((u8 *)&value, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_NONE:
      case SEARCH_MODE_NOT_POINTER:
        break;
      }
    }
    printf("%ld of %ld done \n", offset, (*displayDump)->size()); // maybe consider message box this info
    offset += bufferSize;
  }

  if (newhelperinfo.count != 0) // take care of the last one
  {
    newhelperinfo.address = helperinfo.address;
    newhelperinfo.size = helperinfo.size;
    newhelperDump->addData((u8 *)&newhelperinfo, sizeof(newhelperinfo));
    // printf("%s%lx\n", "newhelperinfo.address ", newhelperinfo.address);
    // printf("%s%lx\n", "newhelperinfo.size ", newhelperinfo.size);
    // printf("%s%lx\n", "newhelperinfo.count ", newhelperinfo.count);
    newhelperinfo.count = 0;
  }
  //end
  newDump->flushBuffer();
  newhelperDump->flushBuffer();
  newdataDump->flushBuffer();

  if (newDump->size() > 0)
  {
    // delete m_memoryDump;
    // remove(EDIZON_DIR "/memdump1.dat");
    // rename(EDIZON_DIR "/memdump2.dat", EDIZON_DIR "/memdump2.dat");
    (*displayDump)->clear();
    (*displayDump)->setSearchParams(searchType, searchMode, (*displayDump)->getDumpInfo().searchRegion, searchValue1, searchValue2, m_use_range);
    (*displayDump)->setDumpType(DumpType::ADDR);

    // begin copy
    offset = 0;
    bufferSize = MAX_BUFFER_SIZE;                 //0x1000000; // match what was created before
    printf("%s%lx\n", "bufferSize ", bufferSize); // printf
    while (offset < newDump->size())
    {
      if (newDump->size() - offset < bufferSize)
        bufferSize = newDump->size() - offset;
      newDump->getData(offset, buffer, bufferSize);
      (*displayDump)->addData(buffer, bufferSize);
      offset += bufferSize;
    }
    // end copy

    (*displayDump)->flushBuffer();
  }
  else
  {
    (new Snackbar("None of values changed to the entered one!"))->show();
    m_nothingchanged = true;
  }

  setLedState(false);
  delete newDump;
  delete newhelperDump;
  delete helperDump;
  // delete debugdump1;
  delete newdataDump;
  delete[] buffer;
  delete[] predatabuffer;
  delete[] ram_buffer;

  remove(EDIZON_DIR "/memdump2.dat");
  time_t unixTime2 = time(NULL);
  printf("%s%lx\n", "Stop Time secondary search ", unixTime2);
  printf("%s%ld\n", "Stop Time ", unixTime2 - unixTime1);
}

///////////////////////////////////////////////
// read
void GuiCheats::searchMemoryValuesPrimary(Debugger *debugger, searchType_t searchType, searchMode_t searchMode, searchRegion_t searchRegion, MemoryDump **displayDump, std::vector<MemoryInfo> memInfos)
{
  bool ledOn = false;

  // searchValue_t zeroValue;
  // zeroValue._u64 = 0;
  // printf("%s\n", "searchMemoryValuesPrimary");
  // printf("%s\n", titleNameStr.c_str());
  // printf("%s\n", tidStr.c_str());
  // printf("%s\n", buildIDStr.c_str());
  // printf("%s%lx\n", "m_addressSpaceBaseAddr ", m_addressSpaceBaseAddr);
  // printf("%s%lx\n", "m_heapBaseAddr ", m_heapBaseAddr);
  // printf("%s%lx\n", "m_mainBaseAddr ", m_mainBaseAddr);
  // printf("%s%lx\n", "m_heapSize ", m_heapSize);
  // printf("%s%lx\n", "m_mainSize ", m_mainSize);
  // printf("%s%X1\n", "searchType ", searchType);
  // printf("%s%X1\n", "searchMode ", searchMode);
  // printf("%s%X1\n", "searchRegion ", searchRegion);
  (new Snackbar("Dumping memory"))->show();
  (*displayDump) = new MemoryDump(EDIZON_DIR "/memdump1.dat", DumpType::DATA, true);
  (*displayDump)->setBaseAddresses(m_addressSpaceBaseAddr, m_heapBaseAddr, m_mainBaseAddr, m_heapSize, m_mainSize);
  (*displayDump)->setSearchParams(searchType, searchMode, searchRegion, {0}, {0}, false);
  // start time
  time_t unixTime1 = time(NULL);
  printf("%s%lx\n", "Start Time ", unixTime1);

  for (MemoryInfo meminfo : memInfos)
  {
    if (searchRegion == SEARCH_REGION_HEAP && meminfo.type != MemType_Heap)
      continue;
    else if (searchRegion == SEARCH_REGION_MAIN &&
             (meminfo.type != MemType_CodeWritable && meminfo.type != MemType_CodeMutable))
      continue;
    else if (searchRegion == SEARCH_REGION_HEAP_AND_MAIN &&
             (meminfo.type != MemType_Heap && meminfo.type != MemType_CodeWritable && meminfo.type != MemType_CodeMutable))
      continue;
    else if (searchRegion == SEARCH_REGION_RAM && (meminfo.perm & Perm_Rw) != Perm_Rw)
      continue;

    setLedState(ledOn);
    ledOn = !ledOn;
    // printf("%s%lx\n", "meminfo.size ", meminfo.size);
    // printf("%s%lx\n", "meminfo.addr ", meminfo.addr);
    // printf("%s%lx\n", "meminfo.type ", meminfo.type);
    // printf("%s%lx\n", "meminfo.perm ", meminfo.perm);

    u64 offset = 0;
    u64 bufferSize = MAX_BUFFER_SIZE; // hack increase from 40K to 1M
    u8 *buffer = new u8[bufferSize];
    while (offset < meminfo.size)
    {

      if (meminfo.size - offset < bufferSize)
        bufferSize = meminfo.size - offset;

      debugger->readMemory(buffer, bufferSize, meminfo.addr + offset);
      (*displayDump)->addData(buffer, bufferSize);

      offset += bufferSize;
    }

    delete[] buffer;
  }

  setLedState(false);
  (*displayDump)->flushBuffer();
  // end time
  time_t unixTime2 = time(NULL);
  printf("%s%lx\n", "Stop Time ", unixTime2);
  printf("%s%ld\n", "Stop Time ", unixTime2 - unixTime1);
}

////////////////////////////////////////////////////
void GuiCheats::searchMemoryValuesSecondary(Debugger *debugger, searchType_t searchType, searchMode_t searchMode, searchRegion_t searchRegion, MemoryDump **displayDump, std::vector<MemoryInfo> memInfos)
{
  bool ledOn = false;
  searchValue_t oldValue = {0}; // check if needed
  searchValue_t newValue = {0};

  MemoryDump *newMemDump = new MemoryDump(EDIZON_DIR "/datadump2.dat", DumpType::DATA, true); // Store Current value
  MemoryDump *addrDump = new MemoryDump(EDIZON_DIR "/memdump3.dat", DumpType::ADDR, true);
  addrDump->setBaseAddresses(m_addressSpaceBaseAddr, m_heapBaseAddr, m_mainBaseAddr, m_heapSize, m_mainSize);
  addrDump->setSearchParams(searchType, searchMode, searchRegion, {0}, {0}, false);

  // work in progress
  // if (searchMode == SEARCH_MODE_DIFFA)
  MemoryDump *valueDumpA = new MemoryDump(EDIZON_DIR "/datadumpA.dat", DumpType::DATA, true); // file to put A
  MemoryDump *valueDumpB = new MemoryDump(EDIZON_DIR "/datadumpB.dat", DumpType::DATA, true); // file to put B
  if ((searchMode == SEARCH_MODE_SAMEA) || (searchMode == SEARCH_MODE_DIFFA))
  {
    (new Snackbar("Creating state B"))->show();
  }
  // end work in progress

  u64 dumpoffset = 0; // file offset need to be for whole session

  // start time
  time_t unixTime1 = time(NULL);
  printf("%s%lx\n", "Start Time second search ", unixTime1);
  // helper init
  MemoryDump *helperDump = new MemoryDump(EDIZON_DIR "/memdump1a.dat", DumpType::ADDR, true); // has address, size, count for fetching buffer from memory
  helperinfo_t helperinfo;

  for (MemoryInfo meminfo : memInfos)
  {
    if (searchRegion == SEARCH_REGION_HEAP && meminfo.type != MemType_Heap)
      continue;
    else if (searchRegion == SEARCH_REGION_MAIN &&
             (meminfo.type != MemType_CodeWritable && meminfo.type != MemType_CodeMutable))
      continue;
    else if (searchRegion == SEARCH_REGION_HEAP_AND_MAIN &&
             (meminfo.type != MemType_Heap && meminfo.type != MemType_CodeWritable && meminfo.type != MemType_CodeMutable))
      continue;
    else if (searchRegion == SEARCH_REGION_RAM && (meminfo.perm & Perm_Rw) != Perm_Rw)
      continue;

    setLedState(ledOn);
    ledOn = !ledOn;
    printf("%s%lx\n", "meminfo.size ", meminfo.size);
    printf("%s%lx\n", "meminfo.addr ", meminfo.addr);
    printf("%s%x\n", "meminfo.type ", meminfo.type);
    printf("%s%x\n", "meminfo.perm ", meminfo.perm);
    u64 offset = 0;
    u64 bufferSize = MAX_BUFFER_SIZE; // hack increase from 40K to 1M
    u8 *buffer = new u8[bufferSize];
    u8 *filebuffer = new u8[bufferSize]; //added filebuffer matching memory buffer memory buffer could be smaller, let's see if that is too much
    u64 addr = meminfo.addr;

    // start count
    helperinfo.count = 0;

    while (offset < meminfo.size)
    {
      setLedState(ledOn);
      ledOn = !ledOn;

      if (meminfo.size - offset < bufferSize)
        bufferSize = meminfo.size - offset;

      debugger->readMemory(buffer, bufferSize, meminfo.addr + offset);
      // printf("%s\n", "readmemory OK ");
      // printf("%s%lx\n", "dumpoffset ", dumpoffset);
      // printf("%s%lx\n", "bufferSize ", bufferSize);
      // printf("%s%lx\n", "displayDump ", displayDump);
      // print_details = true;
      (*displayDump)->getData(dumpoffset, filebuffer, bufferSize);
      // print_details = false;
      // printf("%s\n", "readdata OK ");

      for (u64 i = 0; i < bufferSize; i += dataTypeSizes[searchType])
      {
        switch (dataTypeSizes[searchType])
        {
        case 1:
          newValue._u8 = *reinterpret_cast<u8 *>(&buffer[i]);
          oldValue._u8 = *reinterpret_cast<u8 *>(&filebuffer[i]);
        case 2:
          newValue._u16 = *reinterpret_cast<u16 *>(&buffer[i]);
          oldValue._u16 = *reinterpret_cast<u16 *>(&filebuffer[i]);
        case 4:
          newValue._u32 = *reinterpret_cast<u32 *>(&buffer[i]);
          oldValue._u32 = *reinterpret_cast<u32 *>(&filebuffer[i]);
        case 8:
          newValue._u64 = *reinterpret_cast<u64 *>(&buffer[i]);
          oldValue._u64 = *reinterpret_cast<u64 *>(&filebuffer[i]);
        }
        switch (searchMode)
        {
        case SEARCH_MODE_SAME:
          if (newValue._u64 == oldValue._u64)
          {
            addrDump->addData((u8 *)&addr, sizeof(u64));
            newMemDump->addData((u8 *)&newValue, sizeof(u64));
            helperinfo.count++;
          }
          break;
        case SEARCH_MODE_DIFF:
          if ((newValue._u64 != oldValue._u64) && (newValue._u64 <= m_heapBaseAddr || newValue._u64 >= (m_heapEnd)) )
          {
            addrDump->addData((u8 *)&addr, sizeof(u64));
            newMemDump->addData((u8 *)&newValue, sizeof(u64));
            helperinfo.count++;
          }
          break;
        case SEARCH_MODE_SAMEA:
        case SEARCH_MODE_DIFFA:
          if (newValue._u64 != oldValue._u64)
          {
            // (new Snackbar("Creating state A (previsou) and state B (current) "))->show();
            addrDump->addData((u8 *)&addr, sizeof(u64));
            newMemDump->addData((u8 *)&newValue, sizeof(u64)); // Keep compatibility with other mode
            valueDumpA->addData((u8 *)&oldValue, sizeof(u64)); // save state A
            valueDumpB->addData((u8 *)&newValue, sizeof(u64)); // save state B
            helperinfo.count++;
          }
          break;
        case SEARCH_MODE_INC:
          if (searchType & (SEARCH_TYPE_SIGNED_8BIT | SEARCH_TYPE_SIGNED_16BIT | SEARCH_TYPE_SIGNED_32BIT | SEARCH_TYPE_SIGNED_64BIT | SEARCH_TYPE_FLOAT_32BIT | SEARCH_TYPE_FLOAT_64BIT))
          {
            if (newValue._s64 > oldValue._s64)
            {
              addrDump->addData((u8 *)&addr, sizeof(u64));
              newMemDump->addData((u8 *)&newValue, sizeof(u64));
              helperinfo.count++;
            }
          }
          else
          {
            if (newValue._u64 > oldValue._u64)
            {
              addrDump->addData((u8 *)&addr, sizeof(u64));
              newMemDump->addData((u8 *)&newValue, sizeof(u64));
              helperinfo.count++;
            }
          }
          break;
        case SEARCH_MODE_DEC:
          if (searchType & (SEARCH_TYPE_SIGNED_8BIT | SEARCH_TYPE_SIGNED_16BIT | SEARCH_TYPE_SIGNED_32BIT | SEARCH_TYPE_SIGNED_64BIT | SEARCH_TYPE_FLOAT_32BIT | SEARCH_TYPE_FLOAT_64BIT))
          {
            if (newValue._s64 < oldValue._s64)
            {
              addrDump->addData((u8 *)&addr, sizeof(u64));
              newMemDump->addData((u8 *)&newValue, sizeof(u64));
              helperinfo.count++;
            }
          }
          else
          {
            if (newValue._u64 < oldValue._u64)
            {
              addrDump->addData((u8 *)&addr, sizeof(u64));
              newMemDump->addData((u8 *)&newValue, sizeof(u64));
              helperinfo.count++;
            }
          }
          break;
        case SEARCH_MODE_POINTER:
          if (((newValue._u64 >= m_mainBaseAddr) && (newValue._u64 <= (m_mainend))) || ((newValue._u64 >= m_heapBaseAddr) && (newValue._u64 <= (m_heapEnd))))
          {
            addrDump->addData((u8 *)&addr, sizeof(u64));
            newMemDump->addData((u8 *)&newValue, sizeof(u64));
            helperinfo.count++;
          }
          printf("error 321\n");
          break;
        case SEARCH_MODE_RANGE:
        case SEARCH_MODE_NONE:
        case SEARCH_MODE_NEQ:
        case SEARCH_MODE_EQ:
        case SEARCH_MODE_GT:
        case SEARCH_MODE_LT:
        case SEARCH_MODE_NOT_POINTER:
          printf("error 123\n");
          break;
        }
        addr += dataTypeSizes[searchType];
      }
      // end compare
      // helper info write must be before inc
      helperinfo.address = meminfo.addr + offset;
      helperinfo.size = bufferSize;
      if (helperinfo.count != 0)
        helperDump->addData((u8 *)&helperinfo, sizeof(helperinfo));
      printf("%s%lx\n", "helperinfo.address ", helperinfo.address);
      printf("%s%lx\n", "helperinfo.size ", helperinfo.size);
      printf("%s%lx\n", "helperinfo.count ", helperinfo.count);
      helperinfo.count = 0; // must be after write
                            // end

      offset += bufferSize;
      dumpoffset += bufferSize;

      printf("%s%lx\n", "offset ", offset);
      printf("%s%lx\n", "dumpoffset ", dumpoffset);
    }

    delete[] buffer;
    delete[] filebuffer;
  }
  newMemDump->flushBuffer();
  addrDump->flushBuffer();
  helperDump->flushBuffer();
  valueDumpA->flushBuffer();
  valueDumpB->flushBuffer();
  delete newMemDump; // this should close these two files
  delete addrDump;
  delete helperDump;
  delete valueDumpA;
  delete valueDumpB;

  // old maybe useless stuff to delete later
  // Bigger buffers
  // for (u64 addr = 0; addr < std::min((*displayDump)->size(), newMemDump->size()); addr += dataTypeSizes[searchType])
  // {
  //   searchValue_t oldValue = {0};
  //   searchValue_t newValue = {0};
  //   (*displayDump)->getData(addr, &oldValue, dataTypeSizes[searchType]);
  //   newMemDump->getData(addr, &newValue, dataTypeSizes[searchType]);
  // }

  setLedState(false);

  // end time second search
  time_t unixTime2 = time(NULL);
  printf("%s%lx\n", "Stop Time second search", unixTime2);
  printf("%s%ld\n", "Total Time in decimal seconds  ", unixTime2 - unixTime1);
}

void GuiCheats::searchMemoryValuesTertiary(Debugger *debugger, searchValue_t searchValue1, searchValue_t searchValue2, searchType_t searchType, searchMode_t searchMode, searchRegion_t searchRegion, bool use_range, MemoryDump **displayDump, std::vector<MemoryInfo> memInfos)
{
  MemoryDump *oldvalueDump = new MemoryDump(EDIZON_DIR "/datadump2.dat", DumpType::DATA, false); //file with previous value
  MemoryDump *newvalueDump = new MemoryDump(EDIZON_DIR "/datadump4.dat", DumpType::DATA, true);  // file to put new value

  //work in progress
  // if (searchMode == SEARCH_MODE_SAMEA)
  MemoryDump *valueDumpA = new MemoryDump(EDIZON_DIR "/datadumpA.dat", DumpType::DATA, false);    // file to get A
  MemoryDump *newvalueDumpA = new MemoryDump(EDIZON_DIR "/datadumpAa.dat", DumpType::DATA, true); // file to put new A
  MemoryDump *valueDumpB = new MemoryDump(EDIZON_DIR "/datadumpB.dat", DumpType::DATA, false);    // file to get B
  MemoryDump *newvalueDumpB = new MemoryDump(EDIZON_DIR "/datadumpBa.dat", DumpType::DATA, true); // file to put new B
  bool no_existing_dump = false;
  if ((searchMode == SEARCH_MODE_SAMEA) || (searchMode == SEARCH_MODE_DIFFA))
  {
    if (valueDumpA->size() == oldvalueDump->size())
    {
      delete oldvalueDump;
      oldvalueDump = valueDumpA;
    }
    else
    {
      // delete valueDumpB;
      // valueDumpB = oldvalueDump;
      no_existing_dump = true;
      printf("no existing dump \n");
      if (searchMode == SEARCH_MODE_SAMEA)
        return;
    }
  }

  // create a A and B file valueDumpA and newvalueDumpA ?? to keep track of A B;
  //end work in progress
  MemoryDump *newDump = new MemoryDump(EDIZON_DIR "/memdump3.dat", DumpType::ADDR, true); //file to put new candidates
  newDump->setBaseAddresses(m_addressSpaceBaseAddr, m_heapBaseAddr, m_mainBaseAddr, m_heapSize, m_mainSize);
  newDump->setSearchParams(searchType, searchMode, searchRegion, {0}, {0}, use_range);
  bool ledOn = false;
  //begin
  time_t unixTime1 = time(NULL);
  printf("%s%lx\n", "Start Time Tertiary search", unixTime1);

  u64 offset = 0;
  u64 bufferSize = MAX_BUFFER_SIZE; // this is for file access going for 1M
  bool have_oldvalue = false;
  u8 *buffer = new u8[bufferSize];
  u8 *valuebuffer = new u8[bufferSize];
  u8 *valueBbuffer = new u8[bufferSize];

  // BM7
  // helper init
  MemoryDump *helperDump = new MemoryDump(EDIZON_DIR "/memdump1a.dat", DumpType::ADDR, false);   // has address, size, count for fetching buffer from memory
  MemoryDump *newhelperDump = new MemoryDump(EDIZON_DIR "/memdump3a.dat", DumpType::ADDR, true); // has address, size, count for fetching buffer from memory
  if (helperDump->size() == 0)
  {
    (new Snackbar("Helper file not found !"))->show();
    return;
  }

  // helper integrity check
  if (true)
  {
    printf("start helper integrity check value tertiary  \n");
    u32 helpercount = 0;
    helperinfo_t helperinfo;
    for (u64 i = 0; i < helperDump->size(); i += sizeof(helperinfo))
    {
      helperDump->getData(i, &helperinfo, sizeof(helperinfo));
      helpercount += helperinfo.count;
    }
    if (helpercount != (*displayDump)->size() / sizeof(u64))
    {
      printf("Integrity problem with helper file helpercount = %d  memdumpsize = %ld \n", helpercount, (*displayDump)->size() / sizeof(u64));
      (new Snackbar("Helper integrity check failed !"))->show();
      return;
    }
    printf("end helper integrity check value tertiary \n");
  }
  // end helper integrity check

  u8 *ram_buffer = new u8[bufferSize];
  u64 helper_offset = 0;
  helperinfo_t helperinfo;
  helperinfo_t newhelperinfo;
  newhelperinfo.count = 0;
  helperDump->getData(helper_offset, &helperinfo, sizeof(helperinfo)); // helper_offset+=sizeof(helperinfo)
  printf("%s%lx\n", "helperinfo.address ", helperinfo.address);
  printf("%s%lx\n", "helperinfo.size ", helperinfo.size);
  printf("%s%lx\n", "helperinfo.count ", helperinfo.count);
  printf("%s%lx\n", "helper_offset ", helper_offset);
  debugger->readMemory(ram_buffer, helperinfo.size, helperinfo.address);
  // helper init end

  if (oldvalueDump->size() == (*displayDump)->size())
  {
    printf("%s\n", "Found old value");
    have_oldvalue = true;
  }
  else
  {
    (new Snackbar("previous value file not found !"))->show();
    return; /* code */
  }

  searchValue_t value = {0};
  searchValue_t oldvalue = {0};
  searchValue_t Bvalue = {0};

  u64 address = 0;
  while (offset < (*displayDump)->size())
  {

    if ((*displayDump)->size() - offset < bufferSize)
      bufferSize = (*displayDump)->size() - offset;

    (*displayDump)->getData(offset, buffer, bufferSize); // BM6
    if (have_oldvalue)
      oldvalueDump->getData(offset, valuebuffer, bufferSize);
    if ((searchMode == SEARCH_MODE_SAMEA) || (searchMode == SEARCH_MODE_DIFFA)) //read in data A and B
    {
      if (no_existing_dump == false)
        valueDumpB->getData(offset, valueBbuffer, bufferSize);
    }

    printf("%s\n", "buffer loaded");
    for (u64 i = 0; i < bufferSize; i += sizeof(u64)) // for (size_t i = 0; i < (bufferSize / sizeof(u64)); i++)
    {
      if (helperinfo.count == 0)
      {
        if (newhelperinfo.count != 0)
        {
          newhelperinfo.address = helperinfo.address;
          newhelperinfo.size = helperinfo.size;
          newhelperDump->addData((u8 *)&newhelperinfo, sizeof(newhelperinfo));
          newhelperinfo.count = 0;
        };
        helper_offset += sizeof(helperinfo);
        helperDump->getData(helper_offset, &helperinfo, sizeof(helperinfo));
        debugger->readMemory(ram_buffer, helperinfo.size, helperinfo.address);
        printf("%s%lx\n", "helperinfo.address ", helperinfo.address);
        printf("%s%lx\n", "helperinfo.size ", helperinfo.size);
        printf("%s%lx\n", "helperinfo.count ", helperinfo.count);
        printf("%s%lx\n", "helper_offset ", helper_offset);
      }

      address = *reinterpret_cast<u64 *>(&buffer[i]);
      oldvalue._u64 = *reinterpret_cast<u64 *>(&valuebuffer[i]);
      if ((searchMode == SEARCH_MODE_SAMEA) || (searchMode == SEARCH_MODE_DIFFA)) //read in data A and B
      {
        if (no_existing_dump == false)
          Bvalue._u64 = *reinterpret_cast<u64 *>(&valueBbuffer[i]);
      }

      // fetch value from buffer
      // ram_buffer + i == &ram_buffer[i]
      // value._u64 = 0;
      memset(&value, 0, 8);
      memcpy(&value, ram_buffer + address - helperinfo.address, dataTypeSizes[searchType]);
      helperinfo.count--;

      // searchValue_t *foobar = reinterpret_cast<searchValue_t *>(ram_buffer + (address - helperinfo.address))
      //_u32 bar = (*foobar)._u32;
      //(reinterpret_cast<searchValue_t *>(ram_buffer + (address - helperinfo.address)))._u32

      // debugger->readMemory(&value, dataTypeSizes[searchType], address);

      if (i % 10000 == 0)
      {
        setLedState(ledOn);
        ledOn = !ledOn;
      }

      switch (searchMode)
      {
      case SEARCH_MODE_SAME:
        if (value._s64 == oldvalue._s64)
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newvalueDump->addData((u8 *)&value, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_SAMEA:
        if (no_existing_dump)
        {
          printf("this shouldn't happen\n");
          if (value._s64 != Bvalue._s64) //change
          {
            newDump->addData((u8 *)&address, sizeof(u64));
            newvalueDump->addData((u8 *)&value, sizeof(u64));
            newvalueDumpA->addData((u8 *)&value, sizeof(u64));
            // newvalueDumpA->addData((u8 *)&oldvalue, sizeof(u64));
            newvalueDumpB->addData((u8 *)&Bvalue, sizeof(u64));
            newhelperinfo.count++;
          }
        }
        else if (value._s64 == oldvalue._s64)
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newvalueDump->addData((u8 *)&value, sizeof(u64));
          newvalueDumpA->addData((u8 *)&oldvalue, sizeof(u64));
          newvalueDumpB->addData((u8 *)&Bvalue, sizeof(u64)); //create new file and later rename to A, need this new file for size of A to be in sync
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_DIFFA: //need to be same as B
        if (no_existing_dump)
        {
          if (value._s64 != oldvalue._s64) // change
          {
            newDump->addData((u8 *)&address, sizeof(u64));
            newvalueDump->addData((u8 *)&value, sizeof(u64));
            newvalueDumpA->addData((u8 *)&oldvalue, sizeof(u64));
            newvalueDumpB->addData((u8 *)&value, sizeof(u64));
            // newvalueDumpB->addData((u8 *)&Bvalue, sizeof(u64));
            newhelperinfo.count++;
          }
        }
        else if (value._s64 == Bvalue._s64)
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newvalueDump->addData((u8 *)&value, sizeof(u64));
          newvalueDumpA->addData((u8 *)&oldvalue, sizeof(u64));
          newvalueDumpB->addData((u8 *)&Bvalue, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_DIFF:
        if ((value._s64 != oldvalue._s64))// && (value._u64 <= m_heapBaseAddr || value._u64 >= (m_heapEnd)))
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newvalueDump->addData((u8 *)&value, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_INC:
        if (value._s64 > oldvalue._s64)
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newvalueDump->addData((u8 *)&value, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_DEC:
        if (value._s64 < oldvalue._s64)
        {
          newDump->addData((u8 *)&address, sizeof(u64));
          newvalueDump->addData((u8 *)&value, sizeof(u64));
          newhelperinfo.count++;
        }
        break;
      case SEARCH_MODE_NONE:
      case SEARCH_MODE_POINTER:
      case SEARCH_MODE_RANGE:
      case SEARCH_MODE_NEQ:
      case SEARCH_MODE_EQ:
      case SEARCH_MODE_GT:
      case SEARCH_MODE_LT:
      case SEARCH_MODE_NOT_POINTER:
        break;
      }
    }

    offset += bufferSize;
    // update newhelperinfo

    printf("%s%lx%s%lx\n", "(*displayDump)->size() ", (*displayDump)->size(), "Offset ", offset);
  }

  if (newhelperinfo.count != 0) // need to take care of the last one
  {
    newhelperinfo.address = helperinfo.address;
    newhelperinfo.size = helperinfo.size;
    newhelperDump->addData((u8 *)&newhelperinfo, sizeof(newhelperinfo));
    newhelperinfo.count = 0;
  };
  newDump->flushBuffer();
  newvalueDump->flushBuffer();
  //end
  // should just rename the file ??
  if (newDump->size() > 0)
  {
    printf("%s%lx\n", "newDump->size() ", newDump->size());
  }
  else
  {
    (new Snackbar("None of values changed to the entered one!"))->show();
  }

  setLedState(false);
  delete[] ram_buffer;
  delete[] valuebuffer;
  delete[] valueBbuffer;
  delete[] buffer;
  time_t unixTime2 = time(NULL);
  printf("%s%lx\n", "Stop Time Tertiary search ", unixTime2);
  printf("%s%ld\n", "Stop Time ", unixTime2 - unixTime1);

  newvalueDump->flushBuffer();
  newDump->flushBuffer();
  newhelperDump->flushBuffer();
  newvalueDumpA->flushBuffer();
  newvalueDumpB->flushBuffer();

  delete newvalueDump;
  delete newDump;
  delete newhelperDump;
  delete newvalueDumpA;
  delete newvalueDumpB;

  delete oldvalueDump; //needed to close the file
  delete helperDump;
  delete valueDumpB;

  printf("Done Tertiary \n");
  // remove(EDIZON_DIR "/memdump3.dat");
}

// here

void GuiCheats::pointercheck()
{
  printf("checking pointer...\n");
  m_pointeroffsetDump = new MemoryDump(EDIZON_DIR "/ptrdump1.dat", DumpType::POINTER, false);
  if (m_pointeroffsetDump->size() > 0)
  {
    u64 offset = 0;
    u64 bufferSize = MAX_BUFFER_SIZE;
    u8 *buffer = new u8[bufferSize];
    pointer_chain_t pointer_chain;
    while (offset < m_pointeroffsetDump->size())
    {
      if (m_pointeroffsetDump->size() - offset < bufferSize)
        bufferSize = m_pointeroffsetDump->size() - offset;
      m_pointeroffsetDump->getData(offset, buffer, bufferSize);
      for (u64 i = 0; i < bufferSize; i += sizeof(pointer_chain_t))
      {
        pointer_chain = *reinterpret_cast<pointer_chain_t *>(&buffer[i]);
        u64 nextaddress = m_mainBaseAddr;
        printf("main[%lx]", nextaddress);
        // m_debugger->readMemory(&nextaddress, sizeof(u64), ( m_mainBaseAddr+ pointer_chain.offset[pointer_chain.depth]));
        // printf("(&lx)", nextaddress);
        for (int z = pointer_chain.depth; z >= 0; z--)
        {
          printf("+%lx z=%d ", pointer_chain.offset[z], z);
          nextaddress += pointer_chain.offset[z];
          printf("[%lx]", nextaddress);
          MemoryInfo meminfo = m_debugger->queryMemory(nextaddress);
          if (meminfo.perm == Perm_Rw)
            m_debugger->readMemory(&nextaddress, sizeof(u64), nextaddress);
          else
          {
            printf("*access denied*");
            break;
          }
          printf("(%lx)", nextaddress);
        }
        printf("\n\n");
      }
      offset += bufferSize;
    }
    delete[] buffer;
  }
  else
  {
    printf("no saved poiters\n");
  }
}

// void GuiCheats::startpointersearch(u64 targetaddress) //, MemoryDump **displayDump, MemoryDump **dataDump, pointer_chain_t pointerchain)
// {
//   m_dataDump = new MemoryDump(EDIZON_DIR "/datadump2.dat", DumpType::DATA, false);           // pointed targets is in this file
//   m_pointeroffsetDump = new MemoryDump(EDIZON_DIR "/ptrdump1.dat", DumpType::POINTER, true); // create file but maybe later just open it
//   pointer_chain_t pointerchain;
//   printf("Start pointer search %lx\n", targetaddress);
//   m_Time1 = time(NULL);
//   m_pointer_found = 0;
//   pointerchain.depth = 0;
//   m_abort = false;
//   try
//   {
//     pointersearch(targetaddress, pointerchain); //&m_memoryDump, &m_dataDump,
//   }
//   catch (...)
//   {
//     printf("Caught an exception\n");
//   }
//   // add some rubbish just for testing
//   // char st[250];                                         // replace the printf
//   // snprintf(st, 250, "Just for testing ====="); //
//   // m_pointeroffsetDump->addData((u8 *)&st, sizeof(st)); //
//
//   m_pointeroffsetDump->flushBuffer();
//   delete m_pointeroffsetDump;
//   printf("End pointer search \n");
//   printf("Time taken =%ld  Found %ld pointer chain\n", time(NULL) - m_Time1, m_pointer_found);
// }

void GuiCheats::rebasepointer(searchValue_t value) //struct bookmark_t bookmark) //Go through memory and add to bookmark potential target with different first offset
{
  STARTTIMER
  for (MemoryInfo meminfo : m_memoryInfo)
  {
    if (meminfo.type != MemType_CodeWritable && meminfo.type != MemType_CodeMutable)
      continue;

    u64 offset = 0;
    u64 bufferSize = MAX_BUFFER_SIZE; // consider to increase from 10k to 1M (not a big problem)
    u8 *buffer = new u8[bufferSize];
    while (offset < meminfo.size)
    {
      if (meminfo.size - offset < bufferSize)
        bufferSize = meminfo.size - offset;
      try
      {
        m_debugger->readMemory(buffer, bufferSize, meminfo.addr + offset);
      }
      catch (...)
      {
        printf(" have error with readmemory \n");
      };

      searchValue_t realValue = {0};
      for (u32 i = 0; i < bufferSize; i += sizeof(u64))
      {
        u64 Address = meminfo.addr + offset + i;
        memset(&realValue, 0, 8);
        if (m_32bitmode)
          memcpy(&realValue, buffer + i, sizeof(u32));
        else
          memcpy(&realValue, buffer + i, sizeof(u64));

        if ((realValue._u64 >= m_heapBaseAddr) && (realValue._u64 <= (m_heapEnd)))
        {
          bookmark.pointer.offset[bookmark.pointer.depth] = Address - m_mainBaseAddr;
          bool success = true;
          u64 nextaddress = m_mainBaseAddr;
          u64 address;
#ifdef printpointerchain
          printf("main[%lx]", nextaddress);
#endif
          for (int z = bookmark.pointer.depth; z >= 0; z--)
          {
            // bookmark_t bm;
#ifdef printpointerchain
            printf("+%lx z=%d ", bookmark.pointer.offset[z], z);
#endif
            nextaddress += bookmark.pointer.offset[z];
#ifdef printpointerchain
            printf("[%lx]", nextaddress);
#endif
            MemoryInfo meminfo = m_debugger->queryMemory(nextaddress);
            if (meminfo.perm == Perm_Rw)
            {
              address = nextaddress;
              m_debugger->readMemory(&nextaddress, sizeof(u64), nextaddress);
            }
            else
            {
#ifdef printpointerchain
              printf("*access denied*\n");
#endif
              success = false;
              break;
            }
#ifdef printpointerchain
            printf("(%lx)", nextaddress);
#endif
          }
#ifdef printpointerchain
          printf("\n");
#endif

          if (success && (bookmark.pointer.depth > 4 || valuematch(value, address)))
          {
            bookmark.type = m_searchType;
            m_memoryDumpBookmark->addData((u8 *)&address, sizeof(u64));
            m_AttributeDumpBookmark->addData((u8 *)&bookmark, sizeof(bookmark_t));
#ifdef printpointerchain
            printf("Success  BM added   \n");
#endif
            // (new Snackbar("Adding address from cheat to bookmark"))->show();
          }
        }
      }
      offset += bufferSize;
    }
    delete[] buffer;
  }
  ENDTIMER
  m_memoryDumpBookmark->flushBuffer();
  m_AttributeDumpBookmark->flushBuffer();
}

// bool GuiCheats::check_chain(bookmark_t *bookmark, u64 *address)
// {
//   // return false;
//   bool success = true;
//   u64 nextaddress = m_mainBaseAddr;
//   printf("main[%lx]", nextaddress);
//   for (int z = (*bookmark).pointer.depth; z >= 0; z--)
//   {
//     printf("+%lx z=%d ", (*bookmark).pointer.offset[z], z);
//     nextaddress += (*bookmark).pointer.offset[z];
//     printf("[%lx]", nextaddress);
//     MemoryInfo meminfo = m_debugger->queryMemory(nextaddress);
//     if (meminfo.perm == Perm_Rw)
//     {
//       *address = nextaddress;
//       m_debugger->readMemory(&nextaddress, sizeof(u64), nextaddress);
//     }
//     else
//     {
//       printf("*access denied*");
//       success = false;
//       break;
//     }
//     printf("(%lx)", nextaddress);
//   }
//   printf("\n");
//   return success;
// }

// void GuiCheats::pointersearch(u64 targetaddress, struct pointer_chain_t pointerchain) //MemoryDump **displayDump, MemoryDump **dataDump,
// {
//   // printf("target address = %lx depth = %d \n", targetaddress, pointerchain.depth);
//
//   // printf("check point 1a\n");
//   if ((m_mainBaseAddr <= targetaddress) && (targetaddress <= (m_mainend)))
//   {
//     printf("\ntarget reached!=========================\n");
//     printf("final offset is %lx \n", targetaddress - m_mainBaseAddr);
//     // pointerchain.depth++;
//     // pointerchain.offset[pointerchain.depth] = targetaddress - m_mainBaseAddr;
//     //   // save pointerchain
//     pointerchain.offset[pointerchain.depth] = targetaddress - m_mainBaseAddr;
//     m_pointeroffsetDump->addData((u8 *)&pointerchain, sizeof(pointer_chain_t));
//     m_pointeroffsetDump->flushBuffer(); // is this useful?
//     printf("main");
//     for (int z = pointerchain.depth; z >= 0; z--)
//       printf("+%lx z=%d ", pointerchain.offset[z], z);
//     printf("\n\n");
//     // printf("\nsize=%d\n", sizeof(pointer_chain_t));
//     m_pointer_found++;
//     return; // consider don't return to find more
//   };
//
//   if (pointerchain.depth == m_max_depth)
//   {
//     // printf("max pointer depth reached\n\n");
//     return;
//   }
//
//   // printf("\n starting pointer search for address = %lx at depth %d ", targetaddress, pointerchain.depth);
//   u64 offset = 0;
//   u64 thefileoffset = 0;
//   u64 bufferSize = MAX_BUFFER_SIZE;
//   u8 *buffer = new u8[bufferSize];
//   u64 distance;
//   u64 minimum = m_max_range;         // a large number to start
//   std::vector<sourceinfo_t> sources; // potential sources that points at target with a offset, we will search for the nearest address being pointed by pointer/pointers
//   sourceinfo_t sourceinfo;
//   // std::vector<u64> distances;
//
//   while (offset < m_dataDump->size())
//   {
//     if (m_dataDump->size() - offset < bufferSize)
//       bufferSize = m_dataDump->size() - offset;
//     // printf("checkpoint 2\n");
//     m_dataDump->getData(offset, buffer, bufferSize); // BM4
//     bool writeback = false;
//     // printf("checkpoint 3\n");
//     // return;                                           // just to check
//     for (u64 i = 0; i < bufferSize; i += sizeof(u64)) // for (size_t i = 0; i < (bufferSize / sizeof(u64)); i++)
//     {
//       if (m_abort)
//         return;
//       u64 pointedaddress = *reinterpret_cast<u64 *>(&buffer[i]);
//       if (targetaddress >= pointedaddress)
//       {
//         distance = targetaddress - pointedaddress;
//         if (distance < minimum)
//         {
//           // minimum = distance;
//           // sources.clear();
//           sourceinfo.foffset = offset + i;
//           sourceinfo.offset = distance;
//           sources.push_back(sourceinfo);
//           thefileoffset = offset + i;
//           // *reinterpret_cast<u64 *>(&buffer[i]) = 0; // to prevent endless loop
//           // writeback = true;                         //
//         }
//         else if (distance == minimum)
//         {
//           sourceinfo.foffset = offset + i;
//           sourceinfo.offset = distance;
//           sources.push_back(sourceinfo);
//           // sources.push_back(offset + i);
//           thefileoffset = offset + i;
//           // *reinterpret_cast<u64 *>(&buffer[i]) = 0; // to prevent endless loop
//           // writeback = true;                         //
//           // pointerchain.fileoffset[pointerchain.depth] = offset + i;
//           // pointerchain.offset[pointerchain.depth] = distance;
//         }
//       }
//       if (sources.size() > m_max_source)
//         break;
//     }
//     if (sources.size() > m_max_source)
//       break;
//     // if (writeback)
//     // {
//     //   m_dataDump->putData(offset, buffer, bufferSize);
//     //   m_dataDump->flushBuffer();
//     // }
//     offset += bufferSize;
//   }
//   delete[] buffer; // release memory use for the search of sources
//
//   // Now we have fileoffsets stored in sources to repeat this process
//   // printf("memory scan completed offset is %lx at depth %lx\n\n", minimum, pointerchain.depth);
//   // pointerchain.offset[pointerchain.depth] = minimum;
//   pointerchain.depth++;
//
//   printf("**Found %ld sources for address %lx at depth %ld\n", sources.size(), targetaddress, pointerchain.depth);
//   for (sourceinfo_t sourceinfo : sources)
//   {
//     // targetaddress = 0x1000;
//     // printf("size of memorydump is %lx ", m_memoryDump1->size()); // I swapped the bookmark
//     //m_memoryDump->getData((m_selectedEntry + m_addresslist_offset) * sizeof(u64), &m_EditorBaseAddr, sizeof(u64));
//     u64 newtargetaddress;
//     m_memoryDump1->getData(sourceinfo.foffset, &newtargetaddress, sizeof(u64)); // fileoffset is in byte
//
//     // u64 checkaddress;                                             // debug use
//     // m_dataDump->getData(foffset, &checkaddress, sizeof(u64));     //double check it for debug purpose
//     // printf("fileoffset = %lx thefileoffset =%lx new target address is %lx old target was %lx\n", sourceinfo.foffset, thefileoffset, newtargetaddress, targetaddress);
//     if (m_forwardonly)
//     {
//       if ((targetaddress > newtargetaddress) || ((m_mainBaseAddr <= newtargetaddress) && (newtargetaddress <= (m_mainend))))
//       {
//         pointerchain.fileoffset[pointerchain.depth - 1] = sourceinfo.foffset;
//         pointerchain.offset[pointerchain.depth - 1] = sourceinfo.offset;
//         pointersearch(newtargetaddress, pointerchain);
//       }
//     }
//     else
//     {
//       /* code */
//       pointerchain.fileoffset[pointerchain.depth - 1] = sourceinfo.foffset;
//       pointerchain.offset[pointerchain.depth - 1] = sourceinfo.offset;
//       pointersearch(newtargetaddress, pointerchain);
//     }
//   }
//
//   return;
//
//   // (*displayDump)->getData(pointerchain.fileoffset[pointerchain.depth] * sizeof(u64), &address, sizeof(u64));
//
//   // printf("depth is %d new address is %lx offset is %lx code offset is %lx \n", pointerchain.depth, address, pointerchain.fileoffset[pointerchain.depth], pointerchain.offset[pointerchain.depth]);
//   // if (address < m_mainBaseAddr + m_mainSize)
//   // {
//   //   printf("target reached!");
//   //   printf("final offset is %lx \n", address - m_mainBaseAddr);
//   //   pointerchain.depth++;
//   //   pointerchain.offset[pointerchain.depth] = address - m_mainBaseAddr;
//   //   // save pointerchain
//   //   m_pointeroffsetDump->addData((u8 *)&pointerchain, sizeof(pointer_chain_t));
//   //   return;
// }
// // change address to new one
void GuiCheats::startpointersearch2(u64 targetaddress) // using global m_bookmark which has the label field already set correctly
{
  m_PointerSearch = new PointerSearch_state;
  m_dataDump = new MemoryDump(EDIZON_DIR "/datadump2.dat", DumpType::DATA, false); // pointed targets is in this file
  printf("Start pointer search %lx\n", targetaddress);
  m_Time1 = time(NULL);
  m_pointer_found = 0;
  m_abort = false;
  m_PS_resume = false;
  m_PS_pause = false;
  // PS_depth = 0;   // need this if m_PointerSearch isn't created here
  // m_PointerSearch->index = {0}; //
  try
  {
    pointersearch2(targetaddress, 0); //&m_memoryDump, &m_dataDump,
  }
  catch (...)
  {
    printf("Caught an exception\n");
  }
  printf("End pointer search \n");
  printf("Time taken =%ld  Found %ld pointer chain\n", time(NULL) - m_Time1, m_pointer_found);

  if (!m_PS_pause)
  {
    delete m_PointerSearch;
  }
  else
    m_pointersearch_canresume = true;
  delete m_dataDump;
}
void GuiCheats::resumepointersearch2()
{
  if (m_pointersearch_canresume)
  {
    m_PS_resume = true;
    m_PS_pause = false;
    m_dataDump = new MemoryDump(EDIZON_DIR "/datadump2.dat", DumpType::DATA, false);
    pointersearch2(0, 0);
    delete m_dataDump;
    if (!m_PS_pause)
    {
      delete m_PointerSearch;
    }
  }
}

void GuiCheats::pointersearch2(u64 targetaddress, u64 depth) //MemoryDump **displayDump, MemoryDump **dataDump,
{
  if (!m_PS_resume)
  {
    // printf("targetaddress %lx PS_depth %ld PS_index %ld PS_num_sources %ld\n", targetaddress, PS_depth, PS_index, PS_num_sources);
    if ((m_mainBaseAddr <= targetaddress) && (targetaddress <= (m_mainend)))
    {

      printf("\ntarget reached!=========================\n");
      printf("final offset is %lx \n", targetaddress - m_mainBaseAddr);
      m_bookmark.pointer.offset[PS_depth] = targetaddress - m_mainBaseAddr;
      m_bookmark.pointer.depth = PS_depth;
      for (int z = PS_depth - 1; z >= 0; z--)
      {
        m_bookmark.pointer.offset[z] = m_PointerSearch->sources[z][m_PointerSearch->index[z]].offset;
      }

      m_AttributeDumpBookmark->addData((u8 *)&m_bookmark, sizeof(bookmark_t));
      m_AttributeDumpBookmark->flushBuffer();
      m_memoryDumpBookmark->addData((u8 *)&m_mainBaseAddr, sizeof(u64)); //need to update
      m_memoryDumpBookmark->flushBuffer();
      // m_pointeroffsetDump->addData((u8 *)&pointerchain, sizeof(pointer_chain_t));
      // m_pointeroffsetDump->flushBuffer(); // is this useful?
      printf("main");
      for (int z = m_bookmark.pointer.depth; z >= 0; z--)
        printf("+%lx z=%d ", m_bookmark.pointer.offset[z], z);
      printf("\n\n");
      m_pointer_found++;
      // return; // consider don't return to find more
    };
    if (PS_depth == m_max_depth)
    {
      // printf("max pointer depth reached\n\n");
      return;
    }
    u64 offset = 0;
    // u64 thefileoffset;
    u64 bufferSize = MAX_BUFFER_SIZE;
    u8 *buffer = new u8[bufferSize];
    u64 distance;
    u64 minimum = m_max_range; // a large number to start
    sourceinfo_t sourceinfo;
    std::vector<std::vector<sourceinfo_t>> sources = {{}};
    // printf("PS_num_sources %d ", PS_num_sources);
    PS_num_sources = 0;
    while (offset < m_dataDump->size())
    {
      if (m_dataDump->size() - offset < bufferSize)
        bufferSize = m_dataDump->size() - offset;
      m_dataDump->getData(offset, buffer, bufferSize); // BM4

      for (u64 i = 0; i < bufferSize; i += sizeof(u64)) // for (size_t i = 0; i < (bufferSize / sizeof(u64)); i++)
      {
        if (m_abort)
          return;
        u64 pointedaddress = *reinterpret_cast<u64 *>(&buffer[i]);
        if (targetaddress >= pointedaddress)
        {
          distance = targetaddress - pointedaddress;
          if (distance <= minimum)
          {
            sourceinfo.foffset = offset + i;
            sourceinfo.offset = distance;
            // PS_sources[PS_num_sources] = sourceinfo;
            // PS_num_sources++;
            for (u32 j = 0; j < sources.size(); j++)
            {

              if (sources[j].size() == 0 || sources[j][0].offset == distance)
              {
                sources[j].push_back(sourceinfo);
                break;
              }
              else if (sources[j][0].offset > distance)
              {
                sources.insert(sources.begin() + j, {sourceinfo});
                break;
              }
              else if (j == sources.size() - 1)
              {
                sources.push_back({sourceinfo});
              }
            }
          }
          // else if (distance == minimum)
          // {
          //   sourceinfo.foffset = offset + i;
          //   sourceinfo.offset = distance;
          //   PS_sources[PS_num_sources] = sourceinfo;
          //   PS_num_sources++;
          // }
        }
        // if (PS_num_sources > m_max_source)
        //   break;
      }
      // if (PS_num_sources > m_max_source)
      //   break;
      offset += bufferSize;
    }

    PS_num_sources = 0;
    for (u32 j = 0; j < sources.size(); j++)
    {
      if (j > m_numoffset)
        break;
      for (u32 k = 0; k < sources[j].size(); k++)
      {
        PS_sources[PS_num_sources] = sources[j][k];
        PS_num_sources++;
        if (PS_num_sources > m_max_source)
          break;
      }
      if (PS_num_sources > m_max_source)
        break;
    }

    delete[] buffer; // release memory use for the search of sources
    // printf("**Found %ld sources for address %lx at depth %ld\n", PS_num_sources, targetaddress, PS_depth);
    PS_index = 0;
  }
  else if (PS_depth == PS_lastdepth)
  {
    m_PS_resume = false;
  }

  while (PS_index < PS_num_sources)
  {
    hidScanInput();
    u32 kheld = hidKeysHeld(CONTROLLER_PLAYER_1) | hidKeysHeld(CONTROLLER_HANDHELD);
    // u32 kdown = hidKeysDown(CONTROLLER_PLAYER_1) | hidKeysDown(CONTROLLER_HANDHELD);
    if ((kheld & KEY_B) && (kheld & KEY_ZL))
    {
      m_PS_pause = true;
      PS_lastdepth = PS_depth;
    }
    if (m_PS_pause)
      return;

    // status update
    std::stringstream SS;
    SS.str("");
    SS << "F=" << std::setw(2) << m_pointer_found;
    for (u64 i = 0; i < m_max_depth; i++)
    {
      SS << " Z=" << i << ":" << std::setfill('0') << std::setw(2) << m_PointerSearch->index[i]
         << "/" << std::setfill('0') << std::setw(2) << m_PointerSearch->num_sources[i] << " ";
      if (i == 5 || i == 11)
        SS << "\n";
    }
    // SS << "\n";
    // printf(SS.str().c_str());
    Gui::beginDraw();
    Gui::drawRectangle(70, 420, 1150, 65, currTheme.backgroundColor);
    Gui::drawTextAligned(font20, 70, 420, currTheme.textColor, SS.str().c_str(), ALIGNED_LEFT);
    Gui::endDraw();

    u64 newtargetaddress;
    m_memoryDump1->getData(PS_sources[PS_index].foffset, &newtargetaddress, sizeof(u64)); // fileoffset is in byte
    if (m_forwardonly)
    {
      if ((targetaddress > newtargetaddress) || ((m_mainBaseAddr <= newtargetaddress) && (newtargetaddress <= (m_mainend))))
      {
        pointersearch2(newtargetaddress, PS_depth + 1);
      }
    }
    else
    {
      pointersearch2(newtargetaddress, PS_depth + 1);
    }
    if (m_PS_pause)
      return;
    PS_index++;
  }
  return;
}
// printf("not found \n");
// return;
// m_targetmemInfos.clear();
// m_target = address;
// m_max_depth = depth;
// m_max_range = range;
// m_numoffset = num;
// std::vector<MemoryInfo> mainInfos;
// mainInfos.clear();
// m_low_main_heap_addr = 0x100000000000;
// m_high_main_heap_addr = 0;
// for (MemoryInfo meminfo : m_memoryInfo)
// {
//   // if (m_searchRegion == SEARCH_REGION_RAM)
//   //   if ((meminfo.perm & Perm_Rw) != Perm_Rw) continue; else
//   if (meminfo.type != MemType_Heap && meminfo.type != MemType_CodeWritable && meminfo.type != MemType_CodeMutable)
//     continue;
//   if (meminfo.addr < m_low_main_heap_addr)
//     m_low_main_heap_addr = meminfo.addr;
//   if ((meminfo.addr + meminfo.size) > m_high_main_heap_addr)
//     m_high_main_heap_addr = (meminfo.addr + meminfo.size);
//   m_targetmemInfos.push_back(meminfo);
//   if (meminfo.type == MemType_CodeWritable || meminfo.type == MemType_CodeMutable)
//   {
//     mainInfos.push_back(meminfo);
//     //
//     printf("%s%p", "meminfo.addr, ", meminfo.addr);
//     printf("%s%p", ", meminfo.end, ", meminfo.addr + meminfo.size);
//     printf("%s%p", ", meminfo.size, ", meminfo.size);
//     printf("%s%lx", ", meminfo.type, ", meminfo.type);
//     printf("%s%lx", ", meminfo.attr, ", meminfo.attr);
//     printf("%s%lx", ", meminfo.perm, ", meminfo.perm);
//     printf("%s%lx", ", meminfo.device_refcount, ", meminfo.device_refcount);
//     printf("%s%lx\n", ", meminfo.ipc_refcount, ", meminfo.ipc_refcount);
//     //
//   }
// }
// m_Time1 = time(NULL);
// printf("searching pointer for address %lx\n Range %lx .. %lx ", m_target, m_low_main_heap_addr, m_high_main_heap_addr);
// for (u8 i = 0; i < 20; i++)
//   m_hitcount.offset[i] = 0;
// for (MemoryInfo meminfo : mainInfos)
// {
//   if (meminfo.addr < m_mainBaseAddr)
//     continue;
//   pointer_chain_t ptrchain;
//   ptrchain.offset[0] = meminfo.addr - m_mainBaseAddr;
//   ptrchain.depth = 0;
//   printf("offset %lx \n ", ptrchain.offset[0]);
//   //
//   printf("%s%p", "meminfo.addr, ", meminfo.addr);
//   printf("%s%p", ", meminfo.end, ", meminfo.addr + meminfo.size);
//   printf("%s%p", ", meminfo.size, ", meminfo.size);
//   printf("%s%lx", ", meminfo.type, ", meminfo.type);
//   printf("%s%lx", ", meminfo.attr, ", meminfo.attr);
//   printf("%s%lx", ", meminfo.perm, ", meminfo.perm);
//   printf("%s%lx", ", meminfo.device_refcount, ", meminfo.device_refcount);
//   printf("%s%lx\n", ", meminfo.ipc_refcount, ", meminfo.ipc_refcount);
//   //
//   // return;
//   printf("Top level meminfo.addr %lx\n time= %d\n", meminfo.addr, time(NULL) - m_Time1);
//   searchpointer(meminfo.addr, meminfo.size / sizeof(u64), meminfo.size, ptrchain);
//   //
//   printf("hit count depth");
//   for (u8 i = 0; i < 20; i++)
//     printf("%d= %d ", i, m_hitcount.offset[i]);
// void GuiCheats::searchpointer(u64 address, u64 depth, u64 range, struct pointer_chain_t pointerchain) //assumed range don't extend beyond a segment, need to make seperate call to cover multi segment
// {
//   // using global to reduce overhead
//   // use separate function if need to get rid of range in the passed variable     // u64 m_max_depth; used in first call
//   // u64 m_target;
//   // u64 m_numoffset;
//   // u64 m_max_range;
//   // u64 m_low_main_heap_addr; The lowerst of main or heap start
//   // u64 m_high_main_heap_addr; The highest
//   // printf("in function current depth is %d @@@@@@@@@@@@@@@@@@@@@\n", depth);
//   // return;
//   m_hitcount.offset[pointerchain.depth]++;
//
//   if (address <= m_target && m_target <= address + range)
//   {
//     printf("found =========================");
//     pointerchain.offset[pointerchain.depth] = m_target - address;
//     pointerchain.depth++;
//     m_pointeroffsetDump->addData((u8 *)&pointerchain, sizeof(pointer_chain_t)); //((u8 *)&address, sizeof(u64));
//     // *m_pointeroffsetDump->getData(offset * sizeof(pointer_chain_t) , void *buffer, size_t bufferSize);
//     printf("found at depth %ld\n", pointerchain.depth);
//     return;
//   }
//   if (depth == 0)
//   {
//     // printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
//     return;
//     // printf("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
//   }
//   pointerchain.depth++; // for all call
//   depth--;              // for all call
//   // u8 *buffer = new u8[range];
//   u32 num = m_numoffset;
//   u32 nextrange;
//   u64 nextaddress;
//   // u32 endaddress = address + range;
//   // printf("I am at 1");
//   u64 bufferSize = MAX_BUFFER_SIZE;
//   if (range < bufferSize)
//     bufferSize = range;
//   u8 *buffer = new u8[bufferSize];
//   // printf("I am at 2");
//   for (MemoryInfo meminfo : m_targetmemInfos) // a shorten list that has only the real targets
//   {
//     if (address < meminfo.addr)
//     {
//       // printf("I am at 4");
//       return; // address not accessible}
//     }
//     if (address > meminfo.addr + meminfo.size)
//     {
//       // printf("I am at 5, address =%lx meminfo.addr = %1x, meminfo.size =%1x \n", address, meminfo.addr, meminfo.size);
//       continue; // next segment
//     }
//     u64 offset = 0;
//     u64 segmentend = meminfo.addr + meminfo.size;
//     // printf("I am at 3\n");
//     while (address + offset < segmentend)
//     {
//       if (segmentend - (address + offset) < bufferSize)
//         bufferSize = segmentend - (address + offset);
//
//       // printf("reading address %lx bufferSize %lx meminfo.addr is %lx meminfo.size is %lx   ", (address + offset), bufferSize, meminfo.addr, meminfo.size);
//       // printf("Time since last update %d \n", time(NULL) - m_Time1); //   printf("Top level meminfo.addr %lx\n time= %d\n", meminfo.addr, time(NULL) - m_Time1);
//       // return;
//       m_debugger->readMemory(buffer, bufferSize, (address + offset));
//       for (u64 i = 0; i < bufferSize; i += sizeof(u64)) //for (u64 i = 0; i < bufferSize; i += dataTypeSizes[searchType])
//       {
//         nextaddress = *reinterpret_cast<u64 *>(&buffer[i]);
//         // printf("nextaddress = %lx \n", nextaddress);
//         if ((nextaddress >= m_low_main_heap_addr) && (nextaddress <= m_high_main_heap_addr))
//         {
//           // printf("found ptr === %lx ======================================= pointerchain.depth is %d ==============offset+i is  %d \n",nextaddress, pointerchain.depth, offset + i);
//           pointerchain.offset[pointerchain.depth] = offset + i; // per call
//           searchpointer(nextaddress, depth, m_max_range, pointerchain);
//           num--;
//           if (num == 0)
//           {
//             // printf("not found returning &&&&&&&&&&&&&&&&&&&&\n\n");
//             return;
//           }
//         }
//         range -= sizeof(u64);
//         if (range == 0)
//           return;
//       }
//
//       offset += bufferSize;
//     }
//   }
//   delete[] buffer;
// }
/**
 * Primary:
 *  Initial full memory dump regardless of type
 *  Differentiate between different regions and types
 * 
 * Secondary:
 *  Second full memory dump regardless of type
 *  Differentiate between regions and types. (both fix now)
 * 
 *  Compare both memory dumps based on type and mode
 *   Store match addresses into additional file
 *   Matches should be stored as [MEMADDR][DUMPADDR] for fast comparing later on
 * 
 * Tertiary (Loop infinitely):
 *  Iterate over match addrs file 
 *   Compare value in memory at [MEMADDR] with value in second memory dump at [DUMPADDR]
 *   Store match addresses into file (displayDump)
 *   Dump all values from changed addresses into a file
 *   Matches should be stored as [MEMADDR][DUMPADDR] for fast comparing later on
 */
void GuiCheats::_writegameid()
{
  FILE *pfile;
  pfile = fopen(EDIZON_DIR "/ID.txt", "w");
  if (Title::g_titles[m_debugger->getRunningApplicationTID()] != nullptr)
    fputs(Title::g_titles[m_debugger->getRunningApplicationTID()]->getTitleName().c_str(),pfile);
  fputs("\n",pfile);
  fputs(tidStr.c_str(),pfile);
  fputs("\n", pfile);
  fputs(buildIDStr.c_str(),pfile);
  fputs("\n", pfile);
  fputs(cheatpathStr.c_str(),pfile);
  fclose(pfile);
}
void GuiCheats::_moveLonelyCheats(u8 *buildID, u64 titleID)
{
  std::stringstream lonelyCheatPath;
  // std::stringstream EdizonCheatPath;
  std::stringstream realCheatPath;

  std::stringstream buildIDStr;

  for (u8 i = 0; i < 8; i++)
    buildIDStr << std::uppercase << std::hex << std::setfill('0') << std::setw(2) << (u16)buildID[i];

  lonelyCheatPath << EDIZON_DIR "/cheats/" << buildIDStr.str() << ".txt";
  // EdizonCheatPath << EDIZON_DIR "/" << buildIDStr.str() << ".txt";

  if (access(lonelyCheatPath.str().c_str(), F_OK) == 0)
  {
    realCheatPath << "/atmosphere/contents/" << std::uppercase << std::hex << std::setfill('0') << std::setw(sizeof(u64) * 2) << titleID;
    mkdir(realCheatPath.str().c_str(), 0777);
    realCheatPath << "/cheats/";
    mkdir(realCheatPath.str().c_str(), 0777);

    realCheatPath << buildIDStr.str() << ".txt";

    REPLACEFILE(lonelyCheatPath.str().c_str(), realCheatPath.str().c_str());
    if (!(m_debugger->m_dmnt))
    {
      m_debugger->detatch();
      dmntchtForceOpenCheatProcess();
      printf("force open called\n");
    }
    else
      (new MessageBox("A new cheat has been added for this title. \n Please restart the game to start using it.", MessageBox::OKAY))->show();
  }
  // Move cheat from code database if exist
  std::stringstream zipPath;
  zipPath << EDIZON_DIR "/cheats/" << "titles.zip";
  Config::readConfig();
  if (access(zipPath.str().c_str(), F_OK) == 0)// && Config::getConfig()->enablecheats)
  {
    Config::writeConfig();
    realCheatPath.str("");
    realCheatPath << "/atmosphere/contents/" << std::uppercase << std::hex << std::setfill('0') << std::setw(sizeof(u64) * 2) << titleID;
    mkdir(realCheatPath.str().c_str(), 0777);
    realCheatPath << "/cheats/";
    // realCheatPath << "\\atmosphere\\contents\\" << std::uppercase << std::hex << std::setfill('0') << std::setw(sizeof(u64) * 2) << titleID;
    // mkdir(realCheatPath.str().c_str(), 0777);
    // realCheatPath << "\\cheats\\";
    mkdir(realCheatPath.str().c_str(), 0777);
    realCheatPath << std::uppercase << buildIDStr.str() << ".txt";
    std::stringstream zipCheatPath;
    zipCheatPath << "titles/" << std::uppercase << std::hex << std::setfill('0') << std::setw(sizeof(u64) * 2) << titleID << "/cheats/" << std::uppercase << buildIDStr.str() << ".txt";
    // zipCheatPath << "titles\\" << std::uppercase << std::hex << std::setfill('0') << std::setw(sizeof(u64) * 2) << titleID << "\\cheats\\" << buildIDStr.str() << ".txt";
    // zipCheatPath << buildIDStr.str() << ".txt";
    zipper::Unzipper cheatzip(zipPath.str().c_str()); // cheatzip;
    if (!(access(realCheatPath.str().c_str(), F_OK) == 0) || Config::getConfig()->enablecheats || Config::getConfig()->easymode)
    {
      Config::getConfig()->enablecheats = false;
      std::stringstream realCheatPathold;
      realCheatPathold.str("");
      realCheatPathold << realCheatPath.str() << ".old";
      if ((access(realCheatPath.str().c_str(), F_OK) == 0))
      {
      REPLACEFILE(realCheatPath.str().c_str(),realCheatPathold.str().c_str());
      };
      if (cheatzip.extractEntry(zipCheatPath.str().c_str(), realCheatPath.str().c_str()))
      {
        if (!(m_debugger->m_dmnt))
        {
          m_debugger->detatch();
          dmntchtForceOpenCheatProcess();
          printf("force open called\n");
        }
        // else if (!Config::getConfig()->easymode)
        //   (new MessageBox("A new cheat has been added for this title from database. \n Please reload dmnt or restart the game.", MessageBox::OKAY))->show();
        else
        {
          FILE *fp1 = fopen(realCheatPath.str().c_str(), "rb");
          FILE *fp2 = fopen(realCheatPathold.str().c_str(), "rb");
          size_t fileSize1, fileSize2;
          fseek(fp1, 0, SEEK_END);
          fileSize1 = ftell(fp1);
          rewind(fp1);
          fseek(fp2, 0, SEEK_END);
          fileSize2 = ftell(fp2);
          rewind(fp2);
          bool different = false;
          // printf("fileSize1 = %ld fileSize2 = %ld\n",fileSize1,fileSize2);
          if (fileSize1 != fileSize2) different = true;
          else 
          {
            char *content1;
            char *content2;
            content1 = new char[fileSize1];
            fread(content1, 1, fileSize1, fp1);
            content2 = new char[fileSize2];
            fread(content2, 1, fileSize2, fp2);
            // printf("memcmp(&content1,&content2,fileSize1-1) = %d \n",memcmp(&content1,&content2,fileSize1));
            if (memcmp(content1,content2,fileSize1) !=0) different = true;
            fclose(fp1);
            fclose(fp2);
            delete content1;
            delete content2;
          };
          if (different)
            (new MessageBox("A new cheat has been added for this title from database. \n Please reload dmnt or restart the game.", MessageBox::OKAY))->show();
        }
      }
    }
  }//
  // else
  // {
  //   realCheatPath << "/atmosphere/contents/" << std::uppercase << std::hex << std::setfill('0') << std::setw(sizeof(u64) * 2) << titleID;
  //   realCheatPath << "/cheats/";
  //   realCheatPath << buildIDStr.str() << ".txt";
  // }
  // if (access(realCheatPath.str().c_str(), F_OK) == 0)
  // {
  //   REPLACEFILE(realCheatPath.str().c_str(), EdizonCheatPath.str().c_str());
  // }
  cheatpathStr = realCheatPath.str(); 
}

static bool _wrongCheatsPresent(u8 *buildID, u64 titleID)
{
  return false;
  std::stringstream ss;

  ss << "/atmosphere/contents/" << std::uppercase << std::hex << std::setfill('0') << std::setw(sizeof(u64) * 2) << titleID << "/cheats/";

  if (!std::filesystem::exists(ss.str()))
    return false;

  bool cheatsFolderEmpty = std::filesystem::is_empty(ss.str());

  for (u8 i = 0; i < 8; i++)
    ss << std::nouppercase << std::hex << std::setfill('0') << std::setw(2) << (u16)buildID[i];
  ss << ".txt";

  bool realCheatDoesExist = std::filesystem::exists(ss.str());

  return !(cheatsFolderEmpty || realCheatDoesExist);
}

bool GuiCheats::getinput(std::string headerText, std::string subHeaderText, std::string initialText, searchValue_t *searchValue)
{
  char str[0x21];
  Gui::requestKeyboardInput(headerText, subHeaderText, initialText, m_searchValueFormat == FORMAT_DEC ? SwkbdType_NumPad : SwkbdType_QWERTY, str, 0x20);
  if (std::string(str) == "")
    return false;
  (*searchValue)._u64 = 0;
  if (m_searchValueFormat == FORMAT_HEX)
  {
    (*searchValue)._u64 = static_cast<u64>(std::stoul(str, nullptr, 16));
  }
  else
  {
    switch (m_searchType)
    {
    case SEARCH_TYPE_UNSIGNED_8BIT:
      (*searchValue)._u8 = static_cast<u8>(std::stoul(str, nullptr, 0));
      break;
    case SEARCH_TYPE_UNSIGNED_16BIT:
      (*searchValue)._u16 = static_cast<u16>(std::stoul(str, nullptr, 0));
      break;
    case SEARCH_TYPE_UNSIGNED_32BIT:
      (*searchValue)._u32 = static_cast<u32>(std::stoul(str, nullptr, 0));
      break;
    case SEARCH_TYPE_UNSIGNED_64BIT:
      (*searchValue)._u64 = static_cast<u64>(std::stoul(str, nullptr, 0));
      break;
    case SEARCH_TYPE_SIGNED_8BIT:
      (*searchValue)._s8 = static_cast<s8>(std::stol(str, nullptr, 0));
      break;
    case SEARCH_TYPE_SIGNED_16BIT:
      (*searchValue)._s16 = static_cast<s16>(std::stol(str, nullptr, 0));
      break;
    case SEARCH_TYPE_SIGNED_32BIT:
      (*searchValue)._s32 = static_cast<s32>(std::stol(str, nullptr, 0));
      break;
    case SEARCH_TYPE_SIGNED_64BIT:
      (*searchValue)._s64 = static_cast<s64>(std::stol(str, nullptr, 0));
      break;
    case SEARCH_TYPE_FLOAT_32BIT:
      (*searchValue)._f32 = static_cast<float>(std::stof(str));
      break;
    case SEARCH_TYPE_FLOAT_64BIT:
      (*searchValue)._f64 = static_cast<double>(std::stod(str));
      break;
    case SEARCH_TYPE_POINTER:
      (*searchValue)._u64 = static_cast<u64>(std::stol(str));
      break;
    case SEARCH_TYPE_NONE:
      break;
    }
  }
  return true;
}
bool GuiCheats::valuematch(searchValue_t value, u64 nextaddress)
{
  searchValue_t realvalue;
  realvalue._u64 = 0;
  m_debugger->readMemory(&realvalue, dataTypeSizes[m_searchType], nextaddress);
  if (realvalue._u64 == value._u64)
    return true;
  else
    return false;
}
bool GuiCheats::addcodetofile(u64 index)
{
  std::stringstream buildIDStr;
  std::stringstream filebuildIDStr;
  {
    for (u8 i = 0; i < 8; i++)
      buildIDStr << std::nouppercase << std::hex << std::setfill('0') << std::setw(2) << (u16)m_buildID[i];
    // buildIDStr.str("attdumpbookmark");
    filebuildIDStr << EDIZON_DIR "/" << buildIDStr.str() << ".txt";
  }

  std::stringstream realCheatPath;
  {
    realCheatPath << "/atmosphere/contents/" << std::uppercase << std::hex << std::setfill('0') << std::setw(sizeof(u64) * 2) << m_debugger->getRunningApplicationTID();
    mkdir(realCheatPath.str().c_str(), 0777);
    realCheatPath << "/cheats/";
    mkdir(realCheatPath.str().c_str(), 0777);
    realCheatPath << buildIDStr.str() << ".txt";
  }

  bookmark_t bookmark;
  u64 address;
  m_AttributeDumpBookmark->getData(index * sizeof(bookmark_t), &bookmark, sizeof(bookmark_t));
  m_memoryDump->getData(index * sizeof(u64), &address, sizeof(u64));
  searchValue_t realvalue;
  realvalue._u64 = 0;
  m_debugger->readMemory(&realvalue, dataTypeSizes[bookmark.type], address);

  FILE *pfile;
  pfile = fopen(filebuildIDStr.str().c_str(), "a");
  std::stringstream ss;
  if (pfile != NULL)
  {
    // printf("going to write to file\n");
    ss.str("");
    ss << "[" << bookmark.label << "]"
       << "\n";
    if (bookmark.pointer.offset[bookmark.pointer.depth] > 0 && bookmark.pointer.offset[bookmark.pointer.depth] <= (s64)(m_mainend - m_mainBaseAddr))
      ss << ((m_32bitmode) ? "540F0000 " : "580F0000 ") << std::uppercase << std::hex << std::setfill('0') << std::setw(8) << bookmark.pointer.offset[bookmark.pointer.depth] << "\n";
    else
      ss << ((m_32bitmode) ? "541F0000 " : "581F0000 ") << std::uppercase << std::hex << std::setfill('0') << std::setw(8) << m_mainBaseAddr - m_heapBaseAddr + bookmark.pointer.offset[bookmark.pointer.depth] << "\n";

    for (int z = bookmark.pointer.depth - 1; z > 0; z--)
    {
      if (bookmark.pointer.offset[z] >= 0)
        ss << ((m_32bitmode) ? "540F1000 " : "580F1000 ") << std::uppercase << std::hex << std::setfill('0') << std::setw(8) << bookmark.pointer.offset[z] << "\n";
      else
      {
        ss << ((m_32bitmode) ? "740F1000 " : "780F1000 ") << std::uppercase << std::hex << std::setfill('0') << std::setw(8) << bookmark.pointer.offset[z]*(-1) << "\n";
        ss << ((m_32bitmode) ? "540F1000 00000000" : "580F1000 00000000");
      }
      
    }
    if (bookmark.pointer.offset[0] >= 0)
      ss << "780F0000 " << std::uppercase << std::hex << std::setfill('0') << std::setw(8) << bookmark.pointer.offset[0] << "\n";
    else
      ss << "780F1000 " << std::uppercase << std::hex << std::setfill('0') << std::setw(8) << bookmark.pointer.offset[0]*(-1) << "\n";

    ss << "6" << dataTypeSizes[bookmark.type] + 0 << "0F0000 " << std::uppercase << std::hex << std::setfill('0') << std::setw(16) << realvalue._u64 << "\n";
    printf("index = %ld depth = %ld offset = %ld offset = %ld offset = %ld offset = %ld\n", index, bookmark.pointer.depth, bookmark.pointer.offset[3], bookmark.pointer.offset[2], bookmark.pointer.offset[1], bookmark.pointer.offset[0]);
    printf("address = %lx value = %lx \n", address, realvalue._u64);
    printf("dataTypeSizes[bookmark.type] %d\n", dataTypeSizes[bookmark.type]);

    // std::uppercase << std::hex << std::setfill('0') << std::setw(10) << address;
    // ss << ",0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << realValue._u64;
    // snprintf(st, 27, "%s\n", ss.str().c_str()); char st[27];
    // fputs(filebuildIDStr.str().c_str(), pfile);
    // fputs("fopen example", pfile);
    //std::endl is basicly:
    //std::cout << "\n" << std::flush; "\r\n"
    // char st[1000];
    // snprintf(st, 1000, "%s\n", ss.str().c_str());
    fputs(ss.str().c_str(), pfile);
    fclose(pfile);
  }
  else
    printf("failed writing to cheat file on Edizon dir \n");

  pfile = fopen(realCheatPath.str().c_str(), "a");
  if (pfile != NULL)
  {
    fputs(ss.str().c_str(), pfile);
    fclose(pfile);
  }
  else
    printf("failed writing to cheat file on contents dir \n");

  return true;
}
bool GuiCheats::editcodefile() // not used work in progress
{
  std::stringstream buildIDStr;
  std::stringstream filebuildIDStr;
  {
    for (u8 i = 0; i < 8; i++)
      buildIDStr << std::nouppercase << std::hex << std::setfill('0') << std::setw(2) << (u16)m_buildID[i];
    filebuildIDStr << EDIZON_DIR "/" << buildIDStr.str() << ".txt";
  }

  std::stringstream realCheatPath;
  {
    realCheatPath << "/atmosphere/contents/" << std::uppercase << std::hex << std::setfill('0') << std::setw(sizeof(u64) * 2) << m_debugger->getRunningApplicationTID();
    realCheatPath << "/cheats/";
    realCheatPath << buildIDStr.str() << ".txt";
  }

  FILE *pfile;
  pfile = fopen(realCheatPath.str().c_str(), "r+b");
  fseek(pfile, 0, SEEK_END);
  u64 bufferSize = ftell(pfile);
  u8 *s = new u8[bufferSize + 1];
  /* Read cheats into buffer. */
  fseek(pfile, 0, SEEK_SET);
  fread(s, sizeof(bufferSize), 1, pfile);
  s[bufferSize] = '\x00';
  {
    size_t i = 0;
    while (i < bufferSize)
    {
      if (std::isspace(static_cast<unsigned char>(s[i])))
      {
        /* Just ignore whitespace. */
        i++;
      }
      else if (s[i] == '[')
      {
        size_t j = i + 1;
        while (s[j] != ']')
        {
          j++;
          if (j >= bufferSize)
          {
            return false;
          }
        }
      }
    }
  }
  delete [] s;
  // WIP
  pfile = fopen(filebuildIDStr.str().c_str(), "w+b");
  std::stringstream ss;
  if (pfile != NULL)
  {
    ss.str("");
    fputs(ss.str().c_str(), pfile);
    fclose(pfile);
  }
  else
    printf("failed writing to cheat file on Edizon dir \n");
  return true;
}

bool GuiCheats::reloadcheatsfromfile(u8 *buildID, u64 titleID)
{
  std::stringstream realCheatPath;
  std::stringstream buildIDStr;
  for (u8 i = 0; i < 8; i++)
    buildIDStr << std::nouppercase << std::hex << std::setfill('0') << std::setw(2) << (u16)buildID[i];
  realCheatPath << "/atmosphere/contents/" << std::uppercase << std::hex << std::setfill('0') << std::setw(sizeof(u64) * 2) << titleID;
  realCheatPath << "/cheats/";
  realCheatPath << buildIDStr.str() << ".txt";
  if (access(realCheatPath.str().c_str(), F_OK) == 0)
  {
    reloadcheats();     // reloaded from dmnt
    if (m_cheatCnt > 0) // clear the cheats
    {
      for (u32 i = 0; i < m_cheatCnt; i++)
      {
        dmntchtRemoveCheat(m_cheats[i].cheat_id);
      }
    }
    // read cheat file into buffer
    FILE *pfile;
    pfile = fopen(realCheatPath.str().c_str(), "r+b");
    fseek(pfile, 0, SEEK_END);
    u64 bufferSize = ftell(pfile);
    u8 *s = new u8[bufferSize + 1]; // watch out for memory leak
    /* Read cheats into buffer. */
    fseek(pfile, 0, SEEK_SET);
    fread(s, sizeof(bufferSize), 1, pfile);
    s[bufferSize] = '\x00';
    /* Parse cheat buffer. */
    // return this->ParseCheats(cht_txt, std::strlen(cht_txt));
    //bool CheatProcessManager::ParseCheats(const char *s, size_t len)
    {
      DmntCheatEntry cheat;
      /* Parse the input string. */
      size_t i = 0;
      cheat.definition.num_opcodes = 0;
      // CheatEntry *cur_entry = nullptr;
      while (i < bufferSize)
      {
        if (std::isspace(static_cast<unsigned char>(s[i])))
        {
          /* Just ignore whitespace. */
          i++;
        }
        else if (s[i] == '[')
        {
          if (cheat.definition.num_opcodes > 0)
          {
            if (dmntchtAddCheat(&(cheat.definition), false, &(cheat.cheat_id)))
            {
              cheat.definition.num_opcodes = 0;
            }
            else
            {
              printf("error adding cheat code\n");
              return false;
            }
          }
          /* Parse a readable cheat name. */
          // cur_entry = this->GetFreeCheatEntry();
          // if (cur_entry == nullptr)
          // {
          //   return false;
          // }
          /* Extract name bounds. */
          size_t j = i + 1;
          while (s[j] != ']')
          {
            j++;
            if (j >= bufferSize)
            {
              return false;
            }
          }
          /* s[i+1:j] is cheat name. */
          const size_t cheat_name_len = std::min(j - i - 1, sizeof(cheat.definition.readable_name));
          std::memcpy(cheat.definition.readable_name, &s[i + 1], cheat_name_len);
          cheat.definition.readable_name[cheat_name_len] = 0;
          /* Skip onwards. */
          i = j + 1;
        }
        else if (s[i] == '{')
        {
          /* We're parsing a master cheat. */
          // cur_entry = &this->cheat_entries[0];
          /* There can only be one master cheat. */
          // if (cur_entry->definition.num_opcodes > 0)
          // {
          //   return false;
          // }
          /* Extract name bounds */
          size_t j = i + 1;
          while (s[j] != '}')
          {
            j++;
            if (j >= bufferSize)
            {
              return false;
            }
          }
          /* s[i+1:j] is cheat name. */
          const size_t cheat_name_len = std::min(j - i - 1, sizeof(cheat.definition.readable_name));
          memcpy(cheat.definition.readable_name, &s[i + 1], cheat_name_len);
          cheat.definition.readable_name[cheat_name_len] = 0;
          /* Skip onwards. */
          i = j + 1;
        }
        else if (std::isxdigit(static_cast<unsigned char>(s[i])))
        {
          /* Make sure that we have a cheat open. */
          // if (cur_entry == nullptr)
          // {
          //   return false;
          // }
          /* Bounds check the opcode count. */
          // if (cur_entry->definition.num_opcodes >= util::size(cur_entry->definition.opcodes))
          // {
          //   return false;
          // }
          /* We're parsing an instruction, so validate it's 8 hex digits. */
          for (size_t j = 1; j < 8; j++)
          {
            /* Validate 8 hex chars. */
            if (i + j >= bufferSize || !std::isxdigit(static_cast<unsigned char>(s[i + j])))
            {
              return false;
            }
          }
          /* Parse the new opcode. */
          char hex_str[9] = {0};
          std::memcpy(hex_str, &s[i], 8);
          cheat.definition.opcodes[cheat.definition.num_opcodes++] = std::strtoul(hex_str, NULL, 16);
          /* Skip onwards. */
          i += 8;
        }
        else
        {
          /* Unexpected character encountered. */
          return false;
        }
      }
      if (cheat.definition.num_opcodes > 0)
      {
        if (dmntchtAddCheat(&(cheat.definition), false, &(cheat.cheat_id)))
        {
          cheat.definition.num_opcodes = 0;
        }
        else
        {
          printf("error adding cheat code\n");
          return false;
        }
      }
    }
    //
    reloadcheats();
  }
  return true;
}
void GuiCheats::reloadcheats()
{
  if (m_cheats != nullptr)
    delete m_cheats;
  if (m_cheatDelete != nullptr)
    delete m_cheatDelete;
  dmntchtGetCheatCount(&m_cheatCnt);
  if (m_cheatCnt > 0)
  {
    m_cheats = new DmntCheatEntry[m_cheatCnt];
    m_cheatDelete = new bool[m_cheatCnt];
    for (u64 i = 0; i < m_cheatCnt; i++)
      m_cheatDelete[i] = false;
    dmntchtGetCheats(m_cheats, m_cheatCnt, 0, &m_cheatCnt);
  }
}
void GuiCheats::iconloadcheck()
{
  std::stringstream filenoiconStr;
  filenoiconStr << EDIZON_DIR "/noicon.txt";
  if (access(filenoiconStr.str().c_str(), F_OK) == 0)
  {
    m_havesave = false;
  }
}
void GuiCheats::removef(std::string filePath)
{
  filePath.replace(0, sizeof(EDIZON_DIR)-1, m_edizon_dir);
  remove(filePath.c_str());
}
void GuiCheats::renamef(std::string filePath1,std::string filePath2)
{
  filePath1.replace(0, sizeof(EDIZON_DIR)-1, m_edizon_dir);
  rename(filePath1.c_str(),filePath2.c_str());
}
bool GuiCheats::freeze()
{
  Config::readConfig();
  if (Config::getConfig()->freeze)
  {
    m_debugger->pause();
    return true;
  }
  else
    return false;
}
bool GuiCheats::unfreeze()
{
  Config::readConfig();
  if (Config::getConfig()->freeze)
  {
    m_debugger->resume();
    return true;
  }
  else
    return false;
}
bool GuiCheats::autoattachcheck()
{
  Config::readConfig();
  if (Config::getConfig()->options[0] == 0)
  {
    if (m_debugger->m_dmnt)
      return dmntchtForceOpenCheatProcess();
  }

  return false;
  // std::stringstream filenoiconStr;
  // filenoiconStr << EDIZON_DIR "/noautoattach.txt";
  // if (access(filenoiconStr.str().c_str(), F_OK) != 0)
  // {
  //   if (m_debugger->m_dmnt)
  //   dmntchtForceOpenCheatProcess();
  //   return true;
  // }
  // else
  //   return false;
  // testlz();
}
bool GuiCheats::autoexitcheck()
{
  Config::readConfig();
  if (Config::getConfig()->options[1] == 0)
  {
    return true;
  }
  else
    return false;
  // std::stringstream filenoiconStr;
  // filenoiconStr << EDIZON_DIR "/noautoexit.txt";
  // if (access(filenoiconStr.str().c_str(), F_OK) != 0)
  // {
  //   return true;
  // }
  // else
  //   return false;
}
void GuiCheats::testlz()
{
  time_t unixTime1 = time(NULL);
  std::stringstream filenoiconStr;
  filenoiconStr << EDIZON_DIR "/ff756020d95b3ec5.dmp2";
  MemoryDump *PCDump,*PCDump2;
  u64 bufferSize = 0x1000000;
  u8 *buffer = new u8[bufferSize];
  u8 *outbuffer = new u8[bufferSize + 0x50000];
  PCDump = new MemoryDump(filenoiconStr.str().c_str(), DumpType::DATA, false);
  filenoiconStr << "a";
  PCDump2 = new MemoryDump(filenoiconStr.str().c_str(), DumpType::DATA, true);
  u64 S = PCDump->size();
  u64 total = 0;
  for (u64 index = 0; index < S;)
  {
    if ((S - index) < bufferSize)
      bufferSize = S - index;
    PCDump->getData(index, buffer, bufferSize);
    printf("Start LZ \n");
    u64 count = LZ_Compress(buffer, outbuffer, bufferSize);
    PCDump2->addData((u8*)&count, sizeof(count));
    PCDump2->addData(outbuffer, count);

    float r = (float)count / (float)bufferSize;
    printf("Index = %lx , End LZ bufferSize = %lx , outsize = %lx , ration = %f\n",index, bufferSize, count, r);
    index += bufferSize;
    total +=count;
  }
  delete buffer;
  delete outbuffer;
  time_t unixTime2 = time(NULL);
  printf("%s%ld\n", "Stop Time ", unixTime2 - unixTime1);
  float r = (float) total / (float) S;
  printf("Size = %lx , outsize = %lx , ration = %f\n", S, total, r);
  delete PCDump;
  PCDump2->flushBuffer();
  delete PCDump2;
}
bool GuiCheats::dumpcodetofile()
{
  std::stringstream buildIDStr;
  std::stringstream filebuildIDStr;
  {
    for (u8 i = 0; i < 8; i++)
      buildIDStr << std::nouppercase << std::hex << std::setfill('0') << std::setw(2) << (u16)m_buildID[i];
    filebuildIDStr << EDIZON_DIR "/" << buildIDStr.str() << ".txt";
  }
  std::stringstream realCheatPath;
  {
    realCheatPath << "/atmosphere/contents/" << std::uppercase << std::hex << std::setfill('0') << std::setw(sizeof(u64) * 2) << m_debugger->getRunningApplicationTID();
    mkdir(realCheatPath.str().c_str(), 0777);
    realCheatPath << "/cheats/";
    mkdir(realCheatPath.str().c_str(), 0777);
    realCheatPath << buildIDStr.str() << ".txt";
  }
  FILE *pfile;
  pfile = fopen(filebuildIDStr.str().c_str(), "w");
  std::stringstream SS;
  std::stringstream ss;
  if (pfile != NULL)
  {
    // GuiCheats::reloadcheats();
    SS.str("");
    for (u32 i = 0; i < m_cheatCnt; i++)
    {
      SS << "[" << m_cheats[i].definition.readable_name << "]\n";
      ss.str("");
      for (u32 j = 0; j < m_cheats[i].definition.num_opcodes; j++)
      {
        u16 opcode = (m_cheats[i].definition.opcodes[j] >> 28) & 0xF;
        u8 T = (m_cheats[i].definition.opcodes[j] >> 24) & 0xF;
        if ((opcode == 9) && (((m_cheats[i].definition.opcodes[j] >> 8) & 0xF) == 0))
        {
          ss << std::uppercase << std::hex << std::setfill('0') << std::setw(8) << m_cheats[i].definition.opcodes[j] << "\n";
          continue;
        }
        if (opcode == 0xC)
        {
          opcode = (m_cheats[i].definition.opcodes[j] >> 24) & 0xFF;
          T = (m_cheats[i].definition.opcodes[j] >> 20) & 0xF;
          u8 X = (m_cheats[i].definition.opcodes[j] >> 8) & 0xF;
          if (opcode == 0xC0)
          {
            opcode = opcode * 16 + X;
          }
        }
        if (opcode == 10)
        {
          u8 O = (m_cheats[i].definition.opcodes[j] >> 8) & 0xF;
          if (O == 2 || O == 4 || O == 5)
            T = 8;
          else
            T = 4;
        }
        switch (opcode)
        {
        case 0:
        case 1:
          ss << std::uppercase << std::hex << std::setfill('0') << std::setw(8) << m_cheats[i].definition.opcodes[j++] << " ";
          // 3+1
        case 9:
        case 0xC04:
          // 2+1
          ss << std::uppercase << std::hex << std::setfill('0') << std::setw(8) << m_cheats[i].definition.opcodes[j++] << " ";
        case 3:
        case 10:
          // 1+1
          ss << std::uppercase << std::hex << std::setfill('0') << std::setw(8) << m_cheats[i].definition.opcodes[j] << " ";
          if (T == 8 || (T == 0 && opcode == 3))
          {
            j++;
            ss << std::uppercase << std::hex << std::setfill('0') << std::setw(8) << m_cheats[i].definition.opcodes[j] << " ";
          }
          break;
        case 4:
        case 6:
          // 3
          ss << std::uppercase << std::hex << std::setfill('0') << std::setw(8) << m_cheats[i].definition.opcodes[j++] << " ";
        case 5:
        case 7:
        case 0xC00:
        case 0xC02:
          ss << std::uppercase << std::hex << std::setfill('0') << std::setw(8) << m_cheats[i].definition.opcodes[j++] << " ";
          // 2
        case 2:
        case 8:
        case 0xC1:
        case 0xC2:
        case 0xC3:
        case 0xC01:
        case 0xC03:
        case 0xC05:
        default:
          ss << std::uppercase << std::hex << std::setfill('0') << std::setw(8) << m_cheats[i].definition.opcodes[j] << " ";
          // 1
          break;
        }
        if (j >= (m_cheats[i].definition.num_opcodes)) // better to be ugly than to corrupt
        {
          printf("error encountered in addcodetofile \n ");
          ss.str("");
          for (u32 k = 0; k < m_cheats[i].definition.num_opcodes; k++)
          {
            ss << std::uppercase << std::hex << std::setfill('0') << std::setw(8) << m_cheats[i].definition.opcodes[k++] << " ";
          }
          ss << "\n";
          break;
        }
        ss << "\n";
      }
      SS << ss.str().c_str() << "\n";
    }
    // DmntCheatDefinition cheat = m_cheats[m_selectedEntry].definition;
    // memcpy(&bookmark.label, &cheat.readable_name, sizeof(bookmark.label));
    //    << "\n";
    // ss << "580F0000 " << std::uppercase << std::hex << std::setfill('0') << std::setw(8) << bookmark.pointer.offset[bookmark.pointer.depth] << "\n";
    // for (int z = bookmark.pointer.depth - 1; z > 0; z--)
    // {
    //   ss << "580F1000 " << std::uppercase << std::hex << std::setfill('0') << std::setw(8) << bookmark.pointer.offset[z] << "\n";
    // }
    // ss << "780F0000 " << std::uppercase << std::hex << std::setfill('0') << std::setw(8) << bookmark.pointer.offset[0] << "\n";
    // ss << "6" << dataTypeSizes[bookmark.type] + 0 << "0F0000 " << std::uppercase << std::hex << std::setfill('0') << std::setw(16) << realvalue._u64 << "\n";

    fputs(SS.str().c_str(), pfile);
    fclose(pfile);
  }
  else
    printf("failed writing to cheat file on Edizon dir \n");

  pfile = fopen(realCheatPath.str().c_str(), "w");
  if (pfile != NULL)
  {
    fputs(SS.str().c_str(), pfile);
    fclose(pfile);
  }
  else
    printf("failed writing to cheat file on contents dir \n");
  return true;
}

void GuiCheats::PSsaveSTATE()
{
  PSsetup_t save;
  save.m_numoffset = m_numoffset;
  save.m_max_source = m_max_source;
  save.m_max_depth = m_max_depth;
  save.m_max_range = m_max_range;
  save.m_EditorBaseAddr = m_EditorBaseAddr;
  save.m_mainBaseAddr = m_mainBaseAddr;
  save.m_mainend = m_mainend;
  save.m_pointersearch_canresume = m_pointersearch_canresume;
  save.m_PS_resume = m_PS_resume;
  save.m_PS_pause = m_PS_pause;
  MemoryDump *PSdump;
  PSdump = new MemoryDump(EDIZON_DIR "/PSstatedump.dat", DumpType::UNDEFINED, true);
  PSdump->setBaseAddresses(m_addressSpaceBaseAddr, m_heapBaseAddr, m_mainBaseAddr, m_heapSize, m_mainSize);
  PSdump->addData((u8 *)&save, sizeof(PSsetup_t));
  if (m_PointerSearch != nullptr)
    PSdump->addData((u8 *)m_PointerSearch, sizeof(PointerSearch_state));
  PSdump->flushBuffer();
  delete PSdump;
  printf("done saving PSstate\n");
  // PointerSearch_state *m_PointerSearch = nullptr;
  //if (PSdump->size() > 0)
}

void GuiCheats::PSresumeSTATE()
{
  PSsetup_t save;
  MemoryDump *PSdump;
  PSdump = new MemoryDump(EDIZON_DIR "/PSstatedump.dat", DumpType::UNDEFINED, false);
  if (PSdump->size() > 0 && PSdump->getDumpInfo().heapBaseAddress == m_heapBaseAddr)
  {
    PSdump->getData(0, &save, sizeof(PSsetup_t));
    if (PSdump->size() == sizeof(PSsetup_t) + sizeof(PointerSearch_state))
    {
      if (m_PointerSearch == nullptr)
        m_PointerSearch = new PointerSearch_state;
      PSdump->getData(sizeof(PSsetup_t), m_PointerSearch, sizeof(PointerSearch_state));
    }
    delete PSdump;
    m_numoffset = save.m_numoffset;
    m_max_source = save.m_max_source;
    m_max_depth = save.m_max_depth;
    m_max_range = save.m_max_range;
    m_mainBaseAddr = save.m_mainBaseAddr;
    m_mainend = save.m_mainend;
    m_pointersearch_canresume = save.m_pointersearch_canresume;
    m_PS_resume = save.m_PS_resume;
    m_PS_pause = save.m_PS_pause;
    if (m_pointersearch_canresume)
      m_EditorBaseAddr = save.m_EditorBaseAddr;
  }
}

bool GuiCheats::addstaticcodetofile(u64 index)
{
  std::stringstream buildIDStr;
  std::stringstream filebuildIDStr;
  {
    for (u8 i = 0; i < 8; i++)
      buildIDStr << std::nouppercase << std::hex << std::setfill('0') << std::setw(2) << (u16)m_buildID[i];
    // buildIDStr.str("attdumpbookmark");
    filebuildIDStr << EDIZON_DIR "/" << buildIDStr.str() << ".txt";
  }

  std::stringstream realCheatPath;
  {
    realCheatPath << "/atmosphere/contents/" << std::uppercase << std::hex << std::setfill('0') << std::setw(sizeof(u64) * 2) << m_debugger->getRunningApplicationTID();
    mkdir(realCheatPath.str().c_str(), 0777);
    realCheatPath << "/cheats/";
    mkdir(realCheatPath.str().c_str(), 0777);
    realCheatPath << buildIDStr.str() << ".txt";
  }

  bookmark_t bookmark;
  u64 address;
  m_AttributeDumpBookmark->getData(index * sizeof(bookmark_t), &bookmark, sizeof(bookmark_t));
  m_memoryDump->getData(index * sizeof(u64), &address, sizeof(u64));
  searchValue_t realvalue;
  realvalue._u64 = 0;
  m_debugger->readMemory(&realvalue, dataTypeSizes[bookmark.type], address);

  FILE *pfile;
  pfile = fopen(filebuildIDStr.str().c_str(), "a");
  std::stringstream ss;
  if (pfile != NULL)
  {
    // printf("going to write to file\n");
    ss.str("");
    ss << "[" << bookmark.label << "]"
       << "\n";
    ss << "0" << dataTypeSizes[bookmark.type] + 0 << (bookmark.heap ? 1 : 0) << "00000 " << std::uppercase << std::hex << std::setfill('0') << std::setw(8) << bookmark.offset << " "
       << std::uppercase << std::hex << std::setfill('0') << ((dataTypeSizes[bookmark.type] == 8) ? std::setw(16) : std::setw(8))
       << ((dataTypeSizes[bookmark.type] == 8) ? realvalue._u64 : realvalue._u32) << "\n";
    printf("index = %ld depth = %ld offset = %ld offset = %ld offset = %ld offset = %ld\n", index, bookmark.pointer.depth, bookmark.pointer.offset[3], bookmark.pointer.offset[2], bookmark.pointer.offset[1], bookmark.pointer.offset[0]);
    printf("address = %lx value = %lx \n", address, realvalue._u64);
    printf("dataTypeSizes[bookmark.type] %d\n", dataTypeSizes[bookmark.type]);
    fputs(ss.str().c_str(), pfile);
    fclose(pfile);
  }
  else
    printf("failed writing to cheat file on Edizon dir \n");

  pfile = fopen(realCheatPath.str().c_str(), "a");
  if (pfile != NULL)
  {
    fputs(ss.str().c_str(), pfile);
    fclose(pfile);
  }
  else
    printf("failed writing to cheat file on contents dir \n");

  return true;
}
void GuiCheats::PCdump()
{
  bool ledOn = true;
  u8 j = 1;
  while (access(m_PCDump_filename.str().c_str(), F_OK) == 0)
  {
    m_PCDump_filename.seekp(-1, std::ios_base::end);
    m_PCDump_filename << (0 + j++);
    printf("%s\n", m_PCDump_filename.str().c_str());
  }
  MemoryDump *PCDump;
  PCDump = new MemoryDump(m_PCDump_filename.str().c_str(), DumpType::DATA, true);
  PCDump->addData((u8 *)&m_EditorBaseAddr, sizeof(u64)); // first entry is the target address
  PCDump->addData((u8 *)&m_mainBaseAddr, sizeof(u64));
  PCDump->addData((u8 *)&m_mainend, sizeof(u64));
  for (MemoryInfo meminfo : m_memoryInfo)
  {
    if (meminfo.type != MemType_Heap && meminfo.type != MemType_CodeWritable && meminfo.type != MemType_CodeMutable)
      continue;
    setLedState(ledOn);
    ledOn = !ledOn;
    u64 offset = 0;
    u64 bufferSize = MAX_BUFFER_SIZE; // consider to increase from 10k to 1M (not a big problem)
    u8 *buffer = new u8[bufferSize];
    while (offset < meminfo.size)
    {
      if (meminfo.size - offset < bufferSize)
        bufferSize = meminfo.size - offset;
      m_debugger->readMemory(buffer, bufferSize, meminfo.addr + offset);
      searchValue_t realValue = {0};
      for (u32 i = 0; i < bufferSize; i += sizeof(u64))
      {
        u64 address = meminfo.addr + offset + i;
        memset(&realValue, 0, 8);
        memcpy(&realValue, buffer + i, sizeof(u64));
        if (((realValue._u64 >= m_mainBaseAddr) && (realValue._u64 <= (m_mainend))) || ((realValue._u64 >= m_heapBaseAddr) && (realValue._u64 <= (m_heapEnd))))
        {
          PCDump->addData((u8 *)&address, sizeof(u64));
          PCDump->addData((u8 *)&realValue, sizeof(u64));
        }
      }
    }
    offset += bufferSize;
    delete[] buffer;
  }
  PCDump->flushBuffer();
  delete PCDump;
  setLedState(false);
}

void GuiCheats::searchMemoryAddressesPrimary2(Debugger *debugger, searchValue_t searchValue1, searchValue_t searchValue2, searchType_t searchType, searchMode_t searchMode, searchRegion_t searchRegion, MemoryDump **displayDump, std::vector<MemoryInfo> memInfos)
{
  u8 j = 1;
  int k = -1;
  while (access(m_PCDump_filename.str().c_str(), F_OK) == 0)
  {
    m_PCDump_filename.seekp(k, std::ios_base::end);
    m_PCDump_filename << (0 + j++);
    printf("%s\n", m_PCDump_filename.str().c_str());
    if (j > 10)
      k = -2;
    if (j > 100)
      k = -3;
  }
  if (j == 1)
    j++;
  std::stringstream m_PCAttr_filename;
  m_PCAttr_filename << m_PCDump_filename.str().c_str();
  m_PCAttr_filename.seekp(k - 3, std::ios_base::end);
  m_PCAttr_filename << "att" << (j - 1);

  //MemoryDump *newstringDump = new MemoryDump(EDIZON_DIR "/stringdump.csv", DumpType::DATA, true);
  MemoryDump *PCDump;
  PCDump = new MemoryDump(m_PCDump_filename.str().c_str(), DumpType::DATA, true);
  MemoryDump *PCAttr;
  PCAttr = new MemoryDump(m_PCAttr_filename.str().c_str(), DumpType::DATA, true);
  PCDump->addData((u8 *)&m_mainBaseAddr, sizeof(u64));
  PCDump->addData((u8 *)&m_mainend, sizeof(u64));
  PCDump->addData((u8 *)&m_heapBaseAddr, sizeof(u64));
  PCDump->addData((u8 *)&m_heapEnd, sizeof(u64));
  PCDump->addData((u8 *)&m_EditorBaseAddr, sizeof(u64)); // first entry is the target address
  PCDump->flushBuffer();
  PCDump->m_compress = true;
  bool ledOn = false;

  time_t unixTime1 = time(NULL);
  printf("%s%lx\n", "Start Time primary search", unixTime1);
  dmntchtPauseCheatProcess();
  // printf("main %lx main end %lx heap %lx heap end %lx \n",m_mainBaseAddr, m_mainBaseAddr+m_mainSize, m_heapBaseAddr, m_heapBaseAddr+m_heapSize);
  for (MemoryInfo meminfo : memInfos)
  {

    // printf("%s%p", "meminfo.addr, ", meminfo.addr);
    // printf("%s%p", ", meminfo.end, ", meminfo.addr + meminfo.size);
    // printf("%s%p", ", meminfo.size, ", meminfo.size);
    // printf("%s%lx", ", meminfo.type, ", meminfo.type);
    // printf("%s%lx", ", meminfo.attr, ", meminfo.attr);
    // printf("%s%lx", ", meminfo.perm, ", meminfo.perm);
    // printf("%s%lx", ", meminfo.device_refcount, ", meminfo.device_refcount);
    // printf("%s%lx\n", ", meminfo.ipc_refcount, ", meminfo.ipc_refcount);

    if (searchRegion == SEARCH_REGION_HEAP && meminfo.type != MemType_Heap)
      continue;
    else if (searchRegion == SEARCH_REGION_MAIN &&
             ((meminfo.type != MemType_CodeWritable && meminfo.type != MemType_CodeMutable) || (meminfo.addr >= m_mainend) || (meminfo.addr < m_mainBaseAddr)))
      continue;
    else if (searchRegion == SEARCH_REGION_HEAP_AND_MAIN &&
             (((meminfo.type != MemType_Heap && meminfo.type != MemType_CodeWritable && meminfo.type != MemType_CodeMutable)) || !((meminfo.addr < m_heapEnd && meminfo.addr >= m_heapBaseAddr) || (meminfo.addr < m_mainend && meminfo.addr >= m_mainBaseAddr))))
      //  (meminfo.type != MemType_Heap && meminfo.type != MemType_CodeWritable && meminfo.type != MemType_CodeMutable))
      continue;
    else if (searchRegion == SEARCH_REGION_RAM && (meminfo.perm & Perm_Rw) != Perm_Rw)
      continue;

    setLedState(ledOn);
    ledOn = !ledOn;
    printf("meminfo.addr,%lx,meminfo.size,%lx,meminfo.type,%d,", meminfo.addr, meminfo.size, meminfo.type);
    PCAttr->addData((u8 *)&meminfo, sizeof(MemoryInfo));
    u64 counting_pointers = 0;
    u64 offset = 0;
    u64 bufferSize = MAX_BUFFER_SIZE; // consider to increase from 10k to 1M (not a big problem)
    u8 *buffer = new u8[bufferSize];
    while (offset < meminfo.size)
    {

      if (meminfo.size - offset < bufferSize)
        bufferSize = meminfo.size - offset;

      debugger->readMemory(buffer, bufferSize, meminfo.addr + offset);

      searchValue_t realValue = {0};
      for (u32 i = 0; i < bufferSize; i += 4)
      {
        u64 address = meminfo.addr + offset + i;
        memset(&realValue, 0, 8);
        if (m_32bitmode)
          memcpy(&realValue, buffer + i, 4); //dataTypeSizes[searchType]);
        else
          memcpy(&realValue, buffer + i, dataTypeSizes[searchType]);
        if (realValue._u64 != 0)
          if (((realValue._u64 >= m_mainBaseAddr) && (realValue._u64 <= (m_mainend))) || ((realValue._u64 >= m_heapBaseAddr) && (realValue._u64 <= (m_heapEnd))))
          // if ((realValue._u64 >= m_heapBaseAddr) && (realValue._u64 <= m_heapEnd))
          {
            if ((m_forwarddump) && (address > realValue._u64) && (meminfo.type == MemType_Heap))
              break;
            // (*displayDump)->addData((u8 *)&address, sizeof(u64));
            // newdataDump->addData((u8 *)&realValue, sizeof(u64));
            // helperinfo.count++;

            // realValue._u64 = realValue._u64 - m_heapBaseAddr;
            // MemoryType fromtype;
            // if (meminfo.type == MemType_Heap)
            // {
            //   address = address - m_heapBaseAddr;
            //   fromtype = HEAP;
            // }
            // else
            // {
            //   address = address - m_mainBaseAddr;
            //   fromtype = MAIN;
            // }
            // PCDump->addData((u8 *)&fromtype, sizeof(fromtype));

            PCDump->addData((u8 *)&address, sizeof(u64));
            PCDump->addData((u8 *)&realValue, sizeof(u64));
            counting_pointers++;
            // printf("0x%lx,0x%lx\n",address,realValue);
            // std::stringstream ss; // replace the printf
            // ss.str("");
            // ss << "0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << address;
            // ss << ",0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(10) << realValue._u64;
            // char st[27];
            // snprintf(st, 27, "%s\n", ss.str().c_str());    //
            // newstringDump->addData((u8 *)&st, sizeof(st)); //
          }
      }

      offset += bufferSize;
    }
    printf("count,%lx\n", counting_pointers);
    PCAttr->addData((u8 *)&counting_pointers, sizeof(counting_pointers));
    delete[] buffer;
  }

  setLedState(false);

  time_t unixTime2 = time(NULL);
  printf("%s%lx\n", "Stop Time ", unixTime2);
  printf("%s%ld\n", "Stop Time ", unixTime2 - unixTime1);
  if (PCDump->m_compress)
    printf("mcompress = true\n");
  PCDump->flushBuffer();
  delete PCDump;
  PCAttr->flushBuffer();
  delete PCAttr;
  dmntchtResumeCheatProcess();
  // delete newstringDump;
}
//
// #define inc_prep
void GuiCheats::prep_pointersearch(Debugger *debugger, std::vector<MemoryInfo> memInfos)
{
#ifdef inc_prep
  u8 j = 1;
  int k = -1;
  while (access(m_PCDump_filename.str().c_str(), F_OK) == 0)
  {
    m_PCDump_filename.seekp(k, std::ios_base::end);
    m_PCDump_filename << (0 + j++);
    printf("%s\n", m_PCDump_filename.str().c_str());
    if (j > 10)
      k = -2;
    if (j > 100)
      k = -3;
  }
  // if (j == 1)
  //   j++;
  // std::stringstream m_PCAttr_filename;
  // m_PCAttr_filename << m_PCDump_filename.str().c_str();
  // m_PCAttr_filename.seekp(k - 3, std::ios_base::end);
  // m_PCAttr_filename << "att" << (j - 1);
#endif
  if ((m_PC_Dump != nullptr) && !m_redo_prep_pointersearch)
  {
    // refresh_fromto();
    return;
  };
  m_PCAttr_filename.str("");
  m_PCAttr_filename << m_PCDump_filename.str().c_str() << "A";
  m_PCDumpM_filename.str("");
  m_PCDumpM_filename << m_PCDump_filename.str().c_str() << "M";

  MemoryDump *PCDump;
  // check if data is already for use
  if (access(m_PCDump_filename.str().c_str(), F_OK) == 0)
  {
    PCDump = new MemoryDump(m_PCDump_filename.str().c_str(), DumpType::DATA, false);
    if ((PCDump->getDumpInfo().heapBaseAddress == m_heapBaseAddr) && !m_redo_prep_pointersearch)
    {
      m_PC_Dump = PCDump;
      m_PC_DumpM = new MemoryDump(m_PCDumpM_filename.str().c_str(), DumpType::DATA, false);
      refresh_fromto();
      return;
    };
    delete PCDump;
  }
  if (m_redo_prep_pointersearch)
  {
    m_redo_prep_pointersearch = false;
    delete m_PC_Dump;
    delete m_PC_DumpM;
  };
  (new MessageBox("Preparing JumpBack data.\n \nThis may take a while...", MessageBox::NONE))->show();
  requestDraw();

  PCDump = new MemoryDump(m_PCDump_filename.str().c_str(), DumpType::DATA, true);
  PCDump->setBaseAddresses(m_addressSpaceBaseAddr, m_heapBaseAddr, m_mainBaseAddr, m_heapSize, m_mainSize);
  MemoryDump *PCDumpM;
  PCDumpM = new MemoryDump(m_PCDumpM_filename.str().c_str(), DumpType::DATA, true);
  MemoryDump *PCAttr;
  PCAttr = new MemoryDump(m_PCAttr_filename.str().c_str(), DumpType::DATA, true);
  // PCDump->addData((u8 *)&m_mainBaseAddr, sizeof(u64));
  // PCDump->addData((u8 *)&m_mainend, sizeof(u64));
  // PCDump->addData((u8 *)&m_heapBaseAddr, sizeof(u64));
  // PCDump->addData((u8 *)&m_heapEnd, sizeof(u64));
  // PCDump->addData((u8 *)&m_EditorBaseAddr, sizeof(u64)); // first entry is the target address
  // PCDump->flushBuffer();
  PCDump->m_compress = false;
  u32 data_inc;
  if (m_64bit_offset)
    data_inc = 8;
  else
    data_inc = 4;
  bool ledOn = false;
  time_t unixTime1 = time(NULL);
  printf("%s%lx\n", "Start Time primary search", unixTime1);
  dmntchtPauseCheatProcess();
  // printf("main %lx main end %lx heap %lx heap end %lx \n",m_mainBaseAddr, m_mainBaseAddr+m_mainSize, m_heapBaseAddr, m_heapBaseAddr+m_heapSize);
  for (MemoryInfo meminfo : memInfos)
  {
    if (((meminfo.type != MemType_Heap && meminfo.type != MemType_CodeWritable && meminfo.type != MemType_CodeMutable)) || !((meminfo.addr < m_heapEnd && meminfo.addr >= m_heapBaseAddr) || (meminfo.addr < m_mainend && meminfo.addr >= m_mainBaseAddr)))
      continue;
    setLedState(ledOn);
    ledOn = !ledOn;
    printf("meminfo.addr,%lx,meminfo.size,%lx,meminfo.type,%d,", meminfo.addr, meminfo.size, meminfo.type);
    PCAttr->addData((u8 *)&meminfo, sizeof(MemoryInfo));
    u64 counting_pointers = 0;
    u64 offset = 0;
    u64 bufferSize = MAX_BUFFER_SIZE; // consider to increase from 10k to 1M (not a big problem)
    u8 *buffer = new u8[bufferSize];
    if (meminfo.addr < m_mainend && meminfo.addr >= m_mainBaseAddr)
    {
      while (offset < meminfo.size)
      {
        if (meminfo.size - offset < bufferSize)
          bufferSize = meminfo.size - offset;
        debugger->readMemory(buffer, bufferSize, meminfo.addr + offset);
        searchValue_t realValue = {0};
        for (u64 i = 0; i < bufferSize; i += 4)
        {
          u64 address = meminfo.addr + offset + i - m_mainBaseAddr;
          memset(&realValue, 0, 8);
          if (m_32bitmode)
            memcpy(&realValue, buffer + i, 4); //dataTypeSizes[searchType]);
          else
            memcpy(&realValue, buffer + i, 8);
          if (realValue._u64 != 0)
            if (((realValue._u64 >= m_heapBaseAddr) && (realValue._u64 <= (m_heapEnd))))
            {
              realValue._u64 = realValue._u64 - m_heapBaseAddr;
              PCDumpM->addData((u8 *)&address, data_inc);
              PCDumpM->addData((u8 *)&realValue, data_inc);
              // address = 0;
              // PCDump->addData((u8 *)&address, data_inc);
              // PCDump->addData((u8 *)&realValue, data_inc);
              counting_pointers++;
            }
        }
        offset += bufferSize;
      }
    }
    else
    {
      while (offset < meminfo.size)
      {
        if (meminfo.size - offset < bufferSize)
          bufferSize = meminfo.size - offset;
        debugger->readMemory(buffer, bufferSize, meminfo.addr + offset);
        searchValue_t realValue = {0};
        for (u64 i = 0; i < bufferSize; i += 4)
        {
          u64 address = meminfo.addr + offset + i - m_heapBaseAddr;
          memset(&realValue, 0, 8);
          if (m_32bitmode)
            memcpy(&realValue, buffer + i, 4); //dataTypeSizes[searchType]);
          else
            memcpy(&realValue, buffer + i, 8);
          if (realValue._u64 != 0)
            if (((realValue._u64 >= m_heapBaseAddr) && (realValue._u64 <= (m_heapEnd))))
            {
              realValue._u64 = realValue._u64 - m_heapBaseAddr;
              PCDump->addData((u8 *)&address, data_inc);
              PCDump->addData((u8 *)&realValue, data_inc);
              counting_pointers++;
            }
        }
        offset += bufferSize;
      }
    }
    printf("count,%lx\n", counting_pointers);
    PCAttr->addData((u8 *)&counting_pointers, sizeof(counting_pointers));
    delete[] buffer;
  };
  setLedState(false);
  time_t unixTime2 = time(NULL);
  printf("%s%lx\n", "Stop Time ", unixTime2);
  printf("%s%ld\n", "Stop Time ", unixTime2 - unixTime1);
  if (PCDump->m_compress)
    printf("mcompress = true\n");
  PCDump->flushBuffer();
  // delete PCDump;
  m_PC_Dump = PCDump;
  PCDumpM->flushBuffer();
  // delete PCDumpM;
  m_PC_DumpM = PCDumpM;
  PCAttr->flushBuffer();
  delete PCAttr;
  dmntchtResumeCheatProcess();
  Gui::g_currMessageBox->hide();
}


u64 GuiCheats::get_main_offset32(u64 address)
{
  static u64 last_address = 0, last_count = 0;
  bool found_next_count = false;
  if (address != last_address)
  {
    last_address = address;
    last_count = 0;
  }
  u64 offset = 0, count = 0, first_offset = 0;
  if (m_PC_DumpM == nullptr) 
  {
    printf("m_PC_DumpM == nullptr \n");
    return offset;
  }
  u64 bufferSize = m_PC_DumpM->size();
  // printf("m_PC_DumpM->size() %lx\n",m_PC_DumpM->size());
  u8 *buffer = new u8[bufferSize];
  m_PC_DumpM->getData(0, buffer, bufferSize);
  if (m_64bit_offset)
  {
    for (u64 i = 0; i < bufferSize; i += sizeof(fromto_t))
    {
      if (address == *reinterpret_cast<u64 *>(&buffer[i] + sizeof(fromto_t) / 2))
      {
        offset = *reinterpret_cast<u64 *>(&buffer[i]);
        count++;
        if (count == 1)
          first_offset = offset;
        printf("Main offset = %lx for heap offset = %lx count = %lx\n", offset, address, count);
        if (count == last_count + 1)
        {
          last_count++;
          found_next_count = true;
          break;
        }
      };
    }
  }
  else
  {
    for (u64 i = 0; i < bufferSize; i += 8)
    {
      if (address == *reinterpret_cast<u32 *>(&buffer[i] + 4))
      {
        offset = *reinterpret_cast<u32 *>(&buffer[i]);
        count++;
        if (count == 1)
          first_offset = offset;
        printf("Main offset = %lx for heap offset = %lx count = %lx\n", offset, address, count);
        if (count == last_count + 1)
        {
          last_count++;
          found_next_count = true;
          break;
        }
      };
    }
  }
  
  if (!found_next_count)
  {
    last_count = 1;
    offset = first_offset;
  }
  delete[] buffer;
  printf("get main count = %ld\n",count);
  return offset;
}

void GuiCheats::refresh_fromto()
{
  // if (m_64bit_offset)
  //   return;
  dmntchtPauseCheatProcess();
  bool ledOn = true;
  time_t unixTime1 = time(NULL);
  MemoryDump *PCDump;
  m_PCDumpR_filename.str("");
  m_PCDumpR_filename << m_PCDump_filename.str().c_str() << "R";
  PCDump = new MemoryDump(m_PCDumpR_filename.str().c_str(), DumpType::DATA, true);
  {
    u64 counting_pointers = 0;
    u64 Foffset = 0;
    size_t bufferSize = MAX_BUFFER_SIZE; 
    u8 *buffer = new u8[bufferSize];
    u32 buffer_inc, data_inc;
    if (m_64bit_offset)
    {
      buffer_inc = sizeof(fromto_t);
      data_inc = sizeof(u64);
    }
    else
    {
      buffer_inc = sizeof(fromto32_t);
      data_inc = sizeof(u32);
    }
    size_t FbufferSize = MAX_BUFFER_SIZE - MAX_BUFFER_SIZE % buffer_inc;
    fromto_t fromto_data1 = {0}, fromto_data2 = {0};
    u8 *Fbuffer = new u8[FbufferSize];
    {
      while (Foffset < m_PC_Dump->size())
      {
        setLedState(ledOn);
        ledOn = !ledOn;
        if (m_PC_Dump->size() - Foffset < FbufferSize)
          FbufferSize = m_PC_Dump->size() - Foffset;
        printf("FbufferSize = %lx\n", FbufferSize);
        m_PC_Dump->getData(Foffset, Fbuffer, FbufferSize);
        memcpy(&(fromto_data1.from), Fbuffer, data_inc);
        memcpy(&(fromto_data1.to), Fbuffer + data_inc, data_inc);
        memcpy(&(fromto_data2.from), Fbuffer + (FbufferSize - buffer_inc*2), data_inc);//double check this not including the last two
        memcpy(&(fromto_data2.to), Fbuffer + data_inc + (FbufferSize - buffer_inc*2), data_inc);
        printf("fromto_data1 form = %lx ,to = %lx , fromto_data2 from = %lx , to = %lx \n ", fromto_data1.from, fromto_data1.to, fromto_data2.from, fromto_data2.to);
        size_t i = 0;
        MemoryInfo meminfo;
        u64 memsize, address, start_address, last_address, new_to;
        bool inc = false;
        while (i < FbufferSize) //(fromto_data1.from <= fromto_data2.from)
        {

          bufferSize = MAX_BUFFER_SIZE;
          memsize = fromto_data2.from - fromto_data1.from;
          address = fromto_data1.from + m_heapBaseAddr;
          meminfo = m_debugger->queryMemory(address);
          if (memsize < bufferSize) // don't read usless data
            bufferSize = memsize;

          start_address = address;
          last_address = meminfo.addr + meminfo.size;
          memsize = last_address - address;
          // printf("start address = %lx last address = %lx memsize = %lx i= %lx\n", start_address, last_address, memsize, i);
          if (memsize < bufferSize) // don't read past the segment
            bufferSize = memsize;

          m_debugger->readMemory(buffer, bufferSize, address);

          while (address < start_address + bufferSize)
          {
            memcpy(&new_to, buffer + address - start_address, sizeof(u64));

            if (new_to == (fromto_data1.to + m_heapBaseAddr))
            // if (((realValue._u64 >= m_heapBaseAddr) && (realValue._u64 <= (m_heapEnd))))
            {
              PCDump->addData((u8 *)&fromto_data1.from, data_inc);
              PCDump->addData((u8 *)&fromto_data1.to, data_inc);
              counting_pointers++;
            }
            i += buffer_inc;
            if (i < FbufferSize)
            {
              memcpy(&(fromto_data1.from), Fbuffer + i, data_inc);
              memcpy(&(fromto_data1.to), Fbuffer + data_inc + i, data_inc);
              address = fromto_data1.from + m_heapBaseAddr;
              inc = true;
            }
            else
              break;
          }
          if (!inc)
          {
            printf("not able to access address = %lx i= %lx\n", address, i);
            i += buffer_inc;
            if (i < FbufferSize)
            {
              memcpy(&(fromto_data1.from), Fbuffer + i, data_inc);
              memcpy(&(fromto_data1.to), Fbuffer + data_inc + i, data_inc);
            }
          }
          inc = false;
        };
        printf("next Fbuf\n");
        Foffset += FbufferSize;
      }
    }
    printf("count,%lx\n", counting_pointers);
    delete[] buffer;
    delete[] Fbuffer;
  };
  setLedState(false);
  time_t unixTime2 = time(NULL);
  printf("%s%lx\n", "Stop Time ", unixTime2);
  printf("%s%ld\n", "Stop Time ", unixTime2 - unixTime1);
  if (PCDump->m_compress)
    printf("mcompress = true\n");

// clean up and rename file

  PCDump->flushBuffer();
  delete m_PC_Dump;
  m_PC_Dump = PCDump;
  // PCDumpM->flushBuffer();
  // PCDumpM = PCDumpM;
  dmntchtResumeCheatProcess();
  printf("refresh fromto done\n");
}

void GuiCheats::prep_backjump_stack(u64 address)
{
  const u16 tablesize = 0xFFFF;
  fromto_t *fromto = new fromto_t[tablesize];
  u64 file_range = 0x10000;
  u64 targetaddress = address - m_heapBaseAddr;
  u64 offset = 0;
  u64 count = 0;
  u64 bufferSize = MAX_BUFFER_SIZE;
  u8 *buffer = new u8[bufferSize];
  u64 distance;
  u32 buffer_inc, data_inc;
  if (m_64bit_offset)
  {
    buffer_inc = sizeof(fromto_t);
    data_inc = sizeof(u64);
  }
  else
  {
    buffer_inc = sizeof(fromto32_t);
    data_inc = sizeof(u32);
  }

  while (offset < m_PC_Dump->size())
  {
    if (m_PC_Dump->size() - offset < bufferSize)
      bufferSize = m_PC_Dump->size() - offset;
    m_PC_Dump->getData(offset, buffer, bufferSize); // BM4

    for (u64 i = 0; i < bufferSize; i += buffer_inc) // for (size_t i = 0; i < (bufferSize / sizeof(u64)); i++)
    {
      if (m_abort)
        return;
      u64 pointedaddress = 0; //*reinterpret_cast<u64 *>(&buffer[i]+data_inc);
      memcpy(&pointedaddress, buffer + i + data_inc, data_inc);
      if (targetaddress >= pointedaddress)
      {
        distance = targetaddress - pointedaddress;
        if (distance == 0x18)
        {
          printf("0x18 offset %lx,i %lx\n",offset,i);
        }
        if (distance <= file_range)
        {
          fromto[count].from = 0; //*reinterpret_cast<u64 *>(&buffer[i]);
          memcpy(&(fromto[count].from), buffer + i, data_inc);
          fromto[count].to = pointedaddress;
          // fromto[count].hits = 0;
          count++;
        }
      }
    }
    offset += bufferSize;
  }
// repeat for main
  offset = 0;
  bufferSize = MAX_BUFFER_SIZE;
  while (offset < m_PC_DumpM->size())
  {
    if (m_PC_DumpM->size() - offset < bufferSize)
      bufferSize = m_PC_DumpM->size() - offset;
    m_PC_DumpM->getData(offset, buffer, bufferSize); // BM4

    for (u64 i = 0; i < bufferSize; i += buffer_inc) // for (size_t i = 0; i < (bufferSize / sizeof(u64)); i++)
    {
      if (m_abort)
        return;
      u64 pointedaddress = 0; //*reinterpret_cast<u64 *>(&buffer[i]+8);
      memcpy(&pointedaddress, buffer + i + data_inc, data_inc);
      if (targetaddress >= pointedaddress)
      {
        distance = targetaddress - pointedaddress;
        if (distance <= file_range)
        {
          fromto[count].from = 0;
          fromto[count].to = pointedaddress;
          // fromto[count].hits = 0;
          count++;
        }
      }
    }
    offset += bufferSize;
  }
  // refresh the list




  //
  printf("count = %lx\n",count);
  delete buffer;
  std::sort(fromto,fromto + count, comparefromto);
  m_fromto32_size = count;
  if (m_fromto32 !=nullptr) delete m_fromto32;
  m_fromto32 = fromto;
  m_fromto32_offset = 0;
}

//
void GuiCheats::updatebookmark(bool clearunresolved, bool importbookmark, bool filter)
{
  std::stringstream filebuildIDStr;
  std::stringstream buildIDStr, tempstr;
  searchValue_t value, value2;
  char import[]="import";
  for (u8 i = 0; i < 8; i++)
    buildIDStr << std::nouppercase << std::hex << std::setfill('0') << std::setw(2) << (u16)m_buildID[i];
  if (Config::getConfig()->separatebookmark)
    filebuildIDStr << m_edizon_dir + "/" << buildIDStr.str() << ".dat";
  else
    filebuildIDStr << EDIZON_DIR "/" << buildIDStr.str() << ".dat";

  MemoryDump *tempdump;
  tempdump = new MemoryDump(EDIZON_DIR "/tempbookmark.dat", DumpType::ADDR, true);
  m_memoryDumpBookmark->clear();
  if (m_memoryDumpBookmark!=nullptr) delete m_memoryDumpBookmark;
  m_memoryDumpBookmark = new MemoryDump(EDIZON_DIR "/memdumpbookmark.dat", DumpType::ADDR, true);
  if (m_AttributeDumpBookmark->size() > 0)
  {
    bookmark_t bookmark;
    u64 address;
    for (u64 i = 0; i < m_AttributeDumpBookmark->size(); i += sizeof(bookmark_t))
    {
      m_AttributeDumpBookmark->getData(i, (u8 *)&bookmark, sizeof(bookmark_t));
      if (bookmark.deleted)
        continue; // don't add deleted bookmark
      if (clearunresolved)
      {
        if (unresolved(bookmark.pointer))
          continue;
        else if (filter)
        {
          value2._u64 = m_debugger->peekMemory(m_target);
          memset(&value,0,8);
          memcpy(&value, &value2, dataTypeSizes[m_searchType]);
          // memset(&value + dataTypeSizes[m_searchType], 0, 8 - dataTypeSizes[m_searchType]);
          if (m_searchMode == SEARCH_MODE_EQ)
          {
            if (value._s64 != m_searchValue[0]._s64)
              continue;
          }
          else if (m_searchMode == SEARCH_MODE_RANGE)
          {
            if (!(m_searchValue[0]._s64 <= value._s64 && value._s64 <= m_searchValue[1]._s64))
              continue;
          }
        }
      }
      if (importbookmark)
      {
        tempstr.str("");
        tempstr << bookmark.label;
        if (tempstr.str() == "import")
        continue;
      }
      if (bookmark.heap)
      {
        address = bookmark.offset + m_heapBaseAddr;
      }
      else
      {
        address = bookmark.offset + m_mainBaseAddr;
      }
      MemoryInfo meminfo;
      meminfo = m_debugger->queryMemory(address);
      // static case 
      if (meminfo.perm != Perm_Rw)
        continue;
      m_memoryDumpBookmark->addData((u8 *)&address, sizeof(u64));
      tempdump->addData((u8 *)&bookmark, sizeof(bookmark_t));
    }
    if (importbookmark)
    {
      bookmark_t bookmark;
      bookmark.type = m_searchType;
      // if (Gui::requestKeyboardInput("Import Bookmark", "Enter Label for bookmark to be imported from file.", bookmark.label, SwkbdType_QWERTY, bookmark.label, 18))
      memcpy(&bookmark.label, import, sizeof(import));
      // bookmark.label = "import";
      {
        std::stringstream filebuildIDStr;
        filebuildIDStr << EDIZON_DIR "/" << "DirectTransfer.bmk";
        if (access(filebuildIDStr.str().c_str(), F_OK) != 0)  // rename( "DirectTransfer.bmk", filebuildIDStr.str().c_str());
        {
          filebuildIDStr.str("");
          filebuildIDStr << EDIZON_DIR "/" << buildIDStr.str() << ".bmk";
        }
        MemoryDump *bmkdump;
        bmkdump = new MemoryDump(filebuildIDStr.str().c_str(), DumpType::ADDR, false);
        if (bmkdump->size() > 0)
        {
          printf(" file exist %s \n", filebuildIDStr.str().c_str());
          u64 bufferSize = bmkdump->size();
          printf(" file size is %ld with %ld pointer chains\n", bufferSize, bufferSize / sizeof(pointer_chain_t));
          u8 *buffer = new u8[bufferSize];
          bmkdump->getData(0, buffer, bufferSize);
          u32 goodcount = 0;
          for (u64 i = 0; i < bufferSize; i += sizeof(pointer_chain_t))
          {
            memcpy(&(bookmark.pointer), buffer + i, sizeof(pointer_chain_t));
            if (unresolved(bookmark.pointer))
              continue;
            goodcount++;
            m_memoryDumpBookmark->addData((u8 *)&m_heapBaseAddr, sizeof(u64));
            tempdump->addData((u8 *)&bookmark, sizeof(bookmark_t));
          }
          printf("found %d good ones\n", goodcount);
          delete [] buffer;
          (new Snackbar("Bookmark file imported"))->show();
        }
        else
        {
          printf("bookmark file %s missing \n", filebuildIDStr.str().c_str());
          (new Snackbar("Bookmark file to import from is missing"))->show();
        }
        delete bmkdump;
        remove(filebuildIDStr.str().c_str());
      };
      // filebuildIDStr << EDIZON_DIR "/"
      //                << "DirectTransfer.bmk";
      // if (access(filebuildIDStr.str().c_str(), F_OK) == 0)
    }
    tempdump->setBaseAddresses(m_addressSpaceBaseAddr, m_heapBaseAddr, m_mainBaseAddr, m_heapSize, m_mainSize);
    tempdump->flushBuffer();
    delete tempdump;
    m_AttributeDumpBookmark->clear();
    delete m_AttributeDumpBookmark;
    REPLACEFILE(EDIZON_DIR "/tempbookmark.dat", filebuildIDStr.str().c_str());
    m_AttributeDumpBookmark = new MemoryDump(filebuildIDStr.str().c_str(), DumpType::ADDR, false);
    m_memoryDumpBookmark->setBaseAddresses(m_addressSpaceBaseAddr, m_heapBaseAddr, m_mainBaseAddr, m_heapSize, m_mainSize);
    m_AttributeDumpBookmark->setBaseAddresses(m_addressSpaceBaseAddr, m_heapBaseAddr, m_mainBaseAddr, m_heapSize, m_mainSize);
  }
};
bool GuiCheats::unresolved2(pointer_chain_t *pointer)
{
  printf("source= %lx", pointer->depth);
  for (int z = pointer->depth; z >= 0; z--)
    printf("+ %lx ", pointer->offset[z]);
  printf("\n");
  return true;
}

bool GuiCheats::unresolved(pointer_chain_t pointer)
{
  printf("z=%lx ", pointer.depth);
  if (pointer.depth != 0)
  {
    printf("[main");
    u64 nextaddress = m_mainBaseAddr;
    for (int z = pointer.depth; z >= 0; z--)
    {
      printf("+%lx]", pointer.offset[z]);
      nextaddress += pointer.offset[z];
      MemoryInfo meminfo = m_debugger->queryMemory(nextaddress);
      if (meminfo.perm == Perm_Rw)
        if (z == 0)
        {
          printf("(%lx)\n", nextaddress); // nextaddress = the target
          m_target = nextaddress;
          return false;
        }
        else
        {
          m_debugger->readMemory(&nextaddress, ((m_32bitmode) ? sizeof(u32) : sizeof(u64)), nextaddress);
          printf("[(%lx)", nextaddress);
        }
      else
      {
        printf(" * access denied *\n");
        return true;
      }
    }
    printf("\n");
    return false;
  }
  else
    return false;
}
void GuiCheats::save_meminfos()
{
  MemoryDump *scaninfo = new MemoryDump((m_edizon_dir + "/scaninfo.dat").c_str(), DumpType::UNDEFINED, true);
  scaninfo->setBaseAddresses(m_addressSpaceBaseAddr, m_heapBaseAddr, m_mainBaseAddr, m_heapSize, m_mainSize);
  for (MemoryInfo meminfo : m_targetmemInfos)
  {
    scaninfo->addData((u8 *)&meminfo, sizeof(meminfo));
  }
  scaninfo->flushBuffer();
  delete scaninfo;
}
void GuiCheats::load_meminfos()
{
  MemoryDump *scaninfo = new MemoryDump((m_edizon_dir + "/scaninfo.dat").c_str(), DumpType::UNDEFINED, false);
  m_targetmemInfos.clear();
  MemoryInfo meminfo = {0};
  if (scaninfo->size()> 0 && scaninfo->getDumpInfo().heapBaseAddress == m_heapBaseAddr)
  {
    for (u64 i = 0; i < scaninfo->size() ; i += sizeof(meminfo))
    {
      scaninfo->getData(i, (u8 *)&meminfo, sizeof(meminfo));
      m_targetmemInfos.push_back(meminfo);
    }
  }
  delete scaninfo;
}

static bool compareentry(MultiSearchEntry_t e1, MultiSearchEntry_t e2)
{
  if (e1.on != OFF && e2.on == OFF)
    return true;
  return (e1.offset < e2.offset);
};
static bool comparefromto(fromto_t e1, fromto_t e2)
{
  return (e1.to > e2.to);
};
void GuiCheats::save_multisearch_setup()
{
  std::sort(m_multisearch.Entries, m_multisearch.Entries + sizeof(m_multisearch.Entries) / sizeof(m_multisearch.Entries[0]), compareentry);
  m_multisearch.count = 0;
  m_multisearch.target = 0;
  for (int i = 0; i < M_ENTRY_MAX; i++)
  {
    if (M_ENTRYi.on)
    {
      if (m_multisearch.count == 0) m_multisearch.first = i;
      m_multisearch.count++;
      if (M_ENTRYi.on == TARGET) m_multisearch.target = i;
      m_multisearch.last = i;
    }
  }
  m_multisearch.size = (m_multisearch.Entries[m_multisearch.last].offset / M_ALIGNMENT - m_multisearch.Entries[m_multisearch.first].offset / M_ALIGNMENT) * M_ALIGNMENT;
  m_multisearch.adjustment = (m_multisearch.Entries[m_multisearch.first].offset / M_ALIGNMENT) * M_ALIGNMENT;
  m_multisearch.target_offset = m_multisearch.Entries[m_multisearch.target].offset - (m_multisearch.Entries[m_multisearch.first].offset / M_ALIGNMENT) * M_ALIGNMENT;

  MemoryDump *multisearch = new MemoryDump((m_edizon_dir + "/multisearch.dat").c_str(), DumpType::UNDEFINED, true);
  multisearch->addData((u8 *)&m_multisearch, sizeof(m_multisearch));
  multisearch->flushBuffer();
  delete multisearch;
  printf("MT count = %d",m_multisearch.count);
}
void GuiCheats::load_multisearch_setup()
{
  MemoryDump *multisearch = new MemoryDump((m_edizon_dir + "/multisearch.dat").c_str(), DumpType::UNDEFINED, false);
  if ((multisearch->size() > 0) && (multisearch->size() % sizeof(m_multisearch) == 0))
    multisearch->getData(0, (u8 *)&m_multisearch, sizeof(m_multisearch));
  else
    m_multisearch.Entries[0].on = TARGET;
  delete multisearch;
}
bool GuiCheats::_check_extra_not_OK(u8 *buffer, u32 index)
{
  searchValue_t realValue;
  for (int i = 0; i < M_ENTRY_MAX; i++)
  {
    if (M_ENTRYi.on == ON)
    {
      // printf("i=%d\n",i);
      memset(&realValue, 0, 8);
      memcpy(&realValue, buffer + index + M_ENTRYi.offset - m_multisearch.adjustment, dataTypeSizes[M_ENTRYi.type]);
      switch (M_ENTRYi.mode)
      {
      case SEARCH_MODE_EQ:
        if (realValue._s64 == M_ENTRYi.value1._s64)
        {
          if (Config::getConfig()->exclude_ptr_candidates)
          {
            memset(&realValue, 0, 8);
            memcpy(&realValue, buffer + index + (M_ENTRYi.offset - M_ENTRYi.offset % 8) - m_multisearch.adjustment, 8);
            if (((realValue._u64 >= m_mainBaseAddr) && (realValue._u64 <= (m_mainend))) || ((realValue._u64 >= m_heapBaseAddr) && (realValue._u64 <= (m_heapEnd))))
              return true;
          }
          // printf("match EQ\n");
        }
        else
          return true;
        break;
      case SEARCH_MODE_RANGE:
        if (realValue._s64 >= M_ENTRYi.value1._s64 && realValue._s64 <= M_ENTRYi.value2._s64)
        {
          if (Config::getConfig()->exclude_ptr_candidates)
          {
            memset(&realValue, 0, 8);
            memcpy(&realValue, buffer + index + (M_ENTRYi.offset - M_ENTRYi.offset % 8) - m_multisearch.adjustment, 8);
            if (((realValue._u64 >= m_mainBaseAddr) && (realValue._u64 <= (m_mainend))) || ((realValue._u64 >= m_heapBaseAddr) && (realValue._u64 <= (m_heapEnd))))
              return true;
          }
        }
        else
          return true;
        break;
      case SEARCH_MODE_POINTER: //m_heapBaseAddr, m_mainBaseAddr, m_heapSize, m_mainSize
        if (((realValue._u64 >= m_mainBaseAddr) && (realValue._u64 <= (m_mainend))) || ((realValue._u64 >= m_heapBaseAddr) && (realValue._u64 <= (m_heapEnd))))
        {
          // printf("value is %lx, datatype size %d \n",realValue._u64,dataTypeSizes[M_ENTRYi.type] );
        }
        else
          return true;
        break;
      case SEARCH_MODE_NOT_POINTER: //m_heapBaseAddr, m_mainBaseAddr, m_heapSize, m_mainSize
        if (((realValue._u64 >= m_mainBaseAddr) && (realValue._u64 <= (m_mainend))) || ((realValue._u64 >= m_heapBaseAddr) && (realValue._u64 <= (m_heapEnd))))
        {
          return true;
        }
        break;
      default:
        break;
      }
    };
  }
  return false;
}