proc init { } {
  sb3d::canvas set cull_face 0

  set ::shape [sb3d::shape]

  $::shape set points     {{0.5 0.5 0.0} {0.5 -0.5 0.0} {-0.5 -0.5 0.0} {-0.5 0.5 0.0}}
  $::shape set colors     {{1.0 0.0 0.0} {0.0 1.0 0.0} {0.0 0.0 1.0} {1.0 1.0 1.0}}
  $::shape set tex_coords {{1 1} {1 0} {0 0} {0 1}}
  $::shape set indices    {0 1 3 1 2 3}
# $::shape set texture    "textures/container.jpg"
  $::shape set texture    "textures/Catwoman.jpg"

# $::shape set points {{0.5 0.5 0.0} {0.5 -0.5 0.0} {-0.5 -0.5 0.0}}
# $::shape set colors {{1.0 0.0 0.0} {0.0 1.0 0.0} {0.0 0.0 1.0}}

# $::shape set wireframe 1
}
