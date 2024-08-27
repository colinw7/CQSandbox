#ifndef CQSandboxUtil_H
#define CQSandboxUtil_H

#include <QString>

namespace CQSandbox {

namespace Util {

inline QString intToString(int i) {
  return QString::number(i);
}

inline QString realToString(double r) {
  return QString::number(r);
}

inline QString boolToString(bool b) {
  return QString(b ? "1" : "0");
}

//---

inline int stringToInt(const QString &s) {
  bool ok;
  return s.toInt(&ok);
}

inline double stringToReal(const QString &s) {
  bool ok;
  return s.toDouble(&ok);
}

inline bool stringToBool(const QString &s) {
  auto s1 = s.toLower();
  if (s1 == "false" || s1 == "no" || s1 == "0")
    return false;
  if (s1 == "true" || s1 == "yes" || s1 == "1")
    return true;
  return /*default*/true;
}

//---

}

}

#endif
