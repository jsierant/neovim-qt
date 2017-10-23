#ifndef NEOVIM_GUI_POPUPMENU
#define NEOVIM_GUI_POPUPMENU

#include <QTableWidget>
#include <QString>
#include <QHeaderView>
#include <QScrollBar>
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
        widget->setShowGrid(false);
//         widget->setWindowFlag(Qt::SubWindow, true);
        widget->setTabKeyNavigation(false);
        widget->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
        widget->hide();
        widget->setContentsMargins(0, 0, 0, 0);
        widget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

        widget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        widget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        widget->setFocusPolicy(Qt::NoFocus);
        widget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
        QHeaderView* hdr = widget->horizontalHeader();
        hdr->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        hdr->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        hdr->setSectionResizeMode(2, QHeaderView::ResizeToContents);
        QScrollBar* sb = widget->verticalScrollBar();
        sb->setContentsMargins(10,0,0,0);
        sb->setMaximumWidth(10);
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
      widget->setMaximumHeight(std::min(10, items.size())*widget->rowHeight(0));
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
        widget->scrollToItem(widget->item(selected, 0),
                             QAbstractItemView::PositionAtTop);
      }
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
    void setSelection(bool state) {
      widget->item(selected, 0)->setSelected(state);
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

