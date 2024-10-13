proc mapV { v v1 v2 } {
  return [expr {$v*($v2 - $v1) + $v1}]
}

proc init { } {
  sb3d::canvas set lights.simple 1

  set nc 10

  for {set i 0} {$i < $nc} {incr i} {
    set ::cube($i) [sb3d::cube]

    set x [mapV [expr {rand()}] -0.5 0.5]
    set y [mapV [expr {rand()}] -0.5 0.5]
    set z [mapV [expr {rand()}] -0.5 0.5]

    set xa [mapV [expr {rand()}] 0 360]
    set ya [mapV [expr {rand()}] 0 360]
    set za [mapV [expr {rand()}] 0 360]

    $::cube($i) set scale 0.1
    $::cube($i) set position [list $x $y $z]
    $::cube($i) set angle    [list $xa $ya $za]
    $::cube($i) set texture "textures/container.jpg"
  }
}
