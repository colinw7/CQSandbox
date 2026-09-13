#include <CQSandboxApp.h>
#include <CQSandboxCanvas2D.h>
#include <CQSandboxCanvas3D.h>
#include <CQApp.h>

int
main(int argc, char **argv)
{
  setenv("QT_SCALE_FACTOR", "1", 1);

#ifdef NO_QT_APP
  QApplication qapp(argc, argv);
#else
  CQApp qapp(argc, argv);
#endif

  qapp.setFont(QFont("Sans", 20));

  auto *app = new CQSandbox::App;

  app->resize(2000, 1500);

  QString filename;
  QString modelName;
  bool    is3D     { false };
  bool    overview { false };

  for (int i = 1; i < argc; ++i) {
    auto arg = QString(argv[i]);

    if (arg.left(1) == '-') {
      if      (arg == "-3d")
        is3D = true;
      else if (arg == "-model") {
        ++i;

        if (i < argc) {
          modelName = argv[i];
          is3D      = true ;
        }
        else
          std::cerr << "Missing model\n";
      }
      else if (arg == "-overview")
        overview = true;
      else
        std::cerr << "Invalid option '" << argv[i] << "\n";
    }
    else
      filename = arg;
  }

  if (is3D)
    app->set3D(true);

  if (overview)
    app->setOverview(true);

  app->init();

  if (filename != "") {
    if      (app->canvas()) {
      if (! app->load(app->canvas()->tcl(), filename))
        std::cerr << "Failed to load '" << filename.toStdString() << "'\n";
    }
    else if (app->canvas3D()) {
      if (! app->load(app->canvas3D()->tcl(), filename))
        std::cerr << "Failed to load '" << filename.toStdString() << "'\n";
    }
  }

  if (modelName != "") {
    auto cmd = QString("\
proc init { } {\n\
  set ::model [sb3d::model {%1}]\n\
}\n\
proc bboxChanged { } {\n\
  sb3d::light exec reset 1\n\
  sb3d::camera exec reset\n\
}").arg(modelName);

    app->canvas3D()->runTclCmd(cmd);
  }

  app->show();

  return qapp.exec();
}
