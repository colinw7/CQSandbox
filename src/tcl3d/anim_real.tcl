proc init { } {
  set ::areal [sb3d::anim_real]

  $::areal set value  0
  $::areal set target 1
  $::areal set steps  100

  for {set i 0} {$i < 100} {incr i} {
    $::areal exec step

    echo "Value: [$::areal get value]"
  }
}
