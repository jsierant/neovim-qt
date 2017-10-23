#ifndef NEOVIM_GUI_POPUPMENU
#define NEOVIM_GUI_POPUPMENU

#include <QTableWidget>
#include <QString>
#include <QHeaderView>
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
    { "Text", {"txt", QColor(0x07, 0x66, 0x78)}},
    { "Method", {"mth", QColor(0x79, 0x74, 0x0e)}},
    { "Function", {"fun", QColor(0x00, 0xa7, 0xaf)}},
    { "Constructor", {"ctr", QColor(50, 100, 50)}},
    { "Field", {"fld", QColor(0xaf, 0x3a, 0x03)}},
    { "Variable", {"var", QColor(50, 100, 50)}},
    { "Class", {"cls", QColor(0x42, 0x7b, 0x58)}},
    { "Interface", {"ifc", QColor(50, 100, 50)}},
    { "Module", {"mod", QColor(50, 100, 50)}},
    { "Property", {"pro", QColor(50, 100, 50)}},
    { "Unit", {"unit", QColor(50, 100, 50)}},
    { "Value", {"val", QColor(50, 100, 50)}},
    { "Enum", {"enum", QColor(50, 100, 50)}},
    { "Keyword", {"kwd", QColor(0xdd, 0x6f, 0x48)}},
    { "Snippet", {"snp", QColor(50, 100, 50)}},
    { "Color", {"col", QColor(50, 100, 50)}},
    { "File", {"file", QColor(50, 100, 50)}},
    { "Reference", {"ref", QColor(50, 100, 50)}}
  };

  auto match = abbr.find(kind);
  if(match == abbr.end()) {
    auto* item = new QTableWidgetItem(kind);
    item->setBackground(QBrush(QColor(200,0,0)));
    return item;
  }
  auto* item = new QTableWidgetItem(match->second.first);
  item->setBackground(QBrush(match->second.second));
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
//         widget->setWindowFlag(Qt::SubWindow, true);
        widget->setTabKeyNavigation(false);
        widget->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
        widget->hide();
        widget->setContentsMargins(0, 0, 0, 0);
        widget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
        widget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        widget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        widget->setStyleSheet("color: #fdf4c1; background-color: #393939; selection-background-color: #4a4a4a");
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
      widget->show();
    }
    void select(Idx selected) {
      std::cerr << "popupmenu_select" << std::endl;
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
    QTableWidget* widget;
    GetCellSize getCellSize;
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

