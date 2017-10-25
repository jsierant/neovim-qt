#include "popupmenu.h"

#include <QTableWidget>
#include <QHeaderView>
#include <QScrollBar>
#include <QGraphicsDropShadowEffect>
#include <QDebug>
#include <map>

namespace NeovimQt {
namespace {

inline QTableWidgetItem* getKindItem(QString const& kind,
    PopupMenu::KindConfig const& config) {
  auto def = config.value(kind, {kind, "#232323"});
  auto* item = new QTableWidgetItem(def[0]);
  item->setBackground(QBrush(QColor(def[1])));
  item->setTextAlignment(Qt::AlignHCenter|Qt::AlignBottom);
  return item;
}

PopupMenu::KindConfig const defaultKindConfig {
  { "Text", {"tx", "#076678"}},
  { "Method", {"mt", "#79740e"}},
  { "Function", {"fn", "#00a7af"}},
  { "Constructor", {"ct", "#223322"}},
  { "Field", {"fd", "#af3a03"}},
  { "Variable", {"vr", "#3432d34"}},
  { "Class", {"cl", "#427b58"}},
  { "Interface", {"if", "#842004"}},
  { "Module", {"md", "#b57614"}},
  { "Property", {"pr", "#023435"}},
  { "Unit", {"ut", "#934139"}},
  { "Value", {"vl", "#99e92f"}},
  { "Enum", {"em", "#409349"}},
  { "Keyword", {"kd", "#dd6f48"}},
  { "Snippet", {"st", "#530029"}},
  { "Color", {"cl", "#a42f93"}},
  { "File", {"fe", "#53232d"}},
  { "Reference", {"rf", "#ed2334"}}
};

PopupMenu::Item convertItem(QVariantList const& from) {
  if(from.size() != 4) {
    throw PopupMenuDecoding::ConversionError(
        "Invaid item tab size, failed to convert item");
  }
  PopupMenu::Item item;
  item.word = PopupMenuDecoding::variantVal<QString>(from[0]);
  item.kind = PopupMenuDecoding::variantVal<QString>(from[1]);
  item.menu = PopupMenuDecoding::variantVal<QString>(from[2]);
  return item;
}

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
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(5);
    shadow->setColor(QColor(0, 0, 0, 255));
    shadow->setOffset(0, 2);
    setGraphicsEffect(shadow);
  }

  QSize sizeHint() const override {
    auto sh = QTableWidget::sizeHint();
    auto rCount = static_cast<std::uint32_t>(rowCount());
    if (rCount > PopupMenu::visibleRowCount) {
      static const std::uint32_t scrollbarWidth = 8;
      sh.setWidth(sh.width() + scrollbarWidth);
    }
    return sh;
  }

  void mousePressEvent(QMouseEvent*) override {}
  void mouseReleaseEvent(QMouseEvent*) override {}
  void mouseDoubleClickEvent(QMouseEvent*) override {}
  void wheelEvent(QWheelEvent *) override {}

  void updateStyle(PopupMenu::ColorConfig const& config) {
    setStyleSheet(QString("color: %1;"
        " background-color: %2;"
        " selection-background-color: %3;")
        .arg(config.value("foreground", "#fdf4c1"))
        .arg(config.value("background", "#393939"))
        .arg(config.value("selected", "#4a4a4a")) );

    QScrollBar* sb = verticalScrollBar();
    sb->setStyleSheet(QString(
      "QScrollBar:vertical{margin: 0 0 0 0; width: 8px; background: %2}"
      "QScrollBar::add-line:vertical{background: none; border:none; color:none}"
      "QScrollBar::sub-line:vertical{background: none; border:none; color:none}"
      "QScrollBar::handle:vertical{background-color: %1}"
      "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical{background: none}")
        .arg(config.value("scrollbar-foreground", "#555555"))
        .arg(config.value("scrollbar-background", "#000000")) );

  }
};


const std::uint32_t PopupMenu::visibleRowCount = 15;

PopupMenu::PopupMenu(QWidget* parent,
    GetCellSize cellSizeGetter)
  : widget(new PopupMenuTableWidget(parent)),
    getCellSize(cellSizeGetter),
    selected(-1),
    kindConfig(defaultKindConfig),
    styleSet(false) {

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
  addItems(items);
  setWindowHeight(items.size());
  select(selectIdx);
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
  widget->setItem(row, 0, getKindItem(item.kind, kindConfig));
  widget->setItem(row, 1, new QTableWidgetItem(QString(' ') + item.word));
  widget->setItem(row, 2, new QTableWidgetItem(item.menu.trimmed()));
}

