#ifndef NEOVIM_GUI_POPUPMENU
#define NEOVIM_GUI_POPUPMENU

#include <QSize>
#include <QString>
#include <QVector>
#include <QMap>
#include <QVariantList>
#include <QVariant>

#include <stdexcept>
#include <functional>
#include <cstdint>

class QTableWidget;
class QWidget;

namespace NeovimQt {

class PopupMenuTableWidget;

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
  using ColorConfig = QMap<QString, QString>;
  using KindConfig = QMap<QString, QVector<QString>>;

  static const std::uint32_t visibleRowCount;


  PopupMenu(QWidget* parent,
      GetCellSize cellSizeGetter);

  void setStyle(ColorConfig const&);
  void setKindConfig(KindConfig const&);

  void show(Items items,
      Idx selected,
      std::uint32_t row,
      std::uint32_t col);
  void select(Idx newselected);
  void hide();

private:
  void addItem(std::uint32_t row, Item const& item);
  void addItems(Items const& items);
  void setSelection(bool state);
  void showWindow();
  void setWindowHeight(std::uint32_t items);
  void initStyleIfNotConfigured();
  void moveWindow(std::uint32_t row, std::uint32_t col);

  QWidget* editorWindow;
  PopupMenuTableWidget* widget;
  GetCellSize getCellSize;
  int selected;
  KindConfig kindConfig;
  bool styleSet;
};

class PopupMenuDecoding {
public:
  PopupMenuDecoding(QWidget* parent,
      PopupMenu::GetCellSize getCellSize);

  static PopupMenu::Items convertItems(QVariantList const&);
  static PopupMenu::ColorConfig toColorConfig(QVariantMap const&);
  static PopupMenu::KindConfig toKindConfig(QVariantMap const&);

  void setStyle(QVariantList const& args);
  void setKindConfig(QVariantList const& args);

  void show(QVariantList const& args);
  void select(QVariantList const& args);
  void hide();

private:
  PopupMenu m_menu;
};

}

#endif
