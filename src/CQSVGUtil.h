#ifndef CQSVGUtil_H
#define CQSVGUtil_H

class QString;
class QPainterPath;

namespace CQSVGUtil {
  bool stringToPath(const QString &str, QPainterPath &path);
  QString pathToString(const QPainterPath &path);
}

#endif
