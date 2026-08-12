#ifndef CQSandboxStatus_H
#define CQSandboxStatus_H

#include <QFrame>

class QLabel;

namespace CQSandbox {

class App;

class Status : public QFrame {
  Q_OBJECT

 public:
  Status(App *app);

  void setText(const QString &text);

 private:
  App*    app_   { nullptr };
  QLabel* label_ { nullptr };
};

}

#endif
