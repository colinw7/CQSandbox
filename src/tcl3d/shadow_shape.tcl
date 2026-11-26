proc init { } {
  set ::shape [sb3d::shape]

  $::shape set cube {1 1 1}
  $::shape set color green

  set ::plane [sb3d::plane]

  $::plane set angles {-90 0 0}
  $::plane set scales {2 2 1}
  $::plane set position {0 -1 0}

  sb3d::canvas set shadowed 1
}

proc bboxChanged { } {
  resetProc
}

proc resetProc { } {
  sb3d::camera exec reset
  sb3d::light  exec reset 1
}
