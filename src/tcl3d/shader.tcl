proc init { } {
  set ::shader [sb3d::shader]

  $::shader set vertex_shader   custom_shaders/water.vs
  $::shader set fragment_shader custom_shaders/water.fs
}
