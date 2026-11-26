proc init { } {
  set ::plane [sb3d::plane]

  $::plane set color blue

  set water_texture [sb3d::texture "textures/water_texture.png"]

  $::plane set texture $water_texture

  set ::shader [sb3d::shader]

  $::shader set vertex_shader   custom_shaders/plane_water_shader.vs
  $::shader set fragment_shader custom_shaders/plane_water_shader.fs

  $::plane set shader $::shader
}
