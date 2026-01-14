proc init { } {
  set particles [sb3d::particle_list]

  $particles set generator    lorenz
  $particles set particleSize 0.03
# $particles set texture      textures/particle.png
}
