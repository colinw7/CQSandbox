proc init { } {
  sb3d::canvas set lights.simple 1

  set ::plane [sb3d::plane]

  $::plane set scale    1.0
  $::plane set position [list 0 0 0]
  $::plane set color    white
  $::plane set texture  "textures/container.jpg"
}
