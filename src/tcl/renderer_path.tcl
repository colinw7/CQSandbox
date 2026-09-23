proc init { } {
  set width  [sb::canvas get pixel_width]
  set height [sb::canvas get pixel_height]

  set ::x {}
  lappend ::x 3
  lappend ::x [expr {$width - 3}]
  lappend ::x [expr {$width - 3}]
  lappend ::x 3

  set ::y {}
  lappend ::y 3
  lappend ::y 3
  lappend ::y [expr {$height - 3}]
  lappend ::y [expr {$height - 3}]

  set ::renderer [sb::renderer]
}

proc drawBg { args } {
  $::renderer set pen.color   red
  $::renderer set brush.color green

  $::renderer exec path.start

  set x1 [lindex $::x 0]
  set y1 [lindex $::y 0]
  set x2 [lindex $::x 1]
  set y2 [lindex $::y 1]

  $::renderer exec path.moveTo [list [expr {($x1 + $x2)/2}] [expr {($y1 + $y2)/2}]]
  
  set n [llength $::x]
  
  for {set i 0} {$i < $n} {incr i} {
    set i1 [expr {$i + 1}]
    if {$i1 >= $n} { set i1 0}

    set i2 [expr {$i1 + 1}]
    if {$i2 >= $n} { set i2 0}

    set cx1 [lindex $::x $i1]
    set cy1 [lindex $::y $i1]
  
    set cx2 [lindex $::x $i2]
    set cy2 [lindex $::y $i2]

    set cxm [expr {($cx1 + $cx2)/2}]
    set cym [expr {($cy1 + $cy2)/2}]

    $::renderer exec path.curveTo [list $cx1 $cy1] [list $cx1 $cy1] [list $cxm $cym]
  }

  $::renderer exec path.draw
}
