proc init { } {
  set ::surface [sb3d::surface]

  set n 40

  $::surface set size [list $n $n]

# $::surface set wireframe 1

  for {set iy 0} {$iy < $n} {incr iy} {
    set y [expr {4.0*$iy/($n - 1.0) - 2.0}]

    for {set ix 0} {$ix < $n} {incr ix} {
      set x [expr {4.0*$ix/($n - 1.0) - 2.0}]

      set z [expr {exp(-($x**2+$y**2))*cos($x/4.0)*sin($y)*cos(2*($x*$x+$y*$y))}]

      set inds [list $ix $iy]

      $::surface set point $z $inds

      if {$z < 0.0} {
        set z1 [expr {-1.0*$z}]

        $::surface set color [list 0.0 0.0 $z1] $inds
      } else {
        set z1 [expr {1.0*$z}]

        $::surface set color [list 0.0 $z1 0.0] $inds
      }
    }
  }
}
