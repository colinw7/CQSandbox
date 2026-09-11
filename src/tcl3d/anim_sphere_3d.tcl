proc init { } {
  set ::shape [sb3d::shape]

  $::shape set sphere 0.4

  $::shape set position        [list 0 0 0]
  $::shape set position.target [list 1 1 1]
  $::shape set position.steps  1000
}
