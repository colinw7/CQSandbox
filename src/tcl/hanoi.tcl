proc newRect { rect c } {
  set r [sb::rect]

  $r set rect $rect

  $r set brush.color $c

  return $r
}

proc poleX { i } {
  if {$i == 0} { return -0.5 }
  if {$i == 1} { return  0.0 }
  if {$i == 2} { return  0.5 }
  return 0.0
}

proc init { } {
  set ::disc_count 7

  sb::canvas set range {-1 -1 1 1}

  set pw 0.04 ; # pole width
  set ph 1.00 ; # pole height

  set bw 0.40 ; # base width
  set bh 0.05 ; # base height

  set ::pole_y1 -0.5
  set ::pole_y2 [expr {$::pole_y1 + $ph}]

  set ::base_y2 $::pole_y1
  set ::base_y1 [expr {$::base_y2 - $bh}]

  for {set i 0} {$i < 3} {incr i} {
    set ::pole_x($i) [poleX $i]

    set ::pole_x1($i) [expr {$::pole_x($i) - $pw/2}]
    set ::pole_x2($i) [expr {$::pole_x($i) + $pw/2}]

    set ::pole_obj($i) [newRect [list $::pole_x1($i) $::pole_y1 $::pole_x2($i) $::pole_y2] "white"]

    set ::base_x1($i) [expr {$::pole_x($i) - $bw/2}]
    set ::base_x2($i) [expr {$::pole_x($i) + $bw/2}]

    set ::base_obj($i) [newRect [list $::base_x1($i) $::base_y1 $::base_x2($i) $::base_y2] "white"]
  }

  sb::canvas set brush.color black

  set ::disc_w(0) $bw

  set ::disc_dw [expr {($::disc_w(0) - 0.1)/($::disc_count - 1)}]

  for {set i 1} {$i < $::disc_count} {incr i} {
    set i1 [expr {$i - 1}]

    set ::disc_w($i) [expr {$::disc_w($i1) - $::disc_dw}]
  }

  set ::disc_gap 0.03

  set ::disc_h [expr {($ph - ($::disc_count + 1)*$::disc_gap)/$::disc_count}]

  set ::disc_color(0) "red"
  set ::disc_color(1) "green"
  set ::disc_color(2) "blue"
  set ::disc_color(3) "cyan"
  set ::disc_color(4) "magenta"
  set ::disc_color(5) "yellow"
  set ::disc_color(6) "orange"

  for {set i 0} {$i < $::disc_count} {incr i} {
    set ::disc_pole($i) 0
  }

  for {set i 0} {$i < $::disc_count} {incr i} {
    set ::disc_obj($i) [newRect [list 0 0 1 1] $::disc_color($i)]
  }

  updateDiscs
}

proc updateDiscs { } {
  for {set i 0} {$i < 3} {incr i} {
    set ::pole_top($i) -1
  }

  for {set i 0} {$i < 3} {incr i} {
    set ::disc_y1($i) [expr {$::pole_y1 + $::disc_gap}]
    set ::disc_y2($i) $::disc_y1($i)
  }

  for {set i 0} {$i < $::disc_count} {incr i} {
    set pole $::disc_pole($i)

    set ::pole_top($pole) $i

    set x [poleX $pole]

    set x1 [expr {$x - $::disc_w($i)/2}]
    set x2 [expr {$x + $::disc_w($i)/2}]

    set y1 $::disc_y1($pole)
    set y2 [expr {$y1 + $::disc_h}]

    $::disc_obj($i) set rect [list $x1 $y1 $x2 $y2]

    set ::disc_y1($pole) [expr {$y2 + $::disc_gap}]
  }

  if {$::pole_top(2) == [expr {$::disc_count - 1}]} {
    echo "SOLVED"
  }
}

proc posToPole { x y } {
  if       {$x >= $::base_x1(0) && $x <= $::base_x2(0)} {
    return 0
  } elseif {$x >= $::base_x1(1) && $x <= $::base_x2(1)} {
    return 1
  } elseif {$x >= $::base_x1(2) && $x <= $::base_x2(2)} {
    return 2
  }
  return -1
}

proc mousePress { x y } {
  # echo "$x $y"

  set pole [posToPole $x $y]

  # echo "Pole $pole"

  if {$pole >= 0 && $::pole_top($pole) >= 0} {
    set ::move_pole $pole
  } else {
    set ::move_pole -1
  }
}

proc mouseMove { x y } {
  # echo "$x $y"

  if {$::move_pole >= 0} {
    set from_disc $::pole_top($::move_pole)

    if {$from_disc >= 0} {
      set x1 [expr {$x - $::disc_w($from_disc)/2}]
      set x2 [expr {$x + $::disc_w($from_disc)/2}]

      set y1 [expr {$y - $::disc_h/2}]
      set y2 [expr {$y + $::disc_h/2}]

      $::disc_obj($from_disc) set rect [list $x1 $y1 $x2 $y2]
    }
  }
}

proc mouseRelease { x y } {
  # echo "$x $y"

  set pole [posToPole $x $y]

  if {$::move_pole >= 0} {
    if {$pole >= 0 && $pole != $::move_pole} {
      set from_disc $::pole_top($::move_pole)
      set to_disc   $::pole_top($pole)

      echo "Move Pole $::move_pole -> $pole : $from_disc $to_disc"

      if {$from_disc >= 0 && $from_disc > $to_disc} {
        set ::disc_pole($from_disc) $pole
      }
    }

    updateDiscs
  }
}

proc update { } {
}
