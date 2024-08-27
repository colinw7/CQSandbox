#include <CQSVGUtil.h>
#include <CSVGUtil.h>

#include <QXmlStreamReader>
#include <QPainterPath>
#include <QFile>
#include <QString>

namespace {

struct PathData {
  QString d;
  QString style;
};

using PathDatas = std::vector<PathData>;

}

class CQSVGParserHandler {
 public:
  CQSVGParserHandler(QRectF &bbox, PathDatas &pathDatas) :
    bbox_(bbox), pathDatas_(pathDatas) {
  }

  bool parse(QXmlStreamReader &xml) {
    while (! xml.atEnd()) {
      xml.readNext();

      switch (xml.tokenType()) {
        case QXmlStreamReader::StartDocument: {
          break;
        }
        case QXmlStreamReader::EndDocument: {
          break;
        }
        case QXmlStreamReader::StartElement: {
          if (! startElement(xml.qualifiedName().toString(), xml.attributes()))
            return false;

          break;
        }
        case QXmlStreamReader::EndElement: {
         if (! endElement(xml.qualifiedName().toString()))
            return false;

          break;
        }
        case QXmlStreamReader::Comment: {
          break;
        }
        case QXmlStreamReader::Characters: {
          break;
        }
        default:
          break;
      }
    }

    return xml.hasError();
  }

 private:
  bool startElement(const QString &qName, const QXmlStreamAttributes &attributes) {
    // save bounding box
    if      (qName == "svg") {
      for (int i = 0; i < attributes.length(); ++i) {
        auto &attr = attributes[i];

        auto name  = attr.qualifiedName();
        auto value = attr.value().toString();

        if (name == "viewBox") {
          auto strs = QString(value).split(" ", Qt::SkipEmptyParts);

          if (strs.size() == 4) {
            bool ok { false }, ok1 { true };
            int x = strs[0].toInt(&ok); if (! ok) ok1 = false;
            int y = strs[1].toInt(&ok); if (! ok) ok1 = false;
            int w = strs[2].toInt(&ok); if (! ok) ok1 = false;
            int h = strs[3].toInt(&ok); if (! ok) ok1 = false;

            if (ok1)
              bbox_ = QRectF(x, y, w, h);
          }
        }
      }
    }
    // save path
    else if (qName == "path") {
      PathData pathData;

      for (int i = 0; i < attributes.length(); ++i) {
        auto &attr = attributes[i];

        auto name  = attr.qualifiedName();
        auto value = attr.value().toString();

        if      (name == "d")
          pathData.d = value;
        else if (name == "style")
          pathData.style = value;
      }

      if (pathData.d != "")
        pathDatas_.push_back(std::move(pathData));
    }

    return true;
  }

  bool endElement(const QString &) {
    return true;
  }

 private:
  QRectF    &bbox_;
  PathDatas &pathDatas_;
};

//---

class QPainterPathVisitor {
 public:
  QPainterPathVisitor() = default;

  virtual ~QPainterPathVisitor() = default;

  virtual void init() { }
  virtual void term() { }

  virtual void moveTo (const QPointF &p) = 0;
  virtual void lineTo (const QPointF &p) = 0;
  virtual void quadTo (const QPointF &p1, const QPointF &p2) = 0;
  virtual void curveTo(const QPointF &p1, const QPointF &p2, const QPointF &p3) = 0;

 public:
  const QPainterPath *path { nullptr };

  int     i { -1 };
  int     n { 0 };
  QPointF lastP;
  QPointF nextP;
};

void visitPath(const QPainterPath &path, QPainterPathVisitor &visitor) {
  visitor.path = &path;

  visitor.n = path.elementCount();

  visitor.init();

  for (visitor.i = 0; visitor.i < visitor.n; ++visitor.i) {
    const auto &e = path.elementAt(visitor.i);

    if      (e.isMoveTo()) {
      QPointF p(e.x, e.y);

      if (visitor.i < visitor.n - 1) {
        auto e1 = path.elementAt(visitor.i + 1);

        visitor.nextP = QPointF(e1.x, e1.y);
      }
      else
        visitor.nextP = p;

      visitor.moveTo(p);

      visitor.lastP = p;
    }
    else if (e.isLineTo()) {
      QPointF p(e.x, e.y);

      if (visitor.i < visitor.n - 1) {
        auto e1 = path.elementAt(visitor.i + 1);

        visitor.nextP = QPointF(e1.x, e1.y);
      }
      else
        visitor.nextP = p;

      visitor.lineTo(p);

      visitor.lastP = p;
    }
    else if (e.isCurveTo()) {
      QPointF p(e.x, e.y);

      QPointF p1, p2;

      QPainterPath::ElementType e1t { QPainterPath::MoveToElement };
      QPainterPath::ElementType e2t { QPainterPath::MoveToElement };

      if (visitor.i < visitor.n - 1) {
        auto e1 = path.elementAt(visitor.i + 1);

        e1t = e1.type;

        p1 = QPointF(e1.x, e1.y);
      }

      if (visitor.i < visitor.n - 2) {
        auto e2 = path.elementAt(visitor.i + 2);

        e2t = e2.type;

        p2 = QPointF(e2.x, e2.y);
      }

      if (e1t == QPainterPath::CurveToDataElement) {
        ++visitor.i;

        if (e2t == QPainterPath::CurveToDataElement) {
          ++visitor.i;

          if (visitor.i < visitor.n - 1) {
            auto e3 = path.elementAt(visitor.i + 1);

            visitor.nextP = QPointF(e3.x, e3.y);
          }
          else
            visitor.nextP = p;

          visitor.curveTo(p, p1, p2);

          visitor.lastP = p;
        }
        else {
          if (visitor.i < visitor.n - 1) {
            auto e3 = path.elementAt(visitor.i + 1);

            visitor.nextP = QPointF(e3.x, e3.y);
          }
          else
            visitor.nextP = p;

          visitor.quadTo(p, p1);

          visitor.lastP = p;
        }
      }
    }
    else
      assert(false);
  }

  visitor.term();
}

