#ifndef NEOVIM_GUI_POPUPMENU
#define NEOVIM_GUI_POPUPMENU

#include <QTableWidget>
#include <QString>
#include <QHeaderView>
#include <QScrollBar>
#include <QGraphicsDropShadowEffect>
#include <iostream>
#include <memory>
#include <map>

// supported kinds
// 'Text', 'Method', 'Function', 'Constructor', 'Field', 'Variable', 'Class',
// 'Interface', 'Module', 'Property', 'Unit', 'Value', 'Enum', 'Keyword',
// 'Snippet', 'Color', 'File', 'Reference'

namespace NeovimQt {

inline QTableWidgetItem* getKindItem(QString const& kind) {
  if (kind.isEmpty()) {
    return new QTableWidgetItem("un");
  }
  static const std::map<QString, std::pair<QString, QColor>> abbr = {
    { "Text", {"tx", QColor(0x07, 0x66, 0x78)}},
    { "Method", {"mt", QColor(0x79, 0x74, 0x0e)}},
    { "Function", {"fn", QColor(0x00, 0xa7, 0xaf)}},
    { "Constructor", {"ct", QColor(50, 100, 50)}},
    { "Field", {"fd", QColor(0xaf, 0x3a, 0x03)}},
    { "Variable", {"vr", QColor(50, 100, 50)}},
    { "Class", {"cl", QColor(0x42, 0x7b, 0x58)}},
    { "Interface", {"if", QColor(50, 100, 50)}},
    { "Module", {"md", QColor(0xb5, 0x76, 0x14)}},
    { "Property", {"pr", QColor(50, 100, 50)}},
    { "Unit", {"ut", QColor(50, 100, 50)}},
    { "Value", {"vl", QColor(50, 100, 50)}},
    { "Enum", {"em", QColor(50, 100, 50)}},
    { "Keyword", {"kd", QColor(0xdd, 0x6f, 0x48)}},
    { "Snippet", {"st", QColor(50, 100, 50)}},
    { "Color", {"cl", QColor(50, 100, 50)}},
    { "File", {"fe", QColor(50, 100, 50)}},
    { "Reference", {"rf", QColor(50, 100, 50)}}
  };

  auto match = abbr.find(kind);
  if(match == abbr.end()) {
    auto* item = new QTableWidgetItem(kind);
    item->setBackground(QBrush(QColor(200,0,0)));
    return item;
  }
  auto* item = new QTableWidgetItem(match->second.first);
  item->setBackground(QBrush(match->second.second));
  item->setTextAlignment(Qt::AlignHCenter);
  return item;
}

class PopupMenu {
  public:
    struct Item {
      QString word;
      QString kind;
      QString menu;
    };
    using Idx = int;
    using Items = QVector<Item>;
    using GetCellSize = std::function<QSize()>;

    PopupMenu(QWidget* parent, GetCellSize cellSizeGetter)
      : widget(new QTableWidget(parent)),
        getCellSize(cellSizeGetter) {
        widget->setColumnCount(3);
        widget->verticalHeader()->hide();
        widget->horizontalHeader()->hide();
        widget->setSortingEnabled(false);
        widget->setSelectionBehavior(QAbstractItemView::SelectRows);
        widget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        widget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        widget->setSelectionMode(QAbstractItemView::SingleSelection);
        widget->setShowGrid(true);
        widget->setTabKeyNavigation(false);
        widget->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
        widget->hide();
        widget->setContentsMargins(0, 0, 0, 0);
        widget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

        widget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        widget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        widget->setFocusPolicy(Qt::NoFocus);
        widget->setStyleSheet("color: #fdf4c1;"
            " background-color: #393939;"
            " selection-background-color: #4a4a4a;");
        widget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
        QHeaderView* hdr = widget->horizontalHeader();
        hdr->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        hdr->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        hdr->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        QScrollBar* sb = widget->verticalScrollBar();
        sb->setContentsMargins(0,0,0,0);
        sb->setStyleSheet(
          "QScrollBar:vertical{margin: 0 0 0 0; width: 8px; background: #000000}"
          "QScrollBar::add-line:vertical{background: none; border:none; color:none; subcontrol-position: bottom; subcontrol-origin: margin; height: 20px; border: 2px}"
          "QScrollBar::sub-line:vertical{background: none; border:none; color:none; subcontrol-position: top; subcontrol-origin: margin; height: 20px; border: 2px}"
          "QScrollBar::handle:vertical{background-color: #555555}"
          "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical{background: none;}");

        QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(widget);
        shadow->setBlurRadius(20);
        shadow->setColor(QColor(0, 0, 0, 255));
        shadow->setOffset(0, 2);
        widget->setGraphicsEffect(shadow);
      }
    void show(Items items,
        Idx selected,
        std::uint32_t row,
        std::uint32_t col) {
      std::cerr << "popupmenu_show" << std::endl;
      widget->setRowCount(items.size());

      auto cellSize = getCellSize();
      widget->move(col*cellSize.width(), (row+1)*cellSize.height());

      std::uint32_t idx = 0;
      for(auto const& item: items) {
        addItem(idx++, item);
      }
      select(selected);
      widget->setMaximumHeight(std::min(15, items.size())*widget->rowHeight(0));
      widget->show();
    }
    void select(Idx newselected) {
      std::cerr << "popupmenu_select" << std::endl;
      if(widget->isVisible() && selected >= 0) {
        setSelection(false);
      }
      selected = newselected;
      if(selected >= 0) {
        setSelection(true);
        widget->scrollToItem(widget->item(selected, 0));
      }
    }
    void hide() {
      std::cerr << "popupmenu_hide" << std::endl;
      widget->hide();
      widget->clearContents();
    }
  private:
    void addItem(std::uint32_t row, Item const& item) {
      widget->setItem(row, 0, getKindItem(item.kind));
      widget->setItem(row, 1, new QTableWidgetItem(QString(' ') + item.word));
      widget->setItem(row, 2, new QTableWidgetItem(item.menu));
    }
    void setSelection(bool state) {
      widget->item(selected, 1)->setSelected(state);
      widget->item(selected, 2)->setSelected(state);
    }
    QTableWidget* widget;
    GetCellSize getCellSize;
    int selected;
};

inline PopupMenu::Item convertItem(QVariantList const& from) {
  PopupMenu::Item item;
  item.word = from.at(0).toString();
  item.kind = from.at(1).toString();
  item.menu = from.at(2).toString();
  return item;
}

inline PopupMenu::Items convertItems(QVariantList const& from) {
  PopupMenu::Items items;
  for(auto const& item: from) {
    items.append(convertItem(item.toList()));
  }
  return items;
}

}

#endif

