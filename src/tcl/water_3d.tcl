proc init { } {
  set ::plane [sb3d::model water/plane.ply]

  $::plane set normal_texture water/water_nrm.png

  $::plane set frag_shader shaders/water_model.fs
  $::plane set vert_shader shaders/water_model.vs
}
