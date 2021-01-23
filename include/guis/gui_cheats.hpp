#pragma once

#include "guis/gui.hpp"
#include "types.h"

#include <vector>
#include <set>
#include <unordered_map>
#include <stdbool.h>
#include <time.h>
#include "helpers/debugger.hpp"
#include "helpers/memory_dump.hpp"

#include "helpers/dmntcht.h"
enum on_off_target_t
{
  OFF,
  ON,
  TARGET
};
struct MultiSearchEntry_t
{
  char label[20] = {0};
  u16 offset = 0;
  on_off_target_t on = OFF;
  searchType_t type = SEARCH_TYPE_UNSIGNED_32BIT;
  searchMode_t mode = SEARCH_MODE_EQ;
  searchValue_t value1 = {0}, value2 = {0};
};
struct fromto32_t
{
  u32 from;
  u32 to;
};
struct fromto_t
{
  u64 from;
  u64 to;
};
struct fromtoP_t
{
  u64 from;
  u64 to;
  u8 P;
};
enum
{
  FORMAT_DEC,
  FORMAT_HEX
} m_searchValueFormat = FORMAT_DEC;

class GuiCheats : public Gui
{

public:
  GuiCheats();
  ~GuiCheats();

  void update();
  void draw();
  void onInput(u32 kdown);
  void onTouch(touchPosition &touch);
  void onGesture(touchPosition startPosition, touchPosition endPosition, bool finish);

private:
  Debugger *m_debugger;
  u8 m_ramBuffer[0x10 * 14] = {0};
  color_t m_memory[1024] = {0};

  u32 m_selectedEntry = 0;
  u32 m_selectedEntrySave = 0;
  u32 m_selectedEntrySaveCL = 0;
  u32 m_selectedEntrySaveBM = 0;
  u32 m_selectedEntrySaveSR = 0;

  u32 m_searchValueIndex = 0;
  u32 m_addresslist_offset = 0;
  u32 m_addresslist_offsetSaveBM = 0;
  u32 m_addresslist_offsetSaveSR = 0;

  searchValue_t m_searchValue[2];

  enum
  {
    CHEATS,
    CANDIDATES
  } m_menuLocation = CANDIDATES;
  enum
  {
    SEARCH_NONE,
    SEARCH_TYPE,
    SEARCH_MODE,
    SEARCH_REGION,
    SEARCH_VALUE,
    SEARCH_editRAM,
    SEARCH_editRAM2,
    SEARCH_pickjump,
    SEARCH_editExtraSearchValues,
    SEARCH_POINTER,
    SEARCH_POINTER2
  } m_searchMenuLocation = SEARCH_NONE;

  searchType_t m_searchType = SEARCH_TYPE_NONE;
  searchMode_t m_searchMode = SEARCH_MODE_NONE;
  searchRegion_t m_searchRegion = SEARCH_REGION_NONE;

  std::vector<MemoryInfo> m_memoryInfo;
  std::vector<MemoryInfo> m_targetmemInfos;

  MemoryDump *m_memoryDump;
  MemoryDump *m_memoryDump1;
  MemoryDump *m_memoryDumpBookmark;
  MemoryDump *m_AttributeDumpBookmark;
  MemoryDump *m_pointeroffsetDump;
  MemoryDump *m_dataDump;
  MemoryDump *m_PC_Dump = nullptr;
  MemoryDump *m_PC_DumpM = nullptr;
  MemoryDump *m_PC_DumpP = nullptr; // Pointer distance to main file ptr_distance_t
  MemoryDump *m_PC_DumpTo = nullptr; // File to hold list of "To" to be process next

  #define M_ENTRY_MAX 10
  #define M_TARGET m_multisearch.Entries[m_multisearch.target]
  #define M_ALIGNMENT 16
  #define FORWARD_DEPTH 2
  struct forward_chain_t
  {
    u32 length [FORWARD_DEPTH];
    u32 offset [FORWARD_DEPTH];
    u32 target [FORWARD_DEPTH];
  };
  // #define SET_BIT(i) 0x1 << i
  // #define GET_BIT(i) & (SET_BIT(i)) >> i
  struct ptr_distance_t
  {
    bool main:1,l1:1,l2:1,l3:1,l4:1,l5:1,l6:1,l7:1;
  };
  struct MultiSearch_t
  {
    char laber[40] = {0};
    u32 target = 0;
    u32 count = 0, first = 0, last = 0, size = 0, adjustment = 0, target_offset = 0;
    MultiSearchEntry_t Entries[M_ENTRY_MAX];
  };
  MultiSearch_t m_multisearch;

