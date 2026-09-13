proc init { } {
  set ::areal [sb3d::anim_real]

  set styles [list "one_shot" "bounce_once" "bounce_always"]
 #set styles [list "bounce_once"]

  foreach style $styles {
    echo "Style: $style"

    $::areal set value  0
    $::areal set target 1
    $::areal set steps  100

    $::areal set style $style

    for {set i 0} {$i < 300} {incr i} {
      $::areal exec step

      echo -nonewline "[$::areal get step] = [$::areal get value] "

      if {! [$::areal get can_step]} {
        echo "Done"
        break
      }
    }
  }

  echo "Done"

  exit
}
