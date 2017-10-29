
#include "signature.h"
#include "args_conv.h"

#include <QDebug>
#include <QVariantMap>
#include <QFontMetrics>
#include <QListWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QObjectCleanupHandler>
#include <memory>

namespace NeovimQt {


class LabelVList {
public:
  LabelVList(QWidget* p)
    : parent(p),
      widget(new QWidget(p)),
      layout(new QVBoxLayout(widget)),
      size(0, 0) {
    widget->hide();
    widget->setContentsMargins(0, 0, 0, 0);
    widget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    widget->setFocusPolicy(Qt::NoFocus);
    widget->setStyleSheet(QString(
          "color: %1"
          "; background-color: %2")
        .arg("#fdf4c1")
        .arg("#393939") );
    layout->setContentsMargins(0, 0, 0, 0);
    widget->setLayout(layout);
    }
  void addItem(QString const& text,
      Qt::TextFormat format = Qt::RichText) {
    auto* label = new QLabel(text);
    auto font = parent->font();
    label->setFont(font);
    label->setTextFormat(format);
    layout->addWidget(label);
    auto fm = QFontMetrics(font);
    size = QSize(std::max(size.width(), label->minimumSizeHint().width()), fm.height() + size.height());
    qDebug() << text;
  }

  void clear() {
    size = QSize(0, 0);
    while(layout->count()) {
      delete layout->takeAt(0);
    }
  }

  void showAt(int x, int y) {
    auto f = parent->font();
    widget->setMaximumHeight(size.height());
    widget->setMaximumWidth(size.width());
    widget->setMinimumHeight(size.height());
    widget->setMinimumWidth(size.width());
    if(!widget->isVisible()) {
      widget->move(x, y);
    }
    widget->show();
  }
  void hide() {
    widget->hide();
  }

private:
  QWidget* parent;
  QWidget* widget;
  QVBoxLayout* layout;
  QSize size;
};

Signature::Signature(QWidget* p,
    GetCellSize cellSizeGetter,
    GetCursorPos cursorPosGetter)
  : parent(p),
    widget(new LabelVList(p)),
    getCellSize(cellSizeGetter),
    getCursorPos(cursorPosGetter) {
}

QString join(QVector<QString> list, QString const& sep) {
  if(list.isEmpty()) { return ""; }
  QString joined = list.takeFirst();
  for(auto const& str: list) {
    joined.append(sep);
    joined.append(str);
  }
  return joined;
}

QString joinRange(QVector<QString> const& list,
    QString const& sep,
    int begin, int end) {
  if(list.isEmpty()) { return ""; }
  if(begin >= list.size() || end > list.size()) { return ""; }
  if(begin == end) { return ""; }

  QString joined = list[begin];
  for(auto idx = begin + 1; idx < end; idx++) {
    joined.append(sep);
    joined.append(list[idx]);
  }
  return joined;
}

QString makeSigText(
    Signature::SigInfo const& signature,
    Signature::SeperationChars const& sep) {
  auto seperator = QString("%1 ").arg(sep[2]);
  auto text = QString("%1%2%3%4")
    .arg(signature.label)
    .arg(sep[0])
    .arg(join(signature.params, seperator))
    .arg(sep[1]);
  return text;
}

QString makeActiveSigText(
    Signature::SigInfo const& signature,
    Signature::SeperationChars const& sep,
    int active_param) {
  if(active_param < 0) {
    return makeSigText(signature, sep);
  }
  auto seperator = QString("%1 ").arg(sep[2]);
  auto text = QString("%1%2%3%4%5%6%7%8")
    .arg(signature.label)
    .arg(sep[0])
    .arg(joinRange(signature.params, seperator, 0, active_param))
    .arg(active_param > 0 ? seperator : "")
    .arg(QString("<font style=\"text-decoration:underline\" color=\"red\">%1</font>")
        .arg(signature.params[active_param]))
    .arg(active_param + 1 < signature.params.size() ? seperator : "")
    .arg(joinRange(signature.params, seperator, active_param + 1, signature.params.size()))
    .arg(sep[1]);
  return text;
}

void Signature::show(QVector<SigInfo> signatures,
      int active_signature,
      int active_param,
      SeperationChars const& sep) {
  widget->clear();
  for(auto idx = 0; idx < signatures.size(); idx++) {
    auto label = (idx != active_signature) ?
      makeSigText(signatures[idx], sep) :
      makeActiveSigText(signatures[idx], sep, active_param);
    widget->addItem(label);
  }

  static auto const offset = 2u;
  auto cellSize = getCellSize();
  auto cursorPos = getCursorPos();

  widget->showAt(cursorPos.x()*cellSize.width(), (cursorPos.y()-signatures.size())*cellSize.height() - offset);
  qDebug() << "+++JS: signature show";
}

void Signature::hide() {
  qDebug() << "+++JS: signature hide";
  widget->hide();
}

SignatureDecoding::SignatureDecoding(QWidget* parent,
                                     Signature::GetCellSize getCellSize,
                                     Signature::GetCursorPos cursorPosGetter)
  : m_signature(parent, getCellSize, cursorPosGetter) {
}

Signature::SigInfo toSigInfo(QVariantMap const& from) {
  if(!from.contains("label") || !from.contains("params")) {
    throw ConversionError("Missing mandatory values in signature desciption");
  }
  Signature::SigInfo sig;
  sig.label = getValue<QString>(from["label"]);
  sig.params = toVect<QString>(getValue<QVariantList>(from["params"]));
  return sig;
}

void SignatureDecoding::show(QVariantList const& args) {
  if(args.size() != 5) {
    qWarning() << "Invalid number of args: expected 4, received: " << args.size()-1;
  }
  try {
    m_signature.show(
        toVect<Signature::SigInfo>(getValue<QVariantList>(args[1]),
          [](QVariant const& val){ return toSigInfo(getValue<QVariantMap>(val)); }),
        getValue<int>(args[2]),
        getValue<int>(args[3]),
        toVect<QString>(getValue<QVariantList>(args[4])));
  } catch (ConversionError const& e) {
    qWarning() << "Failed to parse signature show args: " << args << ", what: " << e.what();
  }
}

void SignatureDecoding::hide() {
  m_signature.hide();
}

}

