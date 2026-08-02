proc incrI { i } {
  upvar $i i1
  set i2 $i1
  incr i1
  return $i2
}

proc randIn { min max } {
  return [expr {rand()*($max - $min) + $min}]
}

proc hypot { dx dy } {
  return [expr {sqrt($dx*$dx + $dy*$dy)}]
}

proc addTextLabel { str pos align } {
  set text [sb::text $pos $str]

  $text set pen.color white
  $text set align     $align

  return $text
}

proc addObjectPoints { obj pts } {
  # echo "addObjectPoints $obj $pts"

  $obj set connected  1
  $obj set stroked    1
  $obj set filled     0

  $obj set size [llength $pts]

  set i 0

  foreach p $pts {
    $obj set position $p [incrI i]
  }

  $obj set brush.color white
  $obj set pen.color   white
}

proc addShip { } {
  set ::ship_angle 0

  set ::ship_obj [sb::point_list 5px]

  addObjectPoints $::ship_obj \
    [list { 0.5  0.0} {-0.5  0.5} {-0.3  0.0} {-0.5 -0.5}]

  $::ship_obj set scale 0.03

  addShipThrust

  $::ship_obj set visible 0

  initShip
}

proc initShip { } {
  set ::ship_x 0
  set ::ship_y 0

  set ::ship_vx 0.0
  set ::ship_vy 0.0

  set ::ship_thrust 0.003
}

proc addShipThrust { } {
  set ::ship_thrust_obj [sb::point_list 5px]

  addObjectPoints $::ship_thrust_obj \
    [list {  0.500  0.000} { -0.500  0.500} { -0.375  0.125} \
          { -0.675  0.025} { -0.375 -0.150} { -0.500 -0.500}]

  $::ship_thrust_obj set scale 0.03

  $::ship_thrust_obj set visible 0
}

proc addRock1 { } {
  set rock_ind1 $::rock_ind

  set ::rock_x($::rock_ind) [randIn -1 1]
  set ::rock_y($::rock_ind) [randIn -1 1]

  set ::rock_angle($::rock_ind) 0
  set ::rock_da($::rock_ind) [randIn -1 1]

  set ::rock_vx($::rock_ind) [randIn -0.01 0.01]
  set ::rock_vy($::rock_ind) [randIn -0.01 0.01]

  set ::rock_obj($::rock_ind) [sb::point_list 5px]

  addObjectPoints $::rock_obj($::rock_ind) \
    [list {-0.100 -0.475} { 0.325 -0.500} { 0.500 -0.250} { 0.450  0.225} \
          { 0.250  0.500} {-0.250  0.425} {-0.300  0.225} {-0.475  0.200} \
          {-0.475 -0.075} {-0.250 -0.150} {-0.325 -0.325}]

  $::rock_obj($::rock_ind) set offset [list $::rock_x($::rock_ind) $::rock_y($::rock_ind)]

  $::rock_obj($::rock_ind) set scale 0.1

  $::rock_obj($::rock_ind) set meta 1

  incr ::rock_ind

  incr ::num_rocks

  return $rock_ind1
}

proc newRock1 { p a v da } {
  set rock_ind [addRock1]

  set ::rock_angle($rock_ind) $a

  set ::rock_x($rock_ind) [lindex $p 0]
  set ::rock_y($rock_ind) [lindex $p 1]

  set ::rock_vx($rock_ind) [lindex $v 0]
  set ::rock_vy($rock_ind) [lindex $v 1]

  set ::rock_da($rock_ind) $da

  return $rock_ind
}

proc addRock2 { } {
  set rock_ind1 $::rock_ind

  set ::rock_angle($::rock_ind) 0
  set ::rock_da($::rock_ind) [randIn -1 1]

  set ::rock_x($::rock_ind) [randIn -1 1]
  set ::rock_y($::rock_ind) [randIn -1 1]

  set ::rock_vx($::rock_ind) [randIn -0.01 0.01]
  set ::rock_vy($::rock_ind) [randIn -0.01 0.01]

  set ::rock_obj($::rock_ind) [sb::point_list 5px]

  addObjectPoints $::rock_obj($::rock_ind) \
    [list {-0.125 -0.500} { 0.325 -0.450} { 0.500 -0.250} { 0.425  0.175} \
          { 0.225  0.450} {-0.250  0.500} {-0.300  0.250} {-0.450  0.200} \
          {-0.500 -0.125} {-0.300 -0.175} {-0.350 -0.300}]

  $::rock_obj($::rock_ind) set offset [list $::rock_x($::rock_ind) $::rock_y($::rock_ind)]

  $::rock_obj($::rock_ind) set scale 0.05

  $::rock_obj($::rock_ind) set meta 2

  incr ::rock_ind

  incr ::num_rocks

  return $rock_ind1
}

