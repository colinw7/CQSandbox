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

  layout->addWidget(wireButton_);
  layout->addWidget(solidButton_);
  layout->addWidget(texturedButton_);

  layout->addWidget(addSeparator());

  //---

  pointSelectButton_ =
    addCheckButton("pointSelect" , "POINT_SELECT" , "Point Select" , SLOT(pointSelectSlot()));
  edgeSelectButton_  =
    addCheckButton("edgeSelect"  , "EDGE_SELECT"  , "Edge Select"  , SLOT(edgeSelectSlot()));
  faceSelectButton_  =
    addCheckButton("faceSelect"  , "FACE_SELECT"  , "Face Select"  , SLOT(faceSelectSlot()));
  objectSelectButton_  =
    addCheckButton("objectSelect", "OBJECT_SELECT", "Object Select", SLOT(objectSelectSlot()));

  layout->addWidget(pointSelectButton_);
  layout->addWidget(edgeSelectButton_);
  layout->addWidget(faceSelectButton_);
  layout->addWidget(objectSelectButton_);

  layout->addWidget(addSeparator());

  //---

  bboxButton_ = addCheckButton("bbox", "BBOX", "Show: BBox", SLOT(bboxSlot()));

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

  connect(canvas_, SIGNAL(typeChanged()), this, SLOT(updateInfo()));
  connect(canvas_, SIGNAL(lightChanged()), this, SLOT(updateInfo()));
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

#ifdef CQSANDBOX_META_EDIT
  auto *action1 = menu->addAction("Meta Edit");
  connect(action1, SIGNAL(triggered()), this, SLOT(metaEditSlot()));
#endif

  auto *action2 = menu->addAction("Performance");
  connect(action2, SIGNAL(triggered()), this, SLOT(performanceSlot()));

  auto *action3 = menu->addAction("Options");
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
  disconnect(cameraButton_, SIGNAL(clicked()), this, SLOT(cameraSlot()));
  disconnect(modelButton_ , SIGNAL(clicked()), this, SLOT(modelSlot()));
  disconnect(lightButton_ , SIGNAL(clicked()), this, SLOT(lightSlot()));
  disconnect(gameButton_  , SIGNAL(clicked()), this, SLOT(gameSlot()));

  disconnect(wireButton_    , SIGNAL(clicked()), this, SLOT(wireSlot()));
  disconnect(solidButton_   , SIGNAL(clicked()), this, SLOT(solidSlot()));
  disconnect(texturedButton_, SIGNAL(clicked()), this, SLOT(texturedSlot()));

  disconnect(pointSelectButton_ , SIGNAL(clicked()), this, SLOT(pointSelectSlot()));
  disconnect(edgeSelectButton_  , SIGNAL(clicked()), this, SLOT(edgeSelectSlot()));
  disconnect(faceSelectButton_  , SIGNAL(clicked()), this, SLOT(faceSelectSlot()));
  disconnect(objectSelectButton_, SIGNAL(clicked()), this, SLOT(objectSelectSlot()));

  //---

  auto type = canvas_->type();

  cameraButton_->setChecked(type == Canvas3D::Type::CAMERA);
  modelButton_ ->setChecked(type == Canvas3D::Type::MODEL);
  lightButton_ ->setChecked(type == Canvas3D::Type::LIGHT);
  gameButton_  ->setChecked(type == Canvas3D::Type::GAME);

  wireButton_    ->setChecked(canvas_->isWireframe());
  solidButton_   ->setChecked(canvas_->isSolid());
  texturedButton_->setChecked(canvas_->isTextured());

  objectSelectButton_->setVisible(type == Canvas3D::Type::MODEL);
  faceSelectButton_  ->setVisible(type == Canvas3D::Type::MODEL);
  edgeSelectButton_  ->setVisible(type == Canvas3D::Type::MODEL);
  pointSelectButton_ ->setVisible(type == Canvas3D::Type::MODEL);

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

    auto editMode = canvas_->editMode();
    auto editType = canvas_->editType();

    if      (editMode == Canvas3D::EditMode::MOVE)
      text += " (Move)";
    else if (editMode == Canvas3D::EditMode::SCALE)
      text += " (Scale)";
    else if (editMode == Canvas3D::EditMode::ROTATE)
      text += " (Rotate)";

    if      (editType == Canvas3D::EditType::POINT)
      text += " (Point)";
    else if (editType == Canvas3D::EditType::LINE)
      text += " (Line)";
    else if (editType == Canvas3D::EditType::FACE)
      text += " (Face)";

    objectSelectButton_->setChecked(editType == Canvas3D::EditType::OBJECT);
    faceSelectButton_  ->setChecked(editType == Canvas3D::EditType::FACE);
    edgeSelectButton_  ->setChecked(editType == Canvas3D::EditType::LINE);
    pointSelectButton_ ->setChecked(editType == Canvas3D::EditType::POINT);
  }
  else if (type == Canvas3D::Type::GAME) {
    text += "Mode: Game";
  }

  text += " Light: " + QString::number(canvas_->lightNum());

  if (canvas_->isSimpleLights())
    text += " (Simple)";

  infoLabel_->setText(text);

  //---

  connect(cameraButton_, SIGNAL(clicked()), this, SLOT(cameraSlot()));
  connect(modelButton_ , SIGNAL(clicked()), this, SLOT(modelSlot()));
  connect(lightButton_ , SIGNAL(clicked()), this, SLOT(lightSlot()));
  connect(gameButton_  , SIGNAL(clicked()), this, SLOT(gameSlot()));

  connect(wireButton_    , SIGNAL(clicked()), this, SLOT(wireSlot()));
  connect(solidButton_   , SIGNAL(clicked()), this, SLOT(solidSlot()));
  connect(texturedButton_, SIGNAL(clicked()), this, SLOT(texturedSlot()));

  connect(objectSelectButton_, SIGNAL(clicked()), this, SLOT(objectSelectSlot()));
  connect(faceSelectButton_  , SIGNAL(clicked()), this, SLOT(faceSelectSlot()));
  connect(edgeSelectButton_  , SIGNAL(clicked()), this, SLOT(edgeSelectSlot()));
  connect(pointSelectButton_ , SIGNAL(clicked()), this, SLOT(pointSelectSlot()));
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
objectSelectSlot()
{
  canvas_->setEditType(Canvas3D::EditType::OBJECT);
}

