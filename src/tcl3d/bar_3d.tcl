proc mapV { v v1 v2 } {
  return [expr {$v*($v2 - $v1) + $v1}]
}

proc init { } {
  sb3d::canvas set bg #111122

# sb3d::camera set zoom 90
  sb3d::camera set position [list 0 0 2]

  set nx 10
  set ny 10

  sb3d::canvas set xrange [list 0 $nx]
  sb3d::canvas set yrange [list 0 10]
  sb3d::canvas set zrange [list 0 $ny]

  set i 0

  set dx [expr {0.8/$nx}]
  set dy [expr {0.8/$ny}]

  for {set iy 0} {$iy < $ny} {incr iy} {
    set y [sb3d::canvas get zmap $iy]

    for {set ix 0} {$ix < $nx} {incr ix} {
      set x [sb3d::canvas get xmap $ix]

      set ::bar($i) [sb3d::shape]

      set z [expr {rand()*8 + 2}]

      set z1 [sb3d::canvas get ymap 0.0]
      set z2 [sb3d::canvas get ymap $z]

      set dz [expr {$z2 - $z1}]
      set z3 [expr {($z1 + $z2)/2.0}]

      #puts "$x $y $z1 $z2"

      set b [expr {$x + 0.5}]

      $::bar($i) set cube     [list $dx $dz $dy]
      $::bar($i) set position [list $x $z3 $y]
      $::bar($i) set color    [list 0.4 0.4 $b 0.95]

      incr i
    }
  }

  set base [sb3d::shape]

  $base set cube     [list 1.2 0.001 1.2]
  $base set position [list 0.0 -0.501 0.0]
  $base set color    [list 0.4 0.8 0.4 0.5]

  #---

  set ::xaxis [sb3d::axis]
  set ::yaxis [sb3d::axis]
  set ::zaxis [sb3d::axis]

  $::xaxis set auto_range x
  $::yaxis set auto_range y
  $::zaxis set auto_range z
}