proc addRock3 { } {
  set rock_ind1 $::rock_ind

  set ::rock_angle($::rock_ind) 0
  set ::rock_da($::rock_ind) [randIn -1 1]

  set ::rock_x($::rock_ind) [randIn -1 1]
  set ::rock_y($::rock_ind) [randIn -1 1]

  set ::rock_vx($::rock_ind) [randIn -0.01 0.01]
  set ::rock_vy($::rock_ind) [randIn -0.01 0.01]

  set ::rock_obj($::rock_ind) [sb::point_list 5px]

  addObjectPoints $::rock_obj($::rock_ind) \
    [list {-0.25 -0.500} { 0.10 -0.450} { 0.25 -0.275} { 0.45 -0.275} \
          { 0.50  0.050} { 0.30  0.150} { 0.20  0.450} {-0.15  0.500} \
          {-0.25  0.175} {-0.45  0.100} {-0.50 -0.250} {-0.30 -0.300}]

  $::rock_obj($::rock_ind) set offset [list $::rock_x($::rock_ind) $::rock_y($::rock_ind)]

  $::rock_obj($::rock_ind) set scale 0.025

  $::rock_obj($::rock_ind) set meta 3

  incr ::rock_ind

  incr ::num_rocks

  return $rock_ind1
}

proc hitRock { i } {
  set meta [$::rock_obj($i) get meta]

  if       {$meta == 1} {
    hitRock1 $i
  } elseif {$meta == 2} {
    hitRock2 $i
  } else {
    hitRock3 $i
  }
}

proc hitRock1 { i } {
  # echo "Hit Rock 1"

  set breakSpeed [randIn 1.1 1.3]

  set vx [expr {$::rock_vx($i)*$breakSpeed}]
  set vy [expr {$::rock_vy($i)*$breakSpeed}]

  $::rock_obj($i) set visible 0

  incr ::num_rocks -1

  set rock_ind1 [addRock2]
  set rock_ind2 [addRock2]

  set ::rock_x($rock_ind1) $::rock_x($i)
  set ::rock_y($rock_ind1) $::rock_y($i)

  set ::rock_x($rock_ind2) $::rock_x($i)
  set ::rock_y($rock_ind2) $::rock_y($i)

  set ::rock_angle($rock_ind1) [expr {$::rock_angle($i) + 0.25}]
  set ::rock_angle($rock_ind2) [expr {$::rock_angle($i) - 0.25}]

  set ::rock_vx($rock_ind1) [expr {-$vx}]
  set ::rock_vy($rock_ind1) [expr { $vy}]

  set ::rock_vx($rock_ind2) [expr { $vx}]
  set ::rock_vy($rock_ind2) [expr {-$vy}]

  set ::rock_da($rock_ind1) $::rock_da($i)
  set ::rock_da($rock_ind2) $::rock_da($i)

  set ::score [expr {$::score + 20}]
  $::score_text set text "Score: $::score"
}

proc hitRock2 { i } {
  # echo "Hit Rock 2"

  set breakSpeed [randIn 1.1 1.3]

  set vx [expr {$::rock_vx($i)*$breakSpeed}]
  set vy [expr {$::rock_vy($i)*$breakSpeed}]

  $::rock_obj($i) set visible 0

  incr ::num_rocks -1

  set rock_ind1 [addRock3]
  set rock_ind2 [addRock3]

  set ::rock_x($rock_ind1) $::rock_x($i)
  set ::rock_y($rock_ind1) $::rock_y($i)

  set ::rock_x($rock_ind2) $::rock_x($i)
  set ::rock_y($rock_ind2) $::rock_y($i)

  set ::rock_angle($rock_ind1) [expr {$::rock_angle($i) + 0.25}]
  set ::rock_angle($rock_ind2) [expr {$::rock_angle($i) - 0.25}]

  set ::rock_vx($rock_ind1) [expr {-$vx}]
  set ::rock_vy($rock_ind1) [expr { $vy}]

  set ::rock_vx($rock_ind2) [expr { $vx}]
  set ::rock_vy($rock_ind2) [expr {-$vy}]

  set ::rock_da($rock_ind1) $::rock_da($i)
  set ::rock_da($rock_ind2) $::rock_da($i)

  set ::score [expr {$::score + 50}]
  $::score_text set text "Score: $::score"
}

