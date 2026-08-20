#include <CQSandboxToolbar3D.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxControl3D.h>
#include <CQSandboxOverview3D.h>
#include <CQSandboxApp.h>

#include <CQIconButton.h>
#include <CQPixmapCache.h>

#include <QMenu>
#include <QLabel>
#include <QHBoxLayout>

namespace CQSandbox {

CanvasToolbar3D::
CanvasToolbar3D(Canvas3D *canvas) :
 QFrame(canvas), canvas_(canvas)
{
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  auto *layout = new QHBoxLayout(this);

  //---

  auto addToolButton = [&](const QString &name, const QString &iconName,
                           const QString &tip, const char *slotName) {
    auto *button = new CQIconButton;

    button->setObjectName(name);
    button->setIcon(iconName);
    button->setIconSize(QSize(32, 32));
    button->setAutoRaise(true);
    button->setToolTip(tip);
    button->setFocusPolicy(Qt::NoFocus);

    connect(button, SIGNAL(clicked()), this, slotName);

    return button;
  };

  auto addCheckButton = [&](const QString &name, const QString &iconName,
                            const QString &tip, const char *slotName) {
    auto *button = new CQIconButton;

    button->setObjectName(name);
    button->setCheckable(true);
    button->setIcon(iconName);
    button->setIconSize(QSize(32, 32));
    button->setAutoRaise(true);
    button->setToolTip(tip);
    button->setFocusPolicy(Qt::NoFocus);

    connect(button, SIGNAL(clicked()), this, slotName);

    return button;
  };

  auto addSeparator = [&]() {
    auto *frame = new QFrame;

    frame->setFixedWidth(16);
    frame->setFrameStyle(QFrame::VLine);

    return frame;
  };

  //---

  cameraButton_ = addToolButton("camera", "CAMERA", "Mode: Camera", SLOT(cameraSlot()));
  modelButton_  = addToolButton("model" , "MODEL" , "Mode: Model" , SLOT(modelSlot()));
  lightButton_  = addToolButton("light" , "LIGHT" , "Mode: Light" , SLOT(lightSlot()));
  gameButton_   = addToolButton("game"  , "GAME"  , "Mode: Game"  , SLOT(gameSlot()));

  layout->addWidget(cameraButton_);
  layout->addWidget(modelButton_);
  layout->addWidget(lightButton_);
  layout->addWidget(gameButton_);

  layout->addWidget(addSeparator());

  //---

  wireButton_     =
    addCheckButton("wire"    , "WIREFRAME"   , "Shade: Wireframe", SLOT(wireSlot()));
  solidButton_    =
    addCheckButton("solid"   , "SOLID_FILL"  , "Shade: Solid"    , SLOT(solidSlot()));
  texturedButton_ =
    addCheckButton("textured", "TEXTURE_FILL", "Shade: Textured" , SLOT(texturedSlot()));

  //---

  layout->addWidget(wireButton_);
  layout->addWidget(solidButton_);
  layout->addWidget(texturedButton_);

  layout->addWidget(addSeparator());

  //---

  bboxButton_ = addCheckButton("bbox", "BBOX", "Show: Model", SLOT(bboxSlot()));

  layout->addWidget(bboxButton_);

  layout->addWidget(addSeparator());

  //---

  infoLabel_ = new QLabel(" ");

  layout->addWidget(infoLabel_);

  posLabel_ = new QLabel(" ");

  layout->addWidget(posLabel_);

  layout->addStretch(1);

  //---

  settingsButton_ = addCheckButton("settings", "SETTINGS" , "Settings", SLOT(settingsSlot()));

  layout->addWidget(settingsButton_);

  //---

  auto *debugButton = addDebugButton();

  layout->addWidget(debugButton);

  //---

  updateInfo();
}

QToolButton *
CanvasToolbar3D::
addDebugButton()
{
  int is = QFontMetrics(font()).height() + 6;

  auto *button = new QToolButton;

  button->setIcon(CQPixmapCacheInst->getIcon("MENU"));
  button->setPopupMode(QToolButton::InstantPopup);

  button->setAutoRaise(true);
  button->setIconSize(QSize(is, is));

  auto *menu = new QMenu;

  auto *action1 = menu->addAction("Meta Edit");
  auto *action2 = menu->addAction("Performance");
  auto *action3 = menu->addAction("Options");

  connect(action1, SIGNAL(triggered()), this, SLOT(metaEditSlot()));
  connect(action2, SIGNAL(triggered()), this, SLOT(performanceSlot()));
  connect(action3, SIGNAL(triggered()), this, SLOT(optionsSlot()));

  button->setMenu(menu);

  return button;
}

void
CanvasToolbar3D::
setInfo(const QString &label)
{
  infoLabel_->setText(label);
}

void
CanvasToolbar3D::
setPos(const QString &label)
{
  posLabel_->setText(label);
}

void
CanvasToolbar3D::
updateInfo()
{
  auto type = canvas_->type();

  QString text;

  if      (type == Canvas3D::Type::CAMERA) {
    text += "Mode: Camera";
  }
  else if (type == Canvas3D::Type::LIGHT) {
    text += "Mode: Light";
    text += " #" + QString::number(canvas_->lightNum());
  }
  else if (type == Canvas3D::Type::MODEL) {
    text += "Mode: Model";
  }
  else if (type == Canvas3D::Type::GAME) {
    text += "Mode: Game";
  }

  infoLabel_->setText(text);
}

void
CanvasToolbar3D::
cameraSlot()
{
  canvas_->setType(Canvas3D::Type::CAMERA);
}

void
CanvasToolbar3D::
modelSlot()
{
  canvas_->setType(Canvas3D::Type::MODEL);
}

void
CanvasToolbar3D::
lightSlot()
{
  canvas_->setType(Canvas3D::Type::LIGHT);
}

void
CanvasToolbar3D::
gameSlot()
{
  canvas_->setType(Canvas3D::Type::GAME);
}

void
CanvasToolbar3D::
wireSlot()
{
  auto *button = qobject_cast<CQIconButton *>(sender());

  canvas_->setWireframe(button->isChecked());

  canvas_->update();
}

void
CanvasToolbar3D::
solidSlot()
{
  auto *button = qobject_cast<CQIconButton *>(sender());

  canvas_->setSolid(button->isChecked());

  canvas_->update();
}

void
CanvasToolbar3D::
texturedSlot()
{
  auto *button = qobject_cast<CQIconButton *>(sender());

  canvas_->setTextured(button->isChecked());

  canvas_->update();
}

void
CanvasToolbar3D::
bboxSlot()
{
  auto *button = qobject_cast<CQIconButton *>(sender());

  canvas_->setShowBBox(button->isChecked());

  canvas_->update();
}

void
CanvasToolbar3D::
settingsSlot()
{
  auto *app     = canvas()->app();
  auto *control = app->control3D();

  control->toggleShown();
}

void
CanvasToolbar3D::
metaEditSlot()
{
  auto *app = canvas()->app();

  app->showMetaEdit();
}

void
CanvasToolbar3D::
performanceSlot()
{
  auto *app = canvas()->app();

  app->showPerfDialog();
}

void
CanvasToolbar3D::
optionsSlot()
{
  auto *app = canvas()->app();

  app->showAppOptions();
}

//---

OverviewToolbar3D::
OverviewToolbar3D(Overview3D *overview) :
 QFrame(overview), overview_(overview)
{
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  auto *layout = new QHBoxLayout(this);

  //---

  auto addToolButton = [&](const QString &name, const QString &iconName,
                           const QString &tip, const char *slotName) {
    auto *button = new CQIconButton;

    button->setObjectName(name);
    button->setIcon(iconName);
    button->setIconSize(QSize(32, 32));
    button->setAutoRaise(true);
    button->setToolTip(tip);
    button->setFocusPolicy(Qt::NoFocus);

    connect(button, SIGNAL(clicked()), this, slotName);

    return button;
  };

  auto addSeparator = [&]() {
    auto *frame = new QFrame;

    frame->setFixedWidth(16);
    frame->setFrameStyle(QFrame::VLine);

    return frame;
  };

  auto addCheckButton = [&](const QString &name, const QString &iconName,
                            const QString &tip, const char *slotName) {
    auto *button = new CQIconButton;

    button->setObjectName(name);
    button->setCheckable(true);
    button->setIcon(iconName);
    button->setIconSize(QSize(32, 32));
    button->setAutoRaise(true);
    button->setToolTip(tip);
    button->setFocusPolicy(Qt::NoFocus);

    connect(button, SIGNAL(clicked()), this, slotName);

    return button;
  };

  //---

  cameraButton_ = addToolButton("camera", "CAMERA", "Mode: Camera", SLOT(cameraSlot()));
  modelButton_  = addToolButton("model" , "MODEL" , "Mode: Model" , SLOT(modelSlot()));
  lightButton_  = addToolButton("light" , "LIGHT" , "Mode: Light" , SLOT(lightSlot()));

  layout->addWidget(cameraButton_);
  layout->addWidget(modelButton_);
  layout->addWidget(lightButton_);

  layout->addWidget(addSeparator());

  //---

  objectSelectButton_  =
    addToolButton("objectSelect", "OBJECT_SELECT", "Object Select", SLOT(objectSelectSlot()));
  faceSelectButton_  =
    addToolButton("faceSelect"  , "FACE_SELECT"  , "Face Select"  , SLOT(faceSelectSlot()));
  edgeSelectButton_  =
    addToolButton("edgeSelect"  , "EDGE_SELECT"  , "Edge Select"  , SLOT(edgeSelectSlot()));
  pointSelectButton_ =
    addToolButton("pointSelect" , "POINT_SELECT" , "Point Select" , SLOT(pointSelectSlot()));

  layout->addWidget(objectSelectButton_);
  layout->addWidget(faceSelectButton_);
  layout->addWidget(edgeSelectButton_);
  layout->addWidget(pointSelectButton_);

  layout->addWidget(addSeparator());

  //---

  infoLabel_ = new QLabel(" ");

  layout->addWidget(infoLabel_);

  posLabel_ = new QLabel(" ");

  layout->addWidget(posLabel_);

  layout->addStretch(1);

  //---

  settingsButton_ = addCheckButton("settings", "SETTINGS" , "Settings", SLOT(settingsSlot()));

  layout->addWidget(settingsButton_);
}

void
OverviewToolbar3D::
cameraSlot()
{
  overview_->setEditType(CQSandbox::Overview3D::EditType::CAMERA);
}

void
OverviewToolbar3D::
modelSlot()
{
  overview_->setEditType(CQSandbox::Overview3D::EditType::SELECT);
}

void
OverviewToolbar3D::
lightSlot()
{
  overview_->setEditType(CQSandbox::Overview3D::EditType::LIGHT);
}

void
OverviewToolbar3D::
objectSelectSlot()
{
  overview_->setSelectType(CQSandbox::Overview3D::SelectType::OBJECT);
}

void
OverviewToolbar3D::
faceSelectSlot()
{
  overview_->setSelectType(CQSandbox::Overview3D::SelectType::FACE);
}

void
OverviewToolbar3D::
edgeSelectSlot()
{
  overview_->setSelectType(CQSandbox::Overview3D::SelectType::EDGE);
}

void
OverviewToolbar3D::
pointSelectSlot()
{
  overview_->setSelectType(CQSandbox::Overview3D::SelectType::POINT);
}

void
OverviewToolbar3D::
settingsSlot()
{
  auto *app     = overview_->app();
  auto *control = app->control3D();

  control->toggleShown();
}

}