void PopupMenu::addItems(Items const& items) {
  widget->clearContents();
  widget->setRowCount(items.size());
  std::uint32_t idx = 0;
  for(auto const& item: items) {
    addItem(idx++, item);
  }
}

void PopupMenu::setSelection(bool state) {
  if(widget->rowCount() > selected) {
    widget->item(selected, 1)->setSelected(state);
    widget->item(selected, 2)->setSelected(state);
  }
}

void PopupMenu::showPositionedWindow(std::uint32_t row, std::uint32_t col) {
  static auto const offset = 4u;
  auto cellSize = getCellSize();
  widget->move(col*cellSize.width(), (row+1)*cellSize.height() + offset);
  initStyleIfNotConfigured();
  widget->show();
}

void PopupMenu::setWindowHeight(std::uint32_t items) {
  static const std::uint32_t bordersize = 2;
  widget->setMaximumHeight(visibleRowCount*widget->rowHeight(0) + bordersize);
  widget->setMinimumHeight(widget->rowHeight(0) + bordersize);
}

void PopupMenu::setStyle(ColorConfig const& colors) {
  widget->updateStyle(colors);
  styleSet = true;
}

void PopupMenu::setKindConfig(KindConfig const& newKindConfig) {
  kindConfig = newKindConfig;
}

void PopupMenu::initStyleIfNotConfigured() {
  if(!styleSet) {
    widget->updateStyle(ColorConfig());
    styleSet = true;
  }
}

PopupMenuDecoding::PopupMenuDecoding(QWidget* parent,
                                     PopupMenu::GetCellSize getCellSize)
  : m_menu(parent, getCellSize) {}

PopupMenu::ColorConfig PopupMenuDecoding::toColorConfig(QVariantMap const& in) {
  PopupMenu::ColorConfig cc;
  for(auto const option: in.keys()) {
    cc.insert(option, variantVal<QString>(in[option]));
  }
  return cc;
}


PopupMenu::KindConfig PopupMenuDecoding::toKindConfig(QVariantMap const& in) {
  PopupMenu::KindConfig kc;
  for(auto const option: in.keys()) {
    auto def = toVect<QString>(variantVal<QVariantList>(in[option]));
    if (def.size() != 2) {
      throw ConversionError(
          "Wrong number of parameter for kind value expected [abbr, color]");
    }
    kc.insert(option, def);
  }
  return kc;
}

void PopupMenuDecoding::setStyle(QVariantList const& args) {
  try {
    if(args.size() == 2) {
      m_menu.setStyle(toColorConfig(variantVal<QVariantMap>(args[1])));
      return;
    }
    qWarning() << "Invalid number of args for popupmenu set style: " << args;
  }
  catch (ConversionError const& e) {
    qWarning() << "Failed to load params for popupmenu set style, what: "
               << e.what() << " args:" << args;
  }
}

void PopupMenuDecoding::setKindConfig(QVariantList const& args) {
  try {
    if(args.size() == 2) {
      return m_menu.setKindConfig(toKindConfig(variantVal<QVariantMap>(args[1])));
    }
    qWarning() << "Invalid number of args for popupmenu set kind config: " << args;
  }
  catch (ConversionError const& e) {
    qWarning() << "Failed to load params for popupmenu set kind config, what: "
               << e.what() << " args:" << args;
  }
}

void PopupMenuDecoding::show(QVariantList const& args) {
  try {
    if(args.size() == 4) {
      m_menu.show(
          convertItems(
            variantVal<QVariantList>(args[0])),
          variantVal<int>(args[1]),
          variantVal<std::uint32_t>(args[2]),
          variantVal<std::uint32_t>(args[3]));
      return;
    }
    qWarning() << "invalid number of args for popupmenu show";
  }
  catch (ConversionError const& e) {
    qWarning() << "failed to load params for popupmenu show, what: " << e.what();
  }
}

void PopupMenuDecoding::select(QVariantList const& args) {
  if(args.size() == 1 && args[0].canConvert<int>()) {
    return m_menu.select(args[0].value<int>());
  }
  qWarning() << "invalid arguments for popupmenu_show";
}

void PopupMenuDecoding::hide() {
  m_menu.hide();
}

PopupMenu::Items
PopupMenuDecoding::convertItems(QVariantList const& from) {
  PopupMenu::Items items;
  for(auto const& item: from) {
    items.append(convertItem(
          PopupMenuDecoding::variantVal<QVariantList>(item)));
  }
  return items;
}

}

