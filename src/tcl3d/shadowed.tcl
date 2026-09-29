proc init { } {
  set ::model [sb3d::model models/v3d/F15.V3D]

  sb3d::canvas set shadowed 1
}

proc bboxChanged { } {
  resetProc
}

proc resetProc { } {
  sb3d::camera exec reset
  sb3d::light  exec reset 1
}

proc bboxChanged { } {
  sb3d::canvas exec save_image_buffer
}