proc hitRock3 { i } {
  # echo "Hit Rock 3"

  $::rock_obj($i) set visible 0

  incr ::num_rocks -1

  set ::score [expr {$::score + 100}]
  $::score_text set text "Score: $::score"
}

proc addSmallSaucer { } {
  set ind   -1
  set count 0

  for {set i 0} {$i < $::saucer_ind} {incr i} {
    if {[$::saucer_obj($i) get visible]} {
      incr count
    } else {
      set ind $i
    }
  }

  # echo "addSmallSaucer $count $ind"

  if {$count > 0} {
    return
  }

  if {$ind < 0} {
    set ind $::saucer_ind

    set ::saucer_obj($ind) [sb::point_list 5px]

    addObjectPoints $::saucer_obj($::saucer_ind) \
      [list { 0.50  0.000} { 0.45  0.125} { 0.25  0.250} {-0.25  0.250} \
            {-0.45  0.125} {-0.50  0.000} {-0.25 -0.150} {-0.25 -0.250} \
            { 0.25 -0.250} { 0.25 -0.150}]

    $::saucer_obj($::saucer_ind) set scale 0.03

    incr ::saucer_ind
  }

  set ::saucer_angle($ind) 0
  set ::saucer_da($ind) 0

  set y [randIn 0 0.5]

  if {$y < 0.25} {
    set y [expr {0.625 + $y}]
  } else {
    set y [expr {0.375 - ($y - 0.25)}]
  }

  set ::saucer_x($ind) -1.0
  set ::saucer_y($ind) $y

  set ::saucer_vx($ind) 0.01
  set ::saucer_vy($ind) 0.0

  set ::saucer_ifire($ind) 20
  set ::saucer_fire($ind) $::saucer_ifire($ind)

  $::saucer_obj($ind) set offset [list $::saucer_x($ind) $::saucer_y($ind)]

  $::saucer_obj($ind) set visible 1

  $::saucer_obj($ind) set meta small

  return $ind
}

proc addBigSaucer { } {
  set ind   -1
  set count 0
      
  for {set i 0} {$i < $::saucer_ind} {incr i} {
    if {[$::saucer_obj($i) get visible]} {
      incr count
    } else {
      set ind $i
    }
  }
            
  # echo "addBigSaucer $count $ind"

  if {$count > 0} {
    return
  }   
    
  if {$ind < 0} {
    set ind $::saucer_ind

    set ::saucer_obj($::saucer_ind) [sb::point_list 5px]

    addObjectPoints $::saucer_obj($::saucer_ind) \
      [list { 0.500  0.000} { 0.325  0.100} { 0.200  0.100} { 0.100  0.250} \
            {-0.100  0.250} {-0.175  0.100} {-0.325  0.100} {-0.500  0.000} \
            {-0.500 -0.075} {-0.250 -0.250} { 0.250 -0.250} { 0.500 -0.075}]

    incr ::saucer_ind
  }

  set ::saucer_angle($ind) 0
  set ::saucer_da($ind) 0

  set y [randIn 0 0.5]

  if {$y < 0.25} {
    set y [expr {0.625 + $y}]
  } else {
    set y [expr {0.375 - ($y - 0.25)}]
  }

  set ::saucer_x($ind) -1.0
  set ::saucer_y($ind) $y

  set ::saucer_vx($ind) 0.006
  set ::saucer_vy($ind) 0.0

  set ::saucer_ifire($ind) 30
  set ::saucer_fire($ind) $::saucer_ifire($ind)

  $::saucer_obj($ind) set offset [list $::saucer_x($ind) $::saucer_y($ind)]

  $::saucer_obj($ind) set scale 0.06

  $::saucer_obj($ind) set visible 1

  $::saucer_obj($ind) set meta big

  return $ind
}

proc hitSaucer { i } {
  # echo "Hit Saucer"

  $::saucer_obj($i) set visible 0

  set meta [$::saucer_obj($i) get meta]

  if {$meta == "small"} {
    set ::score [expr {$::score + 1000}]
  } else {
    set ::score [expr {$::score + 500}]
  }

  $::score_text set text "Score: $::score"
}

