#ifndef NEOVIM_GUI_POPUPMENU
#define NEOVIM_GUI_POPUPMENU

#include <QSize>
#include <QString>
#include <QVector>
#include <QVariantList>

#include <stdexcept>
#include <functional>

class QTableWidget;
class QWidget;

namespace NeovimQt {


class PopupMenu {
public:
  struct ConversionError : std::runtime_error{
    ConversionError(char const* msg)
      : std::runtime_error(msg) {}
  };

  struct Item {
    QString word;
    QString kind;
    QString menu;
  };
  using Idx = int;
  using Items = QVector<Item>;
  using GetCellSize = std::function<QSize()>;

  static const std::uint32_t visibleRowCount;

  static Items convertItems(QVariantList const&);

  PopupMenu(QWidget* parent,
      GetCellSize cellSizeGetter);

  void show(Items items,
      Idx selected,
      std::uint32_t row,
      std::uint32_t col);
  void select(Idx newselected);
  void hide();

  template<typename T>
    static T variantVal(QVariant const& from) {
      if(from.canConvert<T>()) {
        return from.value<T>();
      }
      throw PopupMenu::ConversionError("variant conversion failed");
    }

private:
  void addItem(std::uint32_t row, Item const& item);
  std::uint32_t addItems(Items const& items);
  void setSelection(bool state);
  void showPositionedWindow(std::uint32_t row, std::uint32_t col);
  void setWindowHeight(std::uint32_t items);

  QTableWidget* widget;
  GetCellSize getCellSize;
  int selected;
};

}

#endif