  struct PSsetup_t
  {
    u64 m_numoffset = 3;
    u64 m_max_source = 200;
    u64 m_max_depth = 2;
    u64 m_max_range = 0x800;
    u64 m_EditorBaseAddr = 0;
    u64 m_mainBaseAddr;
    u64 m_mainend;
    bool m_pointersearch_canresume = false;
    bool m_PS_resume = false;
    bool m_PS_pause = false;
    // PointerSearch_state *m_PointerSearch = nullptr;
  };
  u64 m_target = 0;
  u64 m_numoffset = 3;
  u64 m_max_source = 200;
  u64 m_max_depth = 5;
  u64 m_max_range = 0x800;
  u64 m_max_P_range = 0x800;
  u64 m_low_main_heap_addr = 0x100000000;
  u64 m_high_main_heap_addr = 0x10000000000;
  u64 m_pointer_found = 0;
  bool m_abort = false;
  bool m_showpointermenu = false;
  bool m_use_range = false;
  bool m_searched = false;
  std::map<u64, u64> m_frozenAddresses;

  bool m_cheatsPresent = false;
  bool m_sysmodulePresent = false;
  bool m_editRAM = false;
  bool m_nothingchanged = false;

  u64 m_addressSpaceBaseAddr = 0x00;
  u64 m_addressSpaceSize = 0x00;
  u64 m_heapBaseAddr = 0x00;
  u64 m_mainBaseAddr = 0x00;
  u64 m_EditorBaseAddr = 0x00;
  u64 m_BookmarkAddr = 0;
  u8 m_addressmod = 0;
  bool m_64bit_offset = false;
  time_t m_Time1;
  struct helperinfo_t
  {
    u64 address;
    u64 size;
    u64 count;
  };
  struct sourceinfo_t
  {
    u64 foffset;
    u64 offset;
  };

#define IS_PTRCODE_START 0x580F0000
#define IS_OFFSET 0x580F1000
#define IS_FINAL 0x780F0000

#define MAX_NUM_SOURCE_POINTER 200 // bound check for debugging;
#define MAX_POINTER_DEPTH 12       // up to 4 seems OK with forward only search took 94s. 215s for big dump
#define MAX_POINTER_RANGE 0x2000
#define MAX_NUM_POINTER_OFFSET 30
#define HAVESAVE (Title::g_titles[m_debugger->getRunningApplicationTID()] != nullptr) //m_havesave
  // #define MAX_JUMP_STACK 50


  struct jump_table_entry_t
  {
    u32 start_address;
    u32 table_index;
    u32 table_entrysize;
  };
  jump_table_entry_t *m_jumptable;
  fromtoP_t *m_fromto32 = nullptr;
  u64 m_fromto32_offset = 0;
  u32 m_fromto32_size = 0;
  u64 m_selectedJumpSource = 0;
  u32 m_selectedJumpSource_offset = 0;

