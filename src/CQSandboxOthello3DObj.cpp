#include <CQSandboxOthello3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#ifdef CQSANDBOX_OTHELLO
#include <COthello.h>
#endif

namespace CQSandbox {

#ifdef CQSANDBOX_OTHELLO
bool
Othello3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new Othello3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

Othello3DObj::
Othello3DObj(Canvas3D *canvas) :
 Object3D(canvas)
{
  board_ = new COthelloBoard;
}

void
Othello3DObj::
init()
{
  Object3D::init();
}

QVariant
Othello3DObj::
getValue(const QString &name, const QStringList &args)
{
  auto *app = canvas()->app();
  auto *tcl = app->tcl();

  struct Index {
    int ix { -1 };
    int iy { -1 };

    bool isValid() { return (ix >= 0 && iy >= 0); }
  };

  auto argsToIndex = [&]() {
    Index ind;

    if (args.size() < 1) {
      app->errorMsg("Missing index for " + name);
      return ind;
    }

    QStringList strs;
    (void) tcl->splitList(args[0], strs);

    if (strs.size() != 2) {
      app->errorMsg("Missing index for " + name);
      return ind;
    }

    ind.ix = Util::stringToInt(strs[0]);
    ind.iy = Util::stringToInt(strs[1]);

    return ind;
  };

  auto stringToPiece = [](const QString &str) {
    if      (str.toLower() == "white")
      return COTHELLO_PIECE_WHITE;
    else if (str.toLower() == "black")
      return COTHELLO_PIECE_BLACK;
    else
      return COTHELLO_PIECE_NONE;
  };

  if      (name == "init_board") {
  }
  else if (name == "board_piece") {
  }
  else if (name == "can_move_anywhere") {
  }
  else if (name == "can_move") {
    if (args.size() != 2) {
      app->errorMsg("Invalid args for " + name);
      return QVariant();
    }

    auto ind = argsToIndex();
    if (! ind.isValid()) return QVariant();

    auto b = board_->canMove(ind.ix, ind.iy, stringToPiece(args[1]));

    return QVariant(b);
  }
  else if (name == "do_move") {
    if (args.size() != 2) {
      app->errorMsg("Invalid args for " + name);
      return QVariant();
    }

    auto ind = argsToIndex();
    if (! ind.isValid()) return QVariant();

    board_->doMove(ind.ix, ind.iy, stringToPiece(args[1]));

    return QVariant();
  }
  else if (name == "is_white_piece") {
    auto ind = argsToIndex();
    if (! ind.isValid()) return QVariant();

    return QVariant(board_->getPiece(ind.ix, ind.iy) == COTHELLO_PIECE_WHITE ? 1 : 0);
  }
  else if (name == "is_black_piece") {
    auto ind = argsToIndex();
    if (! ind.isValid()) return QVariant();

    return QVariant(board_->getPiece(ind.ix, ind.iy) == COTHELLO_PIECE_BLACK ? 1 : 0);
  }
  else if (name == "num_white") {
  }
  else if (name == "num_black") {
  }
  else if (name == "num") {
  }
  else if (name == "best_move") {
    if (args.size() != 1) {
      app->errorMsg("Invalid args for " + name);
      return QVariant();
    }

    int depth = 1;

    int ix, iy;

    auto b = board_->getBestMove(stringToPiece(args[0]), depth, &ix, &iy);

    QString res;

    if (b)
      res = QString("%1 %2").arg(ix).arg(iy);
    else
      res = QString("-1 -1");

    return QVariant(res);
  }
  return Object3D::getValue(name, args);
}

bool
Othello3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  return Object3D::setValue(name, value, args);
}

void
Othello3DObj::
render()
{
}
#endif

}