proc addBullet { } {
  set dt [expr {$::ticks - $::last_bullet_ticks}]
  if {$dt < 5} { return -1 }

  set ::last_bullet_ticks $::ticks

  #---

  set ind -1

  for {set i 0} {$i < $::bullet_ind} {incr i} {
    if {! [$::bullet_obj($i) get visible]} {
      set ind $i
      break
    }
  }

  if {$ind < 0} {
    set ind $::bullet_ind

    set ::bullet_obj($ind) [sb::point_list 5px]

    addObjectPoints $::bullet_obj($ind) \
      [list { 0.000 -0.100} { 0.025 -0.025} { 0.100  0.000} { 0.025  0.025} \
            { 0.000  0.100} {-0.025  0.025} {-0.100  0.000} {-0.025 -0.025}]

    $::bullet_obj($ind) set scale 0.02

    incr ::bullet_ind
  }

  set ::bullet_angle($ind) 0
  set ::bullet_da($ind) 0

  set ::bullet_x($ind) 0
  set ::bullet_y($ind) 0

  set ::bullet_x1($ind) 0
  set ::bullet_y1($ind) 0

  set ::bullet_vx($ind) 0
  set ::bullet_vy($ind) 0

  set ::bullet_life($ind) 30

  $::bullet_obj($ind) set visible 1

  return $ind
}

proc scaledSpeed { x y } {
  return [list [expr {$x/30}] [expr {$y/30}]]
}

proc init { } {
  sb::canvas set buffered 1

# sb::canvas set blend.enabled 1
# sb::canvas set blend.factor  0.8

  sb::canvas set range {-1 -1 1 1}

  sb::canvas set brush.color black

  addShip

  set ::ship_spawn 5

  set ::rock_ind 0

  set ::saucer_ind 0

  set ::big_saucer_iticks 641
  set ::big_saucer_ticks  $::big_saucer_iticks

  set ::small_saucer_iticks 943
  set ::small_saucer_ticks  $::small_saucer_iticks

  set ::bullet_ind        0
  set ::bullet_speed      0.05
  set ::last_bullet_ticks -100

  set ::lives 3
  set ::score 0
  set ::level 1

  set ::lives_text [addTextLabel "Lives: $::lives" {-0.9 0.9} left  ]
  set ::score_text [addTextLabel "Score: $::score" { 0.0 0.9} center]
  set ::level_text [addTextLabel "Level: $::level" { 0.9 0.9} right ]

  set ::game_over_text [addTextLabel "GAME OVER" {0 0} center ]
  $::game_over_text set visible 0

  startLevel

  sb::canvas set play 1
}

proc startLevel { } {
  newRock1 {-1.0 -1.0} 0.0 [scaledSpeed  0.05  0.05] 0.003
  newRock1 { 1.0 -1.0} 0.0 [scaledSpeed -0.06  0.06] 0.004
  newRock1 {-1.0  1.0} 0.0 [scaledSpeed  0.07 -0.07] 0.005
  newRock1 { 1.0  1.0} 0.0 [scaledSpeed -0.08 -0.08] 0.006
}

proc deg_to_rad { a } {
  return [expr {3.141592653*$a/180.0}]
}

proc wrapX { x } {
  upvar $x x1

  set xmin [sb::canvas get range.xmin]
  set xmax [sb::canvas get range.xmax]

  if {$x1 > $xmax} { set x1 $xmin }
  if {$x1 < $xmin} { set x1 $xmax }

  return $x1
}

proc wrapY { y } {
  upvar $y y1

  set ymin [sb::canvas get range.ymin]
  set ymax [sb::canvas get range.ymax]

  if {$y1 > $ymax} { set y1 $ymin }
  if {$y1 < $ymin} { set y1 $ymax }

  return $y1
}

proc shipThrust { } {
  $::ship_obj        set visible 0
  $::ship_thrust_obj set visible 1

  set ship_rad [deg_to_rad $::ship_angle]

  set ship_dx [expr {cos($ship_rad)}]
  set ship_dy [expr {sin($ship_rad)}]

  set ::ship_vx [expr {$::ship_vx + $::ship_thrust*$ship_dx}]
  set ::ship_vy [expr {$::ship_vy + $::ship_thrust*$ship_dy}]
}

