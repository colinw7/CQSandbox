proc init { } {
  sb::canvas set range {0 0 100 100}
  sb::canvas set equal_scale 1

  set points [sb::point_list 5px]

  $points set size 4

  $points set connected   1
  $points set brush.color green

  $points set position { 5  5} 0
  $points set position {10 15} 1
  $points set position {15  6} 2
  $points set position {20 11} 3
}
