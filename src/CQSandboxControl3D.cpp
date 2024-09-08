#include <CQSandboxControl3D.h>
#include <CQSandboxCanvas3D.h>

#include <CQColorEdit.h>

#include <QVBoxLayout>

namespace CQSandbox {

CanvasControl3D::
CanvasControl3D(CQSandbox::Canvas3D *canvas) :
 QFrame(nullptr), canvas_(canvas)
{
  auto *layout = new QVBoxLayout(this);

  auto *colorEdit = new CQColorEdit(this);

  colorEdit->setColor(canvas_->bgColor());

  connect(colorEdit, &CQColorEdit::colorChanged, this, &CanvasControl3D::bgColorSlot);

  layout->addWidget(colorEdit);
}

void
CanvasControl3D::
bgColorSlot(const QColor &c)
{
  canvas_->setBgColor(c);
  canvas_->update();
}

}