proc shipShoot { } {
  set ind [addBullet]

  if {$ind >= 0} {
    set ::bullet_x($ind) $::ship_x
    set ::bullet_y($ind) $::ship_y

    set ::bullet_x1($ind) $::bullet_x($ind)
    set ::bullet_y1($ind) $::bullet_y($ind)

    set ship_rad [deg_to_rad $::ship_angle]

    set ship_dx [expr {cos($ship_rad)}]
    set ship_dy [expr {sin($ship_rad)}]

    set ::bullet_vx($ind) [expr {$ship_dx*$::bullet_speed}]
    set ::bullet_vy($ind) [expr {$ship_dy*$::bullet_speed}]

    $::bullet_obj($ind) set offset [list $::bullet_x($ind) $::bullet_y($ind)]

    $::bullet_obj($ind) set meta ship
  }
}

proc hitShip { } {
  if {$::lives >= 1} {
    set ::lives [expr {$::lives - 1}]

    $::lives_text set text "Lives: $::lives"
  }

  $::ship_obj        set visible 0
  $::ship_thrust_obj set visible 0

  if {$::lives == 0} {
    $::game_over_text set visible 1

    sb::canvas set play 0
  } else {
    set ::ship_spawn 100
  }
}

proc saucerShoot { i } {
  if {! [isShipVisible]} {
    return
  }

  set ind [addBullet]

  if {$ind >= 0} {
    set ::bullet_x($ind) $::saucer_x($i)
    set ::bullet_y($ind) $::saucer_y($i)

    set ::bullet_x1($ind) $::bullet_x($ind)
    set ::bullet_y1($ind) $::bullet_y($ind)

    set dx [expr {$::ship_x - $::saucer_x($i)}]
    set dy [expr {$::ship_y - $::saucer_y($i)}]

    set v [hypot $dx $dy]

    set ::bullet_vx($ind) [expr {$dx*$::bullet_speed/$v}]
    set ::bullet_vy($ind) [expr {$dy*$::bullet_speed/$v}]

    $::bullet_obj($ind) set offset [list $::bullet_x($ind) $::bullet_y($ind)]

    $::bullet_obj($ind) set meta saucer
  }
}

proc isShipVisible { } {
  set ship_visible 1

  if {! [$::ship_obj get visible] && ! [$::ship_thrust_obj get visible]} {
    set ship_visible 0
  }

  return $ship_visible
}

