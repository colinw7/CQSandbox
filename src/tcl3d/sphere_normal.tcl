proc init { } {
  set ::shape [sb3d::shape]

  $::shape set num_patches 64
  $::shape set num_xy      64

  $::shape set sphere 0.4

  $::shape set color white

  set ::shader [sb3d::shader]

  $::shader set vertex_shader   custom_shaders/sphere_normal.vs
  $::shader set fragment_shader custom_shaders/sphere_normal.fs

  $::shape set shader $::shader
}
