proc init { } {
  set ::model [sb3d::model models/v3d/F15.V3D]

  set ::plane [sb3d::plane]

  $::plane set angles {-90 0 0}
  $::plane set scales {100 100 1}
  $::plane set position {0 -10 0}

  sb3d::canvas set shadowed 1
}

proc bboxChanged { } {
  resetProc
}

proc resetProc { } {
  sb3d::camera exec reset
  sb3d::light  exec reset 1
}

proc bboxChanged1 { } {
  sb3d::canvas exec save_image_buffer
}
