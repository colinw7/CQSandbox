proc is_odd { i } {
  return [expr {($i % 2) != 0}]
}

proc init { } {
  set y 0.0

  for {set iy 0} {$iy < 8} {incr iy} {
    set x 0.0

    for {set ix 0} {$ix < 8} {incr ix} {
      set shape [sb3d::shape]

      set ::shape($ix,$iy) $shape

      $shape set cube [list 0.2 0.2 0.01]
      $shape set position [list $x $y 0]

      if {[is_odd $ix] == [is_odd $iy]} {
        $shape set texture "textures/blue_marble.png"
      } else {
        $shape set texture "textures/white_marble.png"
      }

      set x [expr {$x + 0.1}]
    }

    set y [expr {$y + 0.1}]
  }
}
