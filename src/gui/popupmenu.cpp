#include "popupmenu.h"

#include <QTableWidget>
#include <QHeaderView>
#include <QScrollBar>
#include <QGraphicsDropShadowEffect>

namespace NeovimQt {
namespace {

// supported kinds
// 'Text', 'Method', 'Function', 'Constructor', 'Field', 'Variable', 'Class',
// 'Interface', 'Module', 'Property', 'Unit', 'Value', 'Enum', 'Keyword',
// 'Snippet', 'Color', 'File', 'Reference'

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


PopupMenu::Item convertItem(QVariantList const& from) {
  if(from.size() != 4) {
    throw PopupMenu::ConversionError(
        "invaid item tab size, failed to convert item");
  }
  PopupMenu::Item item;
  item.word = PopupMenu::variantVal<QString>(from[0]);
  item.kind = PopupMenu::variantVal<QString>(from[1]);
  item.menu = PopupMenu::variantVal<QString>(from[2]);
  return item;
}

class PopupMenuTableWidget
  : public QTableWidget {

public:
  PopupMenuTableWidget(QWidget* parent)
    : QTableWidget(parent) {
    verticalHeader()->hide();
    horizontalHeader()->hide();
    setSortingEnabled(false);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setShowGrid(true);
    setTabKeyNavigation(false);
    setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
    setContentsMargins(0, 0, 0, 0);
    setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    setFocusPolicy(Qt::NoFocus);
    setStyleSheet("color: #fdf4c1;"
        " background-color: #393939;"
        " selection-background-color: #4a4a4a;");
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    QScrollBar* sb = verticalScrollBar();
    sb->setStyleSheet(
      "QScrollBar:vertical{margin: 0 0 0 0; width: 8px; background: #000000}"
      "QScrollBar::add-line:vertical{background: none; border:none; color:none}"
      "QScrollBar::sub-line:vertical{background: none; border:none; color:none}"
      "QScrollBar::handle:vertical{background-color: #555555}"
      "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical{background: none}");

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 255));
    shadow->setOffset(0, 2);
    setGraphicsEffect(shadow);
  }

  QSize sizeHint() const override {
    auto sh = QTableWidget::sizeHint();
    if (static_cast<std::uint32_t>(rowCount()) > PopupMenu::visibleRowCount) {
      static const std::uint32_t scrollbarWidth = 8;
      sh.setWidth(sh.width() + scrollbarWidth);
    }
    return sh;
  }

};

}

const std::uint32_t PopupMenu::visibleRowCount = 15;

PopupMenu::PopupMenu(QWidget* parent,
    GetCellSize cellSizeGetter)
  : widget(new PopupMenuTableWidget(parent)),
    getCellSize(cellSizeGetter) {

    widget->hide();
    widget->setColumnCount(3);

    QHeaderView* hdr = widget->horizontalHeader();
    hdr->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    hdr->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    hdr->setSectionResizeMode(2, QHeaderView::ResizeToContents);
  }

void PopupMenu::show(Items items,
    Idx selectIdx,
    std::uint32_t row,
    std::uint32_t col) {
  if(items.isEmpty()) { return; }
  auto added = addItems(items);
  select(selectIdx);
  setWindowHeight(added);
  showPositionedWindow(row, col);
}

void PopupMenu::select(Idx newselected) {
  if(widget->isVisible() && selected >= 0) {
    setSelection(false);
  }
  selected = newselected;
  if(selected >= 0) {
    setSelection(true);
    widget->scrollToItem(widget->item(selected, 0));
  }
}

void PopupMenu::hide() {
  widget->hide();
  widget->clearContents();
}

void PopupMenu::addItem(std::uint32_t row, Item const& item) {
  widget->setItem(row, 0, getKindItem(item.kind));
  widget->setItem(row, 1, new QTableWidgetItem(QString(' ') + item.word));
  widget->setItem(row, 2, new QTableWidgetItem(item.menu.trimmed()));
}

std::uint32_t PopupMenu::addItems(Items const& items) {
  widget->clearContents();
  widget->setRowCount(items.size());
  std::uint32_t idx = 0;
  for(auto const& item: items) {
    addItem(idx++, item);
  }
  return static_cast<std::uint32_t>(items.size());
}

void PopupMenu::setSelection(bool state) {
  if(widget->rowCount() > selected) {
    widget->item(selected, 1)->setSelected(state);
    widget->item(selected, 2)->setSelected(state);
  }
}

void PopupMenu::showPositionedWindow(std::uint32_t row, std::uint32_t col) {
  auto cellSize = getCellSize();
  widget->move(col*cellSize.width(), (row+1)*cellSize.height());
  widget->show();
}

void PopupMenu::setWindowHeight(std::uint32_t items) {
  static const std::uint32_t bordersize = 3;
  widget->setMaximumHeight(std::min(visibleRowCount, items)*widget->rowHeight(0) + bordersize);
}

PopupMenu::Items PopupMenu::convertItems(QVariantList const& from) {
  PopupMenu::Items items;
  for(auto const& item: from) {
    items.append(convertItem(PopupMenu::variantVal<QVariantList>(item)));
  }
  return items;
}

}

