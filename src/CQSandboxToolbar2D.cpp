#include <CQSandboxToolbar2D.h>
#include <CQSandboxControl2D.h>
#include <CQSandboxCanvas.h>
#include <CQSandboxApp.h>

#include <CQIconButton.h>

#include <QLabel>
#include <QHBoxLayout>

namespace CQSandbox {

Toolbar2D::
Toolbar2D(Canvas *canvas) :
 QFrame(canvas), canvas_(canvas)
{
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  auto *layout = new QHBoxLayout(this);

  auto addToolButton = [&](const QString &name, const QString &iconName,
                           const QString &tip, const char *slotName) {
    auto *button = new CQIconButton;

    button->setObjectName(name);
    button->setIcon(iconName);
    button->setIconSize(QSize(32, 32));
    button->setAutoRaise(true);
    button->setToolTip(tip);

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

    connect(button, SIGNAL(clicked()), this, slotName);

    return button;
  };

  playButton_  = addToolButton("play" , "PLAY"    , "Play" , SLOT(playSlot()));
  pauseButton_ = addToolButton("pause", "PAUSE"   , "Pause", SLOT(pauseSlot()));
  stepButton_  = addToolButton("step" , "PLAY_ONE", "Step" , SLOT(stepSlot()));

  layout->addWidget(playButton_);
  layout->addWidget(pauseButton_);
  layout->addWidget(stepButton_);

  //---

  infoLabel_ = new QLabel(" ");

  layout->addWidget(infoLabel_);

  layout->addStretch(1);

  //---

  settingsButton_ = addCheckButton("settings", "SETTINGS" , "Settings", SLOT(settingsSlot()));

  layout->addWidget(settingsButton_);
}

void
Toolbar2D::
setInfo(const QString &label)
{
  infoLabel_->setText(label);
}

void
Toolbar2D::
playSlot()
{
  canvas()->play();
}

void
Toolbar2D::
pauseSlot()
{
  canvas()->pause();
}

void
Toolbar2D::
stepSlot()
{
  canvas()->step();
}

void
Toolbar2D::
settingsSlot()
{
  auto *button = qobject_cast<CQIconButton *>(sender());

  auto *app = canvas()->app();

  app->control2D()->setVisible(button->isChecked());
}

}
