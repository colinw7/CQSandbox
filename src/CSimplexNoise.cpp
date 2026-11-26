#include <CSimplexNoise.h>

#include <vec4.h>
#include <vec3.h>
#include <vec2.h>

namespace {

// Some useful functions
vec3 mod289(const vec3 &x) { return x - (x * (1.0 / 289.0)).floor() * 289.0; }
vec2 mod289(const vec2 &x) { return x - (x * (1.0 / 289.0)).floor() * 289.0; }
vec3 permute(const vec3 &x) { return mod289(((x*34.0)+1.0)*x); }

//
// Description : GLSL 2D simplex noise function
//      Author : Ian McEwan, Ashima Arts
//  Maintainer : ijm
//     Lastmod : 20110822 (ijm)
//     License :
//  Copyright (C) 2011 Ashima Arts. All rights reserved.
//  Distributed under the MIT License. See LICENSE file.
//  https://github.com/ashima/webgl-noise
//
double snoise(const vec2 &v) {
  // Precompute values for skewed triangular grid
  const vec4 C = vec4( 0.211324865405187, // (3.0-sqrt(3.0))/6.0
                       0.366025403784439, // 0.5*(sqrt(3.0)-1.0)
                      -0.577350269189626, // -1.0 + 2.0 * C.x
                       0.024390243902439); // 1.0 / 41.0

  // First corner (x0)
  vec2 i  = (v + v.dot(C.yy())).floor();
  vec2 x0 = v - i + i.dot(C.xx());

  // Other two corners (x1, x2)
  vec2 i1 = vec2(0.0);
  i1 = (x0.x > x0.y)? vec2(1.0, 0.0):vec2(0.0, 1.0);
  vec2 x1 = x0.xy() + C.xx() - i1;
  vec2 x2 = x0.xy() + C.zz();

  // Do some permutations to avoid
  // truncation effects in permutation
  i = mod289(i);
  vec3 p = permute(permute(i.y + vec3(0.0, i1.y, 1.0)) +
                           i.x + vec3(0.0, i1.x, 1.0));

  vec3 m = (0.5 - vec3(x0.dot(x0), x1.dot(x1), x2.dot(x2))).max(0.0);

  m = m*m;
  m = m*m;

  // Gradients:
  //  41 pts uniformly over a line, mapped onto a diamond
  //  The ring size 17*17 = 289 is close to a multiple
  //      of 41 (41*7 = 287)

  vec3 x = 2.0 * (p*C.www()).fract() - 1.0;
  vec3 h = x.abs() - 0.5;
  vec3 ox = (x + 0.5).floor();
  vec3 a0 = x - ox;

  // Normalise gradients implicitly by scaling m
  // Approximation of: m *= inversesqrt(a0*a0 + h*h);
  m *= 1.79284291400159 - 0.85373472095314*(a0*a0 + h*h);

  // Compute final noise value at P
  vec3 g = vec3(0.0);
  g.x = a0.x*x0.x + h.x*x0.y;
  g.setYZ(a0.yz()*vec2(x1.x, x2.x) + h.yz()*vec2(x1.y, x2.y));
  return 130.0*m.dot(g);
}

}

//---

CSimplexNoise2D::
CSimplexNoise2D()
{
}

double
CSimplexNoise2D::
noise(const CVector2D &p) const
{
  return snoise(vec2(p.getX(), p.getY()));
}
