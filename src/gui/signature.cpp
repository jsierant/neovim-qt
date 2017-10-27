
#include "signature.h"
#include <QDebug>

namespace NeovimQt {

Signature::Signature(QWidget*) {
}

void Signature::show(SignatureDef,
      std::uint32_t active_signature,
      int active_param) {
  qDebug() << "+++JS: signature show";
}

void Signature::hide() {
  qDebug() << "+++JS: signature hide";
}

SignatureDecoding::SignatureDecoding(QWidget* parent)
  : m_signature(parent) {
}

void SignatureDecoding::show(QVariant const&) {
  qDebug() << "+++JS: signature decode show";
}

void SignatureDecoding::hide() {
  m_signature.hide();
}

}

