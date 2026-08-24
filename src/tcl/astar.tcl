proc init { } {
  set ::nx 12
  set ::ny 10

  set ::w 100
  set ::h 100

  set ::dx [expr {$::w/$::nx}]
  set ::dy [expr {$::h/$::ny}]

  set y1 0

  for {set iy 0} {$iy < $::ny} {incr iy} {
    set y2 [expr {$y1 + $::dy}]

    set x1 0

    for {set ix 0} {$ix < $::nx} {incr ix} {
      set x2 [expr {$x1 + $::dx}]

      set ::cell($ix,$iy) [sb::rect [list $x1 $y1 $x2 $y2]]

      set x1 $x2
    }

    set y1 $y2
  }

  sb::canvas set range [list 0 0 $::w $::h]

  set ::astar [sb3d::astar $::nx $::ny]
}

proc resetColors { } {
  for {set iy 0} {$iy < $::ny} {incr iy} {
    for {set ix 0} {$ix < $::nx} {incr ix} {
      $::cell($ix,$iy) set fill.color white
    }
  }
}

proc setCellColor { pos color } {
  set ix [lindex $pos 0]
  set iy [lindex $pos 1]

  $::cell($ix,$iy) set fill.color $color
}

proc keyPress { args } {
  set key [lindex $args 0]

  if {$key == "space"} {
    resetColors
     
    set from [list 0 0]
    set to   [list 11 9]

    set pos1 [list 3 0]
    set pos2 [list 6 6]

    $::astar set empty 0 $pos1
    $::astar set empty 0 $pos2

    setCellColor $pos1 black
    setCellColor $pos2 black

    while {1} {
      setCellColor $from red

      set from1 [$::astar get next $from $to]

      echo "$from -> $to : $from1"

      set from $from1

      if {[lindex $from 0] == [lindex $to 0] && [lindex $from 1] == [lindex $to 1]} {
        setCellColor $from red
        break
      }
    }
  }
}
