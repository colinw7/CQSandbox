proc init { } {
  sb::canvas set range {0 0 100 100}

  sb::canvas set equal_scale 1

  # arrow
  set ::arrow [sb::arrow {10 10} {90 90}]

  $::arrow set lineWidth 1

  $::arrow set front.visible   1
  $::arrow set front.angle     30
  $::arrow set front.backAngle 45
  $::arrow set front.length    4

  $::arrow set tail.visible   0
  $::arrow set tail.angle     30
  $::arrow set tail.backAngle 45
  $::arrow set tail.length    4
}
