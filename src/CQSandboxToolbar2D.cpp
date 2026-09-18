#include <CQSandboxToolbar2D.h>
#include <CQSandboxControl2D.h>
#include <CQSandboxConsole2D.h>
#include <CQSandboxCanvas2D.h>
#include <CQSandboxApp.h>

#include <CQIconButton.h>

#include <QLabel>
#include <QHBoxLayout>

namespace CQSandbox {

CanvasToolbar2D::
CanvasToolbar2D(Canvas2D *canvas) :
 QFrame(canvas), canvas_(canvas)
{
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  auto *layout = new QHBoxLayout(this);
  layout->setMargin(0); layout->setSpacing(2);

  //---

  auto addToolButton = [&](const QString &name, const QString &iconName, const QString &tip) {
    auto *button = new CQIconButton;

    button->setObjectName(name);
    button->setIcon(iconName);
    button->setIconSize(QSize(32, 32));
    button->setAutoRaise(true);
    button->setToolTip(tip);

    return button;
  };

  auto addCheckButton = [&](const QString &name, const QString &iconName, const QString &tip) {
    auto *button = addToolButton(name, iconName, tip);

    button->setCheckable(true);

    return button;
  };

  auto addSeparator = [&]() {
    auto *frame = new QFrame;

    frame->setFixedWidth(16);
    frame->setFrameStyle(QFrame::VLine);

    return frame;
  };

  //---

  cameraButton_ = addCheckButton("view"  , "CAMERA", "Mode: View"  );
  modelButton_  = addCheckButton("object", "MODEL" , "Mode: Object");
  gameButton_   = addCheckButton("user"  , "GAME"  , "Mode: User"  );

  layout->addWidget(cameraButton_);
  layout->addWidget(modelButton_);
  layout->addWidget(gameButton_);

  layout->addWidget(addSeparator());

  //---

  playButton_  = addToolButton("play" , "PLAY"    , "Play" );
//pauseButton_ = addToolButton("pause", "PAUSE"   , "Pause");
  stepButton_  = addToolButton("step" , "PLAY_ONE", "Step" );

  layout->addWidget(playButton_);
//layout->addWidget(pauseButton_);
  layout->addWidget(stepButton_);

  //---

  infoLabel_ = new QLabel(" ");

  layout->addWidget(infoLabel_);

  layout->addStretch(1);

  //---

  settingsButton_ = addCheckButton("settings", "SETTINGS" , "Settings");

  layout->addWidget(settingsButton_);

  consoleButton_ = addCheckButton("console", "CONSOLE" , "Console");

  layout->addWidget(consoleButton_);

  //---

  connectSlots(true);

  connect(canvas_, SIGNAL(typeChanged()), this, SLOT(updateInfo()));
  connect(canvas_, SIGNAL(runStateChanged()), this, SLOT(updateInfo()));

  //---

  updateInfo();
}

void
CanvasToolbar2D::
setInfo(const QString &label)
{
  infoLabel_->setText(label);
}

void
CanvasToolbar2D::
connectSlots(bool b)
{
  if (b) {
    connect(cameraButton_, SIGNAL(clicked()), this, SLOT(cameraSlot()));
    connect(modelButton_ , SIGNAL(clicked()), this, SLOT(modelSlot()));
    connect(gameButton_  , SIGNAL(clicked()), this, SLOT(gameSlot()));

    connect(playButton_ , SIGNAL(clicked()), this, SLOT(playSlot()));
  //connect(pauseButton_, SIGNAL(clicked()), this, SLOT(pauseSlot()));
    connect(stepButton_ , SIGNAL(clicked()), this, SLOT(stepSlot()));

    connect(settingsButton_, SIGNAL(clicked()), this, SLOT(settingsSlot()));
    connect(consoleButton_ , SIGNAL(clicked()), this, SLOT(consoleSlot()));
  }
  else {
    disconnect(cameraButton_, SIGNAL(clicked()), this, SLOT(cameraSlot()));
    disconnect(modelButton_ , SIGNAL(clicked()), this, SLOT(modelSlot()));
    disconnect(gameButton_  , SIGNAL(clicked()), this, SLOT(gameSlot()));

    disconnect(playButton_ , SIGNAL(clicked()), this, SLOT(playSlot()));
  //disconnect(pauseButton_, SIGNAL(clicked()), this, SLOT(pauseSlot()));
    disconnect(stepButton_ , SIGNAL(clicked()), this, SLOT(stepSlot()));

    disconnect(settingsButton_, SIGNAL(clicked()), this, SLOT(settingsSlot()));
    disconnect(consoleButton_ , SIGNAL(clicked()), this, SLOT(consoleSlot()));
  }
}

void
CanvasToolbar2D::
updateInfo()
{
  connectSlots(false);

  //---

//auto *app  = canvas()->app();
  auto  type = canvas_->type();

  cameraButton_->setChecked(type == Canvas2D::Type::CAMERA);
  modelButton_ ->setChecked(type == Canvas2D::Type::MODEL);
  gameButton_  ->setChecked(type == Canvas2D::Type::GAME);

  playButton_->setChecked(canvas_->isRunning());

  QString text;

  if      (type == Canvas2D::Type::CAMERA) {
    text += "Mode: Camera";
  }
  else if (type == Canvas2D::Type::MODEL) {
    text += "Mode: Model";

#if 0
    auto editMode = canvas_->editMode();
    auto editType = canvas_->editType();

    if      (editMode == Canvas2D::EditMode::MOVE)
      text += " (Move)";
    else if (editMode == Canvas2D::EditMode::SCALE)
      text += " (Scale)";
    else if (editMode == Canvas2D::EditMode::ROTATE)
      text += " (Rotate)";

    if      (editType == Canvas2D::EditType::POINT)
      text += " (Point)";
    else if (editType == Canvas2D::EditType::LINE)
      text += " (Line)";
    else if (editType == Canvas2D::EditType::FACE)
      text += " (Face)";

    objectSelectButton_->setChecked(editType == Canvas2D::EditType::OBJECT);
    faceSelectButton_  ->setChecked(editType == Canvas2D::EditType::FACE);
    edgeSelectButton_  ->setChecked(editType == Canvas2D::EditType::LINE);
    pointSelectButton_ ->setChecked(editType == Canvas2D::EditType::POINT);
#endif
  }
  else if (type == Canvas2D::Type::GAME) {
    text += "Mode: Game";
  }

  infoLabel_->setText(text);

  //---

  connectSlots(true);
}

void
CanvasToolbar2D::
cameraSlot()
{
  canvas_->setType(Canvas2D::Type::CAMERA);
}

void
CanvasToolbar2D::
modelSlot()
{
  canvas_->setType(Canvas2D::Type::MODEL);
}

void
CanvasToolbar2D::
gameSlot()
{
  canvas_->setType(Canvas2D::Type::GAME);
}

void
CanvasToolbar2D::
playSlot()
{
  if (! canvas()->isRunning())
    canvas()->play();
  else
    canvas()->pause();
}

#if 0
void
CanvasToolbar2D::
pauseSlot()
{
  canvas()->pause();
}
#endif

void
CanvasToolbar2D::
stepSlot()
{
  canvas()->step();
}

void
CanvasToolbar2D::
settingsSlot()
{
  auto *app     = canvas()->app();
  auto *control = app->control2D();

  control->toggleShown();
}

void
CanvasToolbar2D::
consoleSlot()
{
  auto *app     = canvas()->app();
  auto *console = app->console2D();

  console->show();
}

#ifdef CQSANDBOX_META_EDIT
void
CanvasToolbar2D::
metaEditSlot()
{
  auto *app = canvas()->app();

  app->showMetaEdit();
}
#endif

void
CanvasToolbar2D::
performanceSlot()
{
  auto *app = canvas()->app();

  app->showPerfDialog();
}

}
