#ifndef NEOVIM_GUI_SIGNATURE
#define NEOVIM_GUI_SIGNATURE

#include <QString>
#include <QVector>
#include <cstdint>

class QWidget;

namespace NeovimQt {

class Signature {
public:
  Signature(QWidget* parent);
  struct SignatureDef {
    enum SepCharIdx { begin = 0, end, sep };
    QString label;
    QVector<QString> params;
    QVector<QChar> seperation_chars;
  };

  void show(SignatureDef,
      std::uint32_t active_signature,
      int active_param);

  void hide();
};

}

#endif

