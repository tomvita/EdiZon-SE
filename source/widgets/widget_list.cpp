#include "widgets/widget_list.hpp"

#include "ui_elements/list_selector.hpp"

WidgetList::WidgetList(Interpreter *interpreter, bool isDummy, std::string tooltip, std::vector<std::string> listItemNames, std::vector<s32> listItemValues) :
 Widget(interpreter, isDummy, tooltip), m_listItemNames(listItemNames), m_intListItemValues(listItemValues) {
  m_widgetDataType = INT;
}

WidgetList::WidgetList(Interpreter *interpreter, bool isDummy, std::string tooltip, std::vector<std::string> listItemNames, std::vector<std::string> listItemValues) :
 Widget(interpreter, isDummy, tooltip), m_listItemNames(listItemNames), m_strListItemValues(listItemValues) {
  m_widgetDataType = STRING;
}

WidgetList::~WidgetList() {

}

void WidgetList::draw(Gui *gui, u16 x, u16 y) {
  std::stringstream ss;
  if (m_widgetDataType == INT) {
    s64 intValue = Widget::getIntegerValue();
    if (std::find(m_intListItemValues.begin(), m_intListItemValues.end(), intValue) != m_intListItemValues.end()) {
      ptrdiff_t pos = find(m_intListItemValues.begin(), m_intListItemValues.end(), intValue) - m_intListItemValues.begin();
      ss << m_listItemNames[pos];
    } else {
      ss << "未知值：" << intValue;
    }
  } else if (m_widgetDataType == STRING) {
    std::string strValue = Widget::getStringValue();
    if (std::find(m_strListItemValues.begin(), m_strListItemValues.end(), strValue) != m_strListItemValues.end()) {
      ptrdiff_t pos = find(m_strListItemValues.begin(), m_strListItemValues.end(), strValue) - m_strListItemValues.begin();
      ss << m_listItemNames[pos];
    } else {
      ss << "未知值：" << strValue;
    }
  }

  gui->drawTextAligned(font20, x + WIDGET_WIDTH - 140, y + (WIDGET_HEIGHT / 2.0F), currTheme.selectedColor, ss.str().c_str(), ALIGNED_RIGHT);
}

void WidgetList::onInput(u32 kdown) {
  if (kdown & KEY_A && Gui::g_currListSelector == nullptr) {
    (new ListSelector("选择项目", "\uE0E0 - 选择      \uE0E1 - 返回", m_listItemNames))->setInputAction([&](u32 k, u16 selectedItem){
      if(k & KEY_A) {
        if (m_widgetDataType == INT)
          Widget::setIntegerValue(m_intListItemValues[selectedItem]);
        else if (m_widgetDataType == STRING)
          Widget::setStringValue(m_strListItemValues[selectedItem]);
        Gui::g_currListSelector->hide();
      }
    })->show();
  }
}

void WidgetList::onTouch(touchPosition &touch) {
  (new ListSelector("选择项目", "\uE0E0 - 选择      \uE0E1 - 返回", m_listItemNames))->setInputAction([&](u32 k, u16 selectedItem){
    if(k & KEY_A) {
      if (m_widgetDataType == INT)
        Widget::setIntegerValue(m_intListItemValues[selectedItem]);
      else if (m_widgetDataType == STRING)
        Widget::setStringValue(m_strListItemValues[selectedItem]);
      Gui::g_currListSelector->hide();
    }
  })->show();
}
