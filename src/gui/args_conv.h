#ifndef NEOVIM_GUI_ARGS
#define NEOVIM_GUI_ARGS

#include <stdexcept>
#include <QVariant>

namespace NeovimQt {

struct ConversionError : std::runtime_error{
  ConversionError(char const* msg)
    : std::runtime_error(msg) {}
};

template<typename T>
T getValue(QVariant const& from) {
  if(from.canConvert<T>()) {
    return from.value<T>();
  }
  throw ConversionError("variant conversion failed");
}

template<typename T>
QVector<T> toVect(QVariantList const& list) {
  QVector<T> converted;
  for(auto const& el: list) {
    converted.push_back(getValue<T>(el));
  }
  return converted;
}

}

#endif

