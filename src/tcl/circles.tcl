proc init { } {
  sb::canvas set range {-2 -2 2 2}
  sb::canvas set equal_scale 1

  set ::pi  {3.141592653}
  set ::tau [expr {2*$::pi}]

  set ::n 0

  set ::anim_iter 16

  set_n 2

  set ::next_button [sb::button {10 30 px} "Next"]

  $::next_button set proc next_n
}

proc set_n { n } {
  set ::old_n $::n

  for {set i 0} {$i < $::old_n} {incr i} {
    set ::old_circle($i) $::circle($i)
  }

  for {set i 0} {$i < $::old_n} {incr i} {
    $::old_circle($i) delete
  }
  
  array unset ::old_circle

  array unset ::circle

  set ::n $n

  for {set i 0} {$i < $::n} {incr i} {
    set ir [expr {(1.0*$i)/$::n}]
    puts "ir: $ir"

    set a [expr {$ir*$::tau}]

    set x [expr {cos($a)}]
    set y [expr {sin($a)}]

    set ::circle_target($i) [list $x $y]
    set ::circle_iter($i) $::anim_iter

    set ::circle($i) [sb::circle {0 0} 16px]

    set c [sb::palette hsv [list $ir 0.5 1]]
    puts "c: $c"

    $::circle($i) set brush.color $c
  }
}

proc update { } {
  for {set i 0} {$i < $::n} {incr i} {
    if {$::circle_iter($i) >= 0} {
      set ii [expr {$::anim_iter - $::circle_iter($i)}]

      set pos $::circle_target($i)

      set xp [lindex $pos 0]
      set yp [lindex $pos 1]

      set x [expr {$ii*$xp/$::anim_iter}]
      set y [expr {$ii*$yp/$::anim_iter}]

      $::circle($i) set center [list $x $y]

      incr ::circle_iter($i) -1
    }
  }
}

proc next_n { } {
  set n [expr {$::n + 1}]

  set_n $n
}
