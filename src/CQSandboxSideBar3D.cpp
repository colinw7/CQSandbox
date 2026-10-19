#include <CQSandboxSideBar3D.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUI.h>

#include <QVBoxLayout>

namespace CQSandbox {

SideBar3D::
SideBar3D(Canvas3D *canvas) :
 QFrame(canvas), canvas_(canvas)
{
  setObjectName("sidebar");

  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

  //---

  auto *layout = new QVBoxLayout(this);
  layout->setMargin(0); layout->setSpacing(2);

  //---

  UI ui(this, layout);

  auto addIconCheckButton = [&](const QString &name, const QString &iconName, const QString &tip) {
    auto *button = ui.addIconCheckButton(name, iconName, tip);

    //checkButtons_.push_back(button);

    return button;
  };

  // edit mode buttons
  moveButton_   = addIconCheckButton("move"  , "MOVE"  , "Move"  );
  rotateButton_ = addIconCheckButton("rotate", "ROTATE", "Rotate");
  scaleButton_  = addIconCheckButton("scale" , "SCALE" , "Scale" );

  ui.addStretch();

  connectSlots(true);
}

void
SideBar3D::
connectSlots(bool b)
{
  CQUtil::connectDisconnect(b,
    canvas_, SIGNAL(editModeChanged()), this, SLOT(updateButtonState()));

  CQUtil::connectDisconnect(b,
    moveButton_, SIGNAL(toggled(bool)), this, SLOT(moveSlot(bool)));
  CQUtil::connectDisconnect(b,
    rotateButton_, SIGNAL(toggled(bool)), this, SLOT(rotateSlot(bool)));
  CQUtil::connectDisconnect(b,
    scaleButton_, SIGNAL(toggled(bool)), this, SLOT(scaleSlot(bool)));
}

void
SideBar3D::
moveSlot(bool)
{
  if (canvas()->editMode() != Canvas3D::EditMode::MOVE)
    canvas_->setEditMode(Canvas3D::EditMode::MOVE);

  updateButtonState();
}

void
SideBar3D::
rotateSlot(bool)
{
  if (canvas()->editMode() != Canvas3D::EditMode::ROTATE)
    canvas_->setEditMode(Canvas3D::EditMode::ROTATE);

  updateButtonState();
}

void
SideBar3D::
scaleSlot(bool)
{
  if (canvas()->editMode() != Canvas3D::EditMode::SCALE)
    canvas_->setEditMode(Canvas3D::EditMode::SCALE);

  updateButtonState();
}

void
SideBar3D::
updateButtonState()
{
  connectSlots(false);

  auto editMode = canvas()->editMode();

  moveButton_  ->setChecked(editMode == Canvas3D::EditMode::MOVE);
  rotateButton_->setChecked(editMode == Canvas3D::EditMode::ROTATE);
  scaleButton_ ->setChecked(editMode == Canvas3D::EditMode::SCALE);

  connectSlots(true);
}

}
