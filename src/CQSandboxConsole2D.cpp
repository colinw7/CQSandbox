#include <CQSandboxConsole2D.h>
#include <CQSandboxCanvas2D.h>
#include <CQSandboxApp.h>

#include <CQTclCommand.h>

#include <QHBoxLayout>

namespace CQSandbox {

Console2D::
Console2D(Canvas2D *canvas) :
 QFrame(nullptr), canvas_(canvas)
{
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  auto *layout = new QHBoxLayout(this);
  layout->setMargin(0); layout->setSpacing(2);

  auto *area = new CQTclCommand::ScrollArea;

  auto *command = area->getCommand();

  command->setMinLines(10);
  command->setPrompt("> ");

  area->setTcl(canvas_->tcl());

  connect(area, SIGNAL(executeCommand(const QString &)),
          this, SLOT(executeCommand(const QString &)));

  layout->addWidget(area);

  area_ = area;
}

void
Console2D::
executeCommand(const QString &line)
{
  auto *area = qobject_cast<CQTclCommand::ScrollArea *>(area_);

  area->executeCommand(line);
}

QSize
Console2D::
sizeHint() const
{
  return QSize(1200, 1600);
}

}
