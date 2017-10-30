#ifndef NEOVIM_GUI_SIGNATURE
#define NEOVIM_GUI_SIGNATURE

#include <QString>
#include <QMap>
#include <QVariantList>
#include <QVector>
#include <QListWidget>
#include <cstdint>

class QWidget;

namespace NeovimQt {

class LabelVList;

class Signature {
public:
  using GetCellSize = std::function<QSize()>;
  using GetCursorPos = std::function<QPoint()>;
  Signature(QWidget* parent,
      GetCellSize,
      GetCursorPos);

  using SeperationChars = QVector<QString>;
  using StyleDef = QMap<QString, QString>;

  struct SigInfo {
    QString label;
    QVector<QString> params;
  };

  void show(QVector<SigInfo> const&,
      int active_signature,
      int active_param,
      SeperationChars const&);

  void hide();
  void setStyle(StyleDef const&);

private:
  void moveAndShowWidget();
  void addItemsToWidget(
      QVector<SigInfo> const&,
      int active_signature,
      int active_param,
      SeperationChars const&);

  QWidget* parent;
  LabelVList* widget;
  GetCellSize getCellSize;
  GetCursorPos getCursorPos;
};

class SignatureDecoding {
public:
  SignatureDecoding(QWidget* parent,
      Signature::GetCellSize,
      Signature::GetCursorPos);

  void show(QVariantList const&);
  void setStyle(QVariantList const&);
  void hide();

private:
  Signature m_signature;
};

}

#endif