proc update { } {
  set ::ticks [sb::canvas get ticks]

  #---

  set ship_visible [isShipVisible]

  if {! $ship_visible} {
    incr ::ship_spawn -1

    if {$::ship_spawn < 0} {
      initShip

      $::ship_obj set visible 1

      $::ship_obj        set offset [list $::ship_x $::ship_y]
      $::ship_thrust_obj set offset [list $::ship_x $::ship_y]

      set ship_visible 1
    }
  }

  #---

  # handle ship key state

  if {$ship_visible} {
    set da 4

    if       {[sb::canvas get key "z"]} {
      set ::ship_angle [expr {$::ship_angle + $da}]
    } elseif {[sb::canvas get key "x"]} {
      set ::ship_angle [expr {$::ship_angle - $da}]
    }

    if {[sb::canvas get key ","]} {
      shipThrust
    } else {
      $::ship_obj        set visible 1
      $::ship_thrust_obj set visible 0
    }

    if {[sb::canvas get key "."]} {
      shipShoot
    }

    #---

    # move ship

    $::ship_obj        set angle $::ship_angle
    $::ship_thrust_obj set angle $::ship_angle

    set ::ship_x [expr {$::ship_x + $::ship_vx}]
    set ::ship_y [expr {$::ship_y + $::ship_vy}]

    wrapX ::ship_x
    wrapY ::ship_y

    $::ship_obj        set offset [list $::ship_x $::ship_y]
    $::ship_thrust_obj set offset [list $::ship_x $::ship_y]
  }

  #---

  # move rocks

  for {set i 0} {$i < $::rock_ind} {incr i} {
    if {! [$::rock_obj($i) get visible]} { continue }

    set ::rock_angle($i) [expr {$::rock_angle($i) + $::rock_da($i)}]

    $::rock_obj($i) set angle $::rock_angle($i)

    set ::rock_x($i) [expr {$::rock_x($i) + $::rock_vx($i)}]
    set ::rock_y($i) [expr {$::rock_y($i) + $::rock_vy($i)}]

    wrapX ::rock_x($i)
    wrapY ::rock_y($i)

    $::rock_obj($i) set offset [list $::rock_x($i) $::rock_y($i)]
  }

  #---

  # move saucers

  set xmax [sb::canvas get range.xmax]

  for {set i 0} {$i < $::saucer_ind} {incr i} {
    if {! [$::saucer_obj($i) get visible]} { continue }

    set ::saucer_angle($i) [expr {$::saucer_angle($i) + $::saucer_da($i)}]

    $::saucer_obj($i) set angle $::saucer_angle($i)

    set ::saucer_x($i) [expr {$::saucer_x($i) + $::saucer_vx($i)}]
    set ::saucer_y($i) [expr {$::saucer_y($i) + $::saucer_vy($i)}]

    if {$::saucer_x($i) > $xmax} {
      $::saucer_obj($i) set visible 0

      # echo "$::small_saucer_ticks $::big_saucer_ticks"
    }

    $::saucer_obj($i) set offset [list $::saucer_x($i) $::saucer_y($i)]

    incr ::saucer_fire($i) -1

    if {$::saucer_fire($i) < 0} {
      saucerShoot $i

      set ::saucer_fire($i) $::saucer_ifire($i)
    }
  }

  #---

  # move bullets

  for {set i 0} {$i < $::bullet_ind} {incr i} {
    if {! [$::bullet_obj($i) get visible]} { continue }

    set ::bullet_angle($i) [expr {$::bullet_angle($i) + $::bullet_da($i)}]

    $::bullet_obj($i) set angle $::bullet_angle($i)

    set ::bullet_x1($i) $::bullet_x($i)
    set ::bullet_y1($i) $::bullet_y($i)

    set ::bullet_x($i) [expr {$::bullet_x($i) + $::bullet_vx($i)}]
    set ::bullet_y($i) [expr {$::bullet_y($i) + $::bullet_vy($i)}]

    wrapX ::bullet_x($i)
    wrapY ::bullet_y($i)

    set bullet_pos [list $::bullet_x($i) $::bullet_y($i)]

    $::bullet_obj($i) set offset $bullet_pos
  }

  #---

  incr ::small_saucer_ticks -1

  if {$::small_saucer_ticks <= 0} {
    addSmallSaucer

    set ::small_saucer_ticks $::small_saucer_iticks
  }

  incr ::big_saucer_ticks -1

  if {$::big_saucer_ticks <= 0} {
    addBigSaucer

    set ::big_saucer_ticks $::big_saucer_iticks
  }

  #---

  if {$::ticks < 100} {
    return
  }

  #----

  if {$ship_visible} {
    # intersect ship and rocks
    for {set i 0} {$i < $::rock_ind} {incr i} {
      if {! [$::rock_obj($i) get visible]} { continue }

      if {[$::rock_obj($i) get intersect_obj $::ship_obj]} {
        $::ship_obj set visible 0

        hitRock $i
        hitShip

        break
      }
    }
  }

  # intersect bullets
  for {set i 0} {$i < $::bullet_ind} {incr i} {
    if {! [$::bullet_obj($i) get visible]} { continue }

    set bullet_pos [list $::bullet_x($i) $::bullet_y($i)]

    set meta [$::bullet_obj($i) get meta]

    set hit 0

    # intersect bullets and rocks
    for {set j 0} {$j < $::rock_ind} {incr j} {
      if {! [$::rock_obj($j) get visible]} { continue }

      if {[$::rock_obj($j) get intersect $bullet_pos]} {
        set hit 1

        hitRock $j

        break
      }
    }

    if {$hit == 1} {
      $::bullet_obj($i) set visible 0
      break
    }

    if {$meta == "ship"} {
      # intersect ship bullets and saucers
      for {set j 0} {$j < $::saucer_ind} {incr j} {
        if {! [$::saucer_obj($j) get visible]} { continue }
      
        if {[$::saucer_obj($j) get intersect $bullet_pos]} {
          set hit 1
    
          hitSaucer $j
    
          break
        }
      }
    } else {
      if {$ship_visible} {
        # intersect saucer bullets and ship
        if {[$::ship_obj get intersect $bullet_pos]} {
          set hit 1

          $::bullet_obj($i) set visible 0
   
          hitShip
   
          break
        }
      }
    }

    if {$hit == 1} {
      $::bullet_obj($i) set visible 0
      break
    }

    incr ::bullet_life($i) -1

    if {$::bullet_life($i) < 0} {
      $::bullet_obj($i) set visible 0
    }
  }

  #---

  if {$::num_rocks <= 0} {
    startLevel
  }
}
