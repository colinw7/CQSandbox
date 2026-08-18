#include <CQSandboxStatus.h>

#include <QLabel>
#include <QHBoxLayout>

namespace CQSandbox {

Status::
Status(App *app) :
 app_(app)
{
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  auto *layout = new QHBoxLayout(this);
  layout->setMargin(0); layout->setSpacing(2);

  label_ = new QLabel(" ");

  layout->addWidget(label_);

  mouseLabel_ = new QLabel(" ");

  layout->addWidget(mouseLabel_);
}

void
Status::
setText(const QString &str)
{
  label_->setText(str);
}

void
Status::
setMouseLabel(const QString &str)
{
  mouseLabel_->setText(str);
}

}
