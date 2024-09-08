proc init { } {
  set ::shape [sb3d::shape]

  $::shape set cube 0.5
  $::shape set texture "textures/container.jpg"

# $::shape set wireframe 1
}
