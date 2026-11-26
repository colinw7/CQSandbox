#ifndef CSimplexNoise_H
#define CSimplexNoise_H

#include <CVector2D.h>

class CSimplexNoise2D {
 public:
  CSimplexNoise2D();

  double noise(const CVector2D &p) const;
};

#endif
