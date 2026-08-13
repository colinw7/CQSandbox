proc init { } {
  set particles [sb3d::particle_list]

  $particles set size 8

  $particles set position {-1.0 0.0 -1.0} 0
  $particles set position { 1.0 0.0 -1.0} 1
  $particles set position { 1.0 1.0 -1.0} 2
  $particles set position {-1.0 1.0 -1.0} 3
  $particles set position {-1.0 0.0  1.0} 4
  $particles set position { 1.0 0.0  1.0} 5
  $particles set position { 1.0 1.0  1.0} 6
  $particles set position {-1.0 1.0  1.0} 7
}
