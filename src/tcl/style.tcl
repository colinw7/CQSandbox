proc init { } {
  sb::canvas set range {-1 -1 1 1}

  sb::style set brush.color red
  sb::style set pen.color blue
  sb::style set pen.width 8

  set ::circle [sb::circle {0 0} 64px]
}
