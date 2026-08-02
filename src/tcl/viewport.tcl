proc init { } {
  set ::view1 [sb::viewport {0.0 0.0 0.5 1.0}]
  set ::view2 [sb::viewport {0.5 0.0 1.0 1.0}]

  sb::canvas set view $::view1

  sb::canvas set range {-2 -2 2 2}
  sb::canvas set equal_scale 1
  sb::canvas set brush.color "yellow"

  set ::circle1 [sb::circle {0 0} 128px]

  $::circle1 set brush.color red

  sb::canvas set view $::view2

  sb::canvas set range {-2 -2 2 2}
  sb::canvas set equal_scale 1
  sb::canvas set brush.color "green"

  set ::circle2 [sb::circle {0 0} 128px]

  $::circle2 set brush.color blue
}

proc update { } {
}