//---

bool
CQSVGUtil::
stringToPath(const QString &str, QPainterPath &path)
{
  class PathVisitor : public CSVGUtil::PathVisitor {
   public:
    PathVisitor() { }

    const QPainterPath &path() const { return path_; }

    void moveTo(double x, double y) override {
      path_.moveTo(x, y);
    }

    void lineTo(double x, double y) override {
      path_.lineTo(x, y);
    }

    void arcTo(double rx, double ry, double xa, int fa, int fs, double x2, double y2) override {
      auto qpoint = [](const CPoint2D &p) { return QPointF(p.x, p.y); };

      bool unit_circle = false;

      //double cx, cy, rx1, ry1, theta, delta;

      //CSVGUtil::convertArcCoords(lastX(), lastY(), x2, y2, xa, rx, ry, fa, fs, unit_circle,
      //                           &cx, &cy, &rx1, &ry1, &theta, &delta);

      //path_.arcTo(QRectF(cx - rx1, cy - ry1, 2*rx1, 2*ry1), -theta, -delta);

      //double a1 = CMathUtil::Deg2Rad(theta);
      //double a2 = CMathUtil::Deg2Rad(theta + delta);

      CSVGUtil::BezierList beziers;

      CSVGUtil::arcToBeziers(lastX(), lastY(), x2, y2, xa, rx, ry, fa, fs, unit_circle, beziers);

      if (! beziers.empty())
        path_.lineTo(qpoint(beziers[0].getFirstPoint()));

      for (const auto &bezier : beziers)
        path_.cubicTo(qpoint(bezier.getControlPoint1()),
                      qpoint(bezier.getControlPoint2()),
                      qpoint(bezier.getLastPoint    ()));
    }

    void bezier2To(double x1, double y1, double x2, double y2) override {
      path_.quadTo(QPointF(x1, y1), QPointF(x2, y2));
    }

    void bezier3To(double x1, double y1, double x2, double y2, double x3, double y3) override {
      path_.cubicTo(QPointF(x1, y1), QPointF(x2, y2), QPointF(x3, y3));
    }

    void closePath(bool /*relative*/) override {
      path_.closeSubpath();
    }

   private:
    QPainterPath path_;
  };

  PathVisitor visitor;

  if (! CSVGUtil::visitPath(str.toStdString(), visitor)) {
    //std::cerr << "Invalid path: " << str.toStdString() << "\n";
    return false;
  }

  path = visitor.path();

  return true;
}

QString
CQSVGUtil::
pathToString(const QPainterPath &path)
{
  class PathVisitor : public QPainterPathVisitor {
   public:
    void moveTo(const QPointF &p) override {
      if (str_.length()) str_ += " ";

      str_ += QString("M %1 %2").arg(p.x()).arg(p.y());
    }

    void lineTo(const QPointF &p) override {
      if (str_.length()) str_ += " ";

      str_ += QString("L %1 %2").arg(p.x()).arg(p.y());
    }

    void quadTo(const QPointF &p1, const QPointF &p2) override {
      if (str_.length()) str_ += " ";

      str_ += QString("Q %1 %2 %3 %4").arg(p1.x()).arg(p1.y()).arg(p2.x()).arg(p2.y());
    }

    void curveTo(const QPointF &p1, const QPointF &p2, const QPointF &p3) override {
      if (str_.length()) str_ += " ";

      str_ += QString("C %1 %2 %3 %4 %5 %6").
                arg(p1.x()).arg(p1.y()).arg(p2.x()).arg(p2.y()).arg(p3.x()).arg(p3.y());
    }

    const QString &str() const { return str_; }

   private:
    QString str_;
  };

  PathVisitor visitor;

  visitPath(path, visitor);

  return visitor.str();
}