  fromto_t m_jump_stack[MAX_POINTER_DEPTH + 1];
  s16 m_depth_count = 0;
  u16 m_jump_stack_index = 0;
  u16 m_jump_stack_max = 0;
  u16 m_z = 0;
  bool m_show_ptr = true;
  bool m_havesave = true;
  void iconloadcheck();
  void removef(std::string filePath);
  void renamef(std::string filePath1,std::string filePath2);
  bool autoattachcheck();
  bool freeze();
  bool unfreeze();
  bool autoexitcheck();
  void testlz(); 
  struct PointerSearch_state
  {
    u64 depth = 0;                                                       // depth and index[depth] is where the search is at, pointersearch2 will increment depth and call itself with nexttarget
    u64 index[MAX_POINTER_DEPTH + 1] = {0};                              // when there is a hit retrieve the offsets with indexs and sources[index].offset
    u64 num_offsets[MAX_POINTER_DEPTH + 1] = {0};                        // for analysis
    u64 num_sources[MAX_POINTER_DEPTH + 1] = {0};                        // this is how we will go down the column
    sourceinfo_t sources[MAX_POINTER_DEPTH + 1][MAX_NUM_SOURCE_POINTER]; //the data matrix to get nexttarget from foffset and m_memoryDump1
  };
  PointerSearch_state *m_PointerSearch = nullptr;
  void startpointersearch2(u64 targetaddress);
  void pointersearch2(u64 targetaddress, u64 depth);
  void resumepointersearch2();
  bool m_pointersearch_canresume = false;
  bool m_PS_resume = false;
  bool m_PS_pause = false;
#define PS_depth depth
#define PS_index m_PointerSearch->index[PS_depth]
// #define PS_indexP m_PointerSearch->index[PS_depth-1]
#define PS_num_offsets m_PointerSearch->num_offsets[PS_depth]
#define PS_num_sources m_PointerSearch->num_sources[PS_depth]
// #define PS_num_sourcesP m_PointerSearch->num_sources[PS_depth-1]
#define PS_sources m_PointerSearch->sources[PS_depth]
#define PS_lastdepth m_PointerSearch->depth
#define REPLACEFILE(file1, file2)             \
  remove(file2);                              \
  while (access(file2, F_OK) == 0)            \
  {                                           \
    printf("waiting for delete %s\n", file2); \
  }                                           \
  rename(file1, file2);                       \
  while (access(file2, F_OK) != 0)            \
  {                                           \
    printf("waiting for rename %s\n", file1); \
  }

  bool m_forwardonly = false;
  bool m_forwarddump = false; // reduce from 138 to 26
  struct pointer_chain_t
  {
    u64 depth = 0;
    s64 offset[MAX_POINTER_DEPTH + 1] = {0}; // offset to address pointed by pointer
    // below is for debugging can consider removing;
    // u64 fileoffset[MAX_POINTER_DEPTH + 1] = {0}; // offset to the file that has the address where the pointer was stored in this instance for debugging
  };
  struct bookmark_t
  {
    char label[19] = {0};
    searchType_t type;
    pointer_chain_t pointer;
    bool heap = true;
    u64 offset = 0;
    bool deleted = false;
  };
  bookmark_t m_bookmark;      //global for use in pointer search , target address to be updated dynamically by display routine TBD
  bookmark_t bookmark;        //used in add book mark
  pointer_chain_t m_hitcount; // maybe not used

  std::stringstream m_PCDump_filename; // Pointer base file
  std::stringstream m_PCAttr_filename; // For information only
  std::stringstream m_PCDumpM_filename; // main pointer data
  std::stringstream m_PCDumpP_filename; // Pointer distance to main file ptr_distance_t
  std::stringstream m_PCDumpT_filename; // list of "To" for processing
  std::stringstream m_PCDumpR_filename; // Pointer refresh file
  bool m_redo_prep_pointersearch = false;

  void PCdump();
  enum MemoryType
  {
    MAIN,
    HEAP
  };
  void rebasepointer(searchValue_t value); //struct bookmark_t bookmark);
  // bool check_chain(bookmark_t *bookmark, u64 *address);
  // void startpointersearch(u64 address, u64 depth, u64 range, u64 num, MemoryDump **displayDump);
  // void startpointersearch(u64 address, u64 depth, u64 range, u64 num, MemoryDump **displayDump);
  // void pointersearch(u64 targetaddress, MemoryDump **displayDump, MemoryDump **dataDump, pointer_chain_t pointerchain);
  void pointersearch(u64 targetaddress, struct pointer_chain_t pointerchain);
  void pointercheck();
  void startpointersearch(u64 targetaddress); //, MemoryDump **displayDump);
  void searchpointer(u64 address, u64 depth, u64 range, struct pointer_chain_t pointerchain);
  bool valuematch(searchValue_t value, u64 nextaddress);
  bool getinput(std::string headerText, std::string subHeaderText, std::string initialText, searchValue_t *searchValue);
  bool addcodetofile(u64 index);
  bool editcodefile();
  bool dumpcodetofile();
  void reloadcheats();
  bool reloadcheatsfromfile(u8 *buildID, u64 titleID);
  bool addstaticcodetofile(u64 index);
  void PSsaveSTATE();
  void PSresumeSTATE();
  void updatebookmark(bool clearunresolved, bool importbookmark, bool filter);
  bool unresolved(pointer_chain_t pointer);
  bool unresolved2(pointer_chain_t *pointer);
  void save_meminfos();
  void load_meminfos();
  void save_multisearch_setup();
  void load_multisearch_setup();
  bool _check_extra_not_OK(u8 *buffer, u32 i);
  void _moveLonelyCheats(u8 *buildID, u64 titleID);
  void _writegameid();
  u64 m_heapSize = 0;
  u64 m_mainSize = 0;
  u64 m_heapEnd = 0;
  u64 m_mainend = 0;
  u8 m_buildID[0x20];
  // NsApplicationControlData appControlData;
  // std::string m_titleName;
  // std::string m_versionString;

