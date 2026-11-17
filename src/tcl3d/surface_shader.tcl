proc init { } {
  set ::cube [sb3d::shape]
  $::cube set cube {1 1 1}
  $::cube set color red
  $::cube set position {0 1 0}

  set ::surface [sb3d::surface]

  $::surface set size {32 32}

  $::surface set color white

  $::surface set position {-0.5 -0.5 0}

  set ::shader [sb3d::shader]

  $::shader set vertex_shader   custom_shaders/surface_shader.vs
  $::shader set fragment_shader custom_shaders/surface_shader.fs

  $::surface set shader $::shader

  $::surface set cull_face 0
}
