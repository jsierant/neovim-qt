
#include "signature.h"
#include "args_conv.h"

#include <QDebug>
#include <QVariantMap>
#include <QFontMetrics>
#include <QListWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QObjectCleanupHandler>
#include <memory>

namespace NeovimQt {


class LabelVList {
public:
  LabelVList(QWidget* p)
    : parent(p),
      widget(new QFrame(p)),
      layout(new QVBoxLayout(widget)),
      size(0, 0) {
    widget->hide();
    widget->setContentsMargins(0, 0, 0, 0);
    widget->setSizePolicy(QSizePolicy::Maximum,
                          QSizePolicy::Maximum);
    widget->setFocusPolicy(Qt::NoFocus);
    widget->setStyleSheet(QString(
          "color: %1"
          "; background-color: %2")
        .arg("#fdf4c1")
        .arg("#494949") );
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    widget->setLayout(layout);
    widget->setLineWidth(0);
  }

  QFrame* createSeperator() {
    auto* line = new QFrame(widget);
    line->setFrameShape(QFrame::HLine); // Horizontal line
    line->setFrameShadow(QFrame::Plain);
    line->setLineWidth(1);
    return line;
  }

  void addItem(QString const& text, bool last,
      Qt::TextFormat format = Qt::RichText) {
    auto* label = new QLabel(text);
    label->setFrameStyle(QFrame::Panel | QFrame::Raised);

    auto font = parent->font();
    label->setFont(font);
    label->setTextFormat(format);
    layout->addWidget(label);
    auto fm = QFontMetrics(font);
    size = QSize(std::max(size.width(), label->minimumSizeHint().width()),
                 fm.height() + size.height()+3);
  }

  void clear() {
    size = QSize(0, 0);
    while(layout->count()) {
      delete layout->takeAt(0);
    }
  }

  int count() const { return layout->count(); }

  bool isVisible() { return widget->isVisible(); }
  void move(int x, int y) { widget->move(x, y); }

  void show() {
    widget->setMaximumHeight(size.height());
    widget->setMaximumWidth(size.width());
    widget->setMinimumHeight(size.height());
    widget->setMinimumWidth(size.width());
    widget->show();
  }

  void hide() {
    widget->hide();
  }

  void setStyle(Signature::StyleDef const&) {}

private:
  QWidget* parent;
  QFrame* widget;
  QVBoxLayout* layout;
  QFrame* frame;
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
    Signature::Seperators const& sep) {
  auto seperator = QString("%1 ").arg(sep.sep);
  auto text = QString("%1%2%3%4")
    .arg(signature.label)
    .arg(sep.start)
    .arg(join(signature.params, seperator))
    .arg(sep.stop);
  return text;
}

QString makeActiveSigText(
    Signature::SigInfo const& signature,
    Signature::Seperators const& sep,
    int active_param) {
  if(active_param < 0) {
    return makeSigText(signature, sep);
  }
  auto seperator = QString("%1 ").arg(sep.sep);
  auto text = QString("%1%2%3%4%5%6%7%8")
    .arg(signature.label)
    .arg(sep.start)
    .arg(joinRange(signature.params, seperator, 0, active_param))
    .arg(active_param > 0 ? seperator : "")
    .arg(QString("<font style=\"text-decoration:underline\" color=\"red\">%1</font>")
        .arg(signature.params[active_param]))
    .arg(active_param + 1 < signature.params.size() ? seperator : "")
    .arg(joinRange(signature.params, seperator, active_param + 1, signature.params.size()))
    .arg(sep.stop);
  return text;
}

void Signature::show(QVector<SigInfo> const& signatures,
      int active_signature,
      int active_param,
      Seperators const& sep) {

  widget->clear();
  addItemsToWidget(signatures,
      active_signature,
      active_param, sep);
  moveAndShowWidget();
}

void Signature::addItemsToWidget(
    QVector<SigInfo> const& signatures,
    int active_signature,
    int active_param,
    Seperators const& sep) {
  for(auto idx = 0; idx < signatures.size(); idx++) {
    auto text = (idx != active_signature) ?
      makeSigText(signatures[idx], sep) :
      makeActiveSigText(signatures[idx], sep, active_param);
    widget->addItem(text, idx == signatures.size()-1);
  }
}

void Signature::moveAndShowWidget() {
  if(!widget->isVisible()) {
    static auto const offset = 6u;
    auto cellSize = getCellSize();
    auto cursorPos = getCursorPos();
    widget->move(cursorPos.x()*cellSize.width(),
        (cursorPos.y()-widget->count())*cellSize.height() - offset);
  }
  widget->show();
}

void Signature::hide() {
  widget->hide();
}


void Signature::setStyle(StyleDef const& style) {
  widget->setStyle(style);
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

Signature::Seperators toSeperators(QVariantMap const& from) {
  if(!from.contains("start") || !from.contains("stop") || !from.contains("sep")) {
    throw ConversionError("Missing mandatory values in seperators");
  }
  Signature::Seperators sep;
  sep.start = getValue<QString>(from["start"]);
  sep.stop = getValue<QString>(from["stop"]);
  sep.sep = getValue<QString>(from["sep"]);
  return sep;
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
        toSeperators(getValue<QVariantMap>(args[4])));
  } catch (ConversionError const& e) {
    qWarning() << "Failed to parse signature show args: " << args << ", what: " << e.what();
  }
}

void SignatureDecoding::hide() {
  m_signature.hide();
}

void SignatureDecoding::setStyle(QVariantList const&) {
//   m_signature.setStyle(style);
}

}