void
CanvasToolbar3D::
faceSelectSlot()
{
  canvas_->setEditType(Canvas3D::EditType::FACE);
}

void
CanvasToolbar3D::
edgeSelectSlot()
{
  canvas_->setEditType(Canvas3D::EditType::LINE);
}

void
CanvasToolbar3D::
pointSelectSlot()
{
  canvas_->setEditType(Canvas3D::EditType::POINT);
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

#ifdef CQSANDBOX_META_EDIT
void
CanvasToolbar3D::
metaEditSlot()
{
  auto *app = canvas()->app();

  app->showMetaEdit();
}
#endif

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

  //---

  updateInfo();

  connect(overview_, SIGNAL(editTypeChanged()), this, SLOT(updateInfo()));
  connect(overview_, SIGNAL(selectTypeChanged()), this, SLOT(updateInfo()));
}

void
OverviewToolbar3D::
cameraSlot()
{
  overview_->setEditType(Overview3D::EditType::CAMERA);
}

void
OverviewToolbar3D::
modelSlot()
{
  overview_->setEditType(Overview3D::EditType::SELECT);
}

void
OverviewToolbar3D::
lightSlot()
{
  overview_->setEditType(Overview3D::EditType::LIGHT);
}

void
OverviewToolbar3D::
objectSelectSlot()
{
  overview_->setSelectType(Overview3D::SelectType::OBJECT);
}

void
OverviewToolbar3D::
faceSelectSlot()
{
  overview_->setSelectType(Overview3D::SelectType::FACE);
}

void
OverviewToolbar3D::
edgeSelectSlot()
{
  overview_->setSelectType(Overview3D::SelectType::EDGE);
}

void
OverviewToolbar3D::
pointSelectSlot()
{
  overview_->setSelectType(Overview3D::SelectType::POINT);
}

void
OverviewToolbar3D::
settingsSlot()
{
  auto *app     = overview_->app();
  auto *control = app->control3D();

  control->toggleShown();
}

void
OverviewToolbar3D::
updateInfo()
{
  auto type = overview_->editType();

  auto *canvas = overview_->app()->canvas3D();

  QString text;

  if      (type == Overview3D::EditType::SELECT) {
    text += "Mode: Select";
  }
  else if (type == Overview3D::EditType::CAMERA) {
    text += "Mode: Camera";
  }
  else if (type == Overview3D::EditType::LIGHT) {
    text += "Mode: Light";
    text += " #" + QString::number(canvas->lightNum());
  }

  infoLabel_->setText(text);
}

}
