#ifndef NEOVIM_GUI_POPUPMENU
#define NEOVIM_GUI_POPUPMENU

#include <QTableWidget>
#include <QString>
#include <QHeaderView>
#include <iostream>
#include <memory>

namespace NeovimQt {

class PopupMenu {
  public:
    struct Item {
      QString word;
      QString kind;
      QString menu;
    };
    using Idx = int;
    using Items = QVector<Item>;

    PopupMenu(QWidget* parent)
      : widget(new QTableWidget(parent)) {
        widget->setColumnCount(3);
        widget->verticalHeader()->hide();
        widget->horizontalHeader()->hide();
        widget->setSortingEnabled(false);
        widget->setSelectionBehavior(QAbstractItemView::SelectRows);
        widget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        widget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        widget->setSelectionMode(QAbstractItemView::SingleSelection);
        widget->setShowGrid(false);
//         widget->setWindowFlag(Qt::SubWindow, true);
        widget->setTabKeyNavigation(false);
        widget->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
        widget->hide();
        widget->setContentsMargins(0, 0, 0, 0);
        widget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
        widget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        widget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
      }
    void show(Items items,
        Idx selected,
        std::uint32_t raw,
        std::uint32_t col) {
      std::cerr << "popupmenu_show" << std::endl;
      widget->setRowCount(items.size());

		int(float64(col)*editor.font.truewidth)-popupItems[0].kindLable.Width()-8,
		(row+1)*editor.font.lineHeight,

      widget->move();

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
      widget->setItem(row, 0, new QTableWidgetItem(item.word));
      widget->setItem(row, 1, new QTableWidgetItem(item.kind));
      widget->setItem(row, 2, new QTableWidgetItem(item.menu));
    }
    std::unique_ptr<QTableWidget> widget;
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