  DmntCheatEntry *m_cheats;
  u64 m_cheatCnt;
  bool *m_cheatDelete;

  void drawSearchRAMMenu();

  void drawEditRAMMenu();
  void draw_easymode();
  std::string buttonStr(u32 buttoncode);
  void drawEditRAMMenu2();
  void drawEditExtraSearchValues();
  void drawSEARCH_pickjump();
  bool m_editCheat = false;
  bool m_32bitmode = false;
  void editor_input(u32 kdown, u32 kheld);
  void EditExtraSearchValues_input(u32 kdown, u32 kheld);
  void easymode_input(u32 kdown, u32 kheld);
  void pickjump_input(u32 kdown, u32 kheld);
  void drawSearchPointerMenu();
  void searchMemoryAddressesPrimary(Debugger *debugger, searchValue_t searchValue1,
                                    searchValue_t searchValue2, searchType_t searchType,
                                    searchMode_t searchMode, searchRegion_t searchRegion,
                                    MemoryDump **displayDump, std::vector<MemoryInfo> memInfos);

  void MTsearchMemoryAddressesPrimary(Debugger *debugger, searchValue_t searchValue1,
                                    searchValue_t searchValue2, searchType_t searchType,
                                    searchMode_t searchMode, searchRegion_t searchRegion,
                                    MemoryDump **displayDump, std::vector<MemoryInfo> memInfos);

  void searchMemoryAddressesPrimary2(Debugger *debugger, searchValue_t searchValue1,
                                     searchValue_t searchValue2, searchType_t searchType,
                                     searchMode_t searchMode, searchRegion_t searchRegion,
                                     MemoryDump **displayDump, std::vector<MemoryInfo> memInfos);

  void prep_pointersearch(Debugger *debugger, std::vector<MemoryInfo> memInfos);

  u64 get_main_offset32(u64 address);
  
  void refresh_fromto();

  void prep_backjump_stack(u64 address);

  void prep_forward_stack();

  void searchMemoryAddressesSecondary(Debugger *debugger, searchValue_t searchValue1,
                                      searchValue_t searchValue2, searchType_t searchType,
                                      searchMode_t searchMode, bool use_range, MemoryDump **displayDump);

  void searchMemoryAddressesSecondary2(Debugger *debugger, searchValue_t searchValue1,
                                       searchValue_t searchValue2, searchType_t searchType,
                                       searchMode_t searchMode, MemoryDump **displayDump);

  void searchMemoryValuesPrimary(Debugger *debugger, searchType_t searchType, searchMode_t searchMode,
                                 searchRegion_t searchRegion, MemoryDump **displayDump, std::vector<MemoryInfo> memInfos);

  void searchMemoryValuesSecondary(Debugger *debugger, searchType_t searchType,
                                   searchMode_t searchMode, searchRegion_t searchRegion,
                                   MemoryDump **displayDump, std::vector<MemoryInfo> memInfos);

  void searchMemoryValuesTertiary(Debugger *debugger, searchValue_t searchValue1,
                                  searchValue_t searchValue2, searchType_t searchType,
                                  searchMode_t searchMode, searchRegion_t searchRegion, bool use_range,
                                  MemoryDump **displayDump, std::vector<MemoryInfo> memInfos);
};
