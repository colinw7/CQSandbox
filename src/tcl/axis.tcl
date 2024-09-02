proc init { } {
  sb::canvas set range {0 0 100 100}

  sb::canvas set equal_scale 1

  # axis
  set ::xaxis [sb::axis {10 10} 80]
  set ::yaxis [sb::axis {10 10} 80]

  $::yaxis set direction vertical
}
