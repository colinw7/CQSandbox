proc init { } {
  set particles [sb3d::particle_list]

  $particles set flocking 1
  $particles set texture  textures/particle.png
}
