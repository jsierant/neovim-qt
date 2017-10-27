
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

void SignatureDecoding::show(QVariantList const& args) {
  if(args.size() != 5) {
    qDebug() << "Invalid number of args: expected 4, received: " << args.size() ;
  }

  qDebug() << "+++JS: signature decode show: " << args;
}

void SignatureDecoding::hide() {
  m_signature.hide();
}

}

