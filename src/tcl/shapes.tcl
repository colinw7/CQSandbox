proc init { } {
  sb::canvas set range {-2 -2 2 2}
  sb::canvas set equal_scale 1

  set ::circle [sb::circle {-0.5 -0.5} 1]
  set ::rect   [sb::rect   { 0.5 0.5 1.5 1.5}]

  $::circle set brush.color blue
  $::rect set brush.color green
}
