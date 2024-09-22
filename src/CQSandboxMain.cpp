#include <CQSandboxApp.h>
#include <CQSandboxCanvas.h>

#include <QApplication>

int
main(int argc, char **argv)
{
  QApplication qapp(argc, argv);

  qapp.setFont(QFont("Sans", 20));

  auto *app = new CQSandbox::App;

  app->resize(2000, 1500);

  QString filename;
  bool    is3D { false };

  for (int i = 1; i < argc; ++i) {
    auto arg = QString(argv[i]);

    if (arg.left(1) == '-') {
      if (arg == "-3d")
        is3D = true;
    }
    else
      filename = arg;
  }

  if (is3D)
    app->set3D(true);

  app->init();

  if (filename != "")
    app->load(filename);

  app->show();

  return qapp.exec();
}
