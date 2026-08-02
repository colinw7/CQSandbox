# TODO
#  . alien spawn
#  . better scanner
#  . 9 screens wide
#  . bullet
#  . rotating ground
#  . alien hit ship
#  . alien shoot when no human
#  . explode alien

# ---

namespace eval tcl::mathfunc {
  proc min { args } {
    if {[lindex $args 0] < [lindex $args 1]} {
      return [lindex $args 0]
    }
    return [lindex $args 1]
  }

  proc max { args } {
    if {[lindex $args 0] > [lindex $args 1]} {
      return [lindex $args 0]
    }
    return [lindex $args 1]
  }
}

proc randIn { min max } {
  return [expr {rand()*($max - $min) + $min}]
}

proc hypot { dx dy } {
  return [expr {sqrt($dx*$dx + $dy*$dy)}]
}

# ---

# Globals

set debug 0

set main_view sb::viewport_obj.1

# ---

# 3D View

sb::canvas set play 1

sb::canvas set brush.color "#444444"

$main_view set range {-1 -1 1 1}
# $main_view set clip {0 -1 0 1}

set scanner [sb::viewport [list 0.3 0.85 0.7 0.95]]

$scanner set brush.color "#222222"

$scanner set range {-4.5 -1 4.5 1}
# $scanner set clip {-1 0 1 0}

# ---

# State

proc addOverlayText { str pos align } {
  set text [sb::text $pos $str]

  $text set pen.color   white
  $text set brush.color white
  $text set align       $align

  return $text
}

set lives_text [addOverlayText "Lives: 0" {-0.9 0.9} left  ]
set score_text [addOverlayText "Score: 0" { 0.0 0.9} center]
set level_text [addOverlayText "Level: 0" { 0.9 0.9} right ]

set game_over_text [addOverlayText "GAME OVER" {0 0} center ]

$game_over_text set visible 0

# ---

# Aliens

proc loadAlien { filename name } {
  set alien [sb::image {0 0 px} $filename]

  $alien set scale {0.2 0.2}

  return $alien
}

proc addAlien { } {
  if {$::dead_aliens >= $::num_aliens} {
    return
  }

  set i ""

  for {set ii 0} {$ii < $::num_aliens} {incr ii} {
    if {! [info exists ::alien($ii)]} {
      set meta ""

      set alien1_obj [loadAlien "defender/alien1.png" "alien1"]
      set alien2_obj [loadAlien "defender/alien2.png" "alien2"]
      set alien3_obj [loadAlien "defender/alien3.png" "alien3"]

      set ::alien1($ii) $alien1_obj
      set ::alien2($ii) $alien2_obj
      set ::alien3($ii) $alien3_obj

      $::alien1($ii) set visible 0
      $::alien2($ii) set visible 0
      $::alien3($ii) set visible 0

      set ::alien($ii) $::alien1($ii)

      sb::canvas set view $::scanner

      set ::scanner_alien($ii) [sb::circle {0 0} 4px]

      $::scanner_alien($ii) set brush.color red

      sb::canvas set view $::main_view

      set meta alien1

      set i $ii

      break
    } else {
      set meta [$::alien($ii) get meta]

      if {$meta == "dead"} {
        set meta alien1

        set ::alien($ii) $::alien1($ii)

        set i $ii

        break
      }
    }
  }

  if {$i != ""} {
    echo "Add Alien $i"

    set ::alien_x($i) [randIn -1 1]
    set ::alien_y($i) [randIn -0.75 0.75]

    $::alien($i) set id "alien.${i}"
    $::alien($i) set meta $meta
    $::alien($i) set visible 1

    positionAlien $i

    set ::alien_target($i) "human"
    set ::alien_human($i) ""

    set ::alien_target_x($i) [randIn -1 1]
    set ::alien_target_y($i) [randIn -1 1]
  }
}

proc alienScore { alien } {
  set meta [$alien get meta]

  if {$meta == "alien1"} { return 100 }
  if {$meta == "alien2"} { return 200 }
  if {$meta == "alien3"} { return 300 }

  # echo "No Score for Alien $meta"

  return 0
}

proc alienExists { i } {
  if {[info exists ::alien($i)]} {
    set meta [$::alien($i) get meta]

    if {$meta != "dead"} {
      return 1
    }
  }

  return 0
}

proc nearestHuman { pos } {
  set x [lindex $pos 0]
  set y [lindex $pos 1]

  set min_human ""
  set min_dist  0

  for {set i 0} {$i < $::num_humans} {incr i} {
    set meta [$::humans($i) get meta]
    if {$meta != "ground"} { continue }

    set dx [expr {$::human_x($i) - $x}]
    set dy [expr {$::human_y($i) - $y}]

    set dist [hypot $dx $dy]

    if {$min_human == "" || $dist < $min_dist} {
      set min_human $i
      set min_dist  $dist
    }
  }

  return $min_human
}

proc numAliveHumans { } {
  set n 0

  for {set i 0} {$i < $::num_humans} {incr i} {
    set meta [$::humans($i) get meta]
    if {$meta != "dead"} { incr n }
  }

  return $n
}

proc mutateAlien { i } {
  echo "Mutate Alien"

  set ih $::alien_human($i)

  set ::alien_target($i) "mutant"
  set ::alien_human($i) ""

  $::alien($i) set visible 0

  set ::alien($i) $::alien3($i)

  $::alien($i) set visible 1

  positionAlien $i

  if {$ih != ""} {
    set human $::humans($ih)

    $human set visible 0
    $human set meta "dead"
  }

  # move to ship
  set ::alien_target_x($i) $::ship_x
  set ::alien_target_y($i) $::ship_y
}

proc alienPickupHuman { i ih } {
  echo "Alien pickup Human"

  set ::alien_target($i) "escape"
  set ::alien_human($i) $ih

  $::alien($i) set visible 0

  set ::alien($i) $::alien2($i)

  $::alien($i) set meta "alien2"
  $::alien($i) set visible 1

  positionAlien $i

  $::humans($ih) set meta "captured"
}

# alien hit ship
proc alienHitShip { i } {
  echo "Hit Ship"

  killAlien $i

  shipLoseLife
}

proc killAlien { i } {
  set ::score [expr {$::score + [alienScore $::alien($i)]}]

  $::alien($i) set meta   "dead"
  $::alien($i) set visible 0

  incr ::dead_aliens

  $::score_text set text "Score: $::score"

  if {$::dead_aliens == $::num_aliens} {
    nextLevel
  }
}

proc moveAlienToTarget { i } {
  set target_dx [expr {$::alien_target_x($i) - $::alien_x($i)}]
  set target_dy [expr {$::alien_target_y($i) - $::alien_y($i)}]

  set l [hypot $target_dx $target_dy]

  set ::alien_x($i) [wrapX [expr {$::alien_x($i) + $::alien_v*$target_dx/$l}]]
  set ::alien_y($i) [expr {$::alien_y($i) + $::alien_v*$target_dy/$l}]

  positionAlien $i

  if {$l < 0.01} {
    return 1
  }

  return 0
}

proc moveAlien { i } {
  set ::alien_x($i) [wrapX [expr {$::alien_x($i) - $::ship_vx}]]

  # find human
  if     {$::alien_target($i) == "human"} {
    set n [numAliveHumans]

    if {$n == 0} {
      mutateAlien $i
      return
    }

    set pos [list $::alien_x($i) $::alien_y($i) 0]

    set ih [nearestHuman $pos]

    if {$ih != ""} {
      set ::alien_target_x($i) $::human_x($ih)
      set ::alien_target_y($i) $::human_y($ih)
    }

    if {[moveAlienToTarget $i]} {
      # pick up human and escape
      if {$ih != ""} {
        alienPickupHuman $i $ih
      } else {
        # move to random point
        set ::alien_target_x($i) [randIn -1 1]
        set ::alien_target_y($i) [randIn -1 1]

        if {[randIn 0 1] < 0.1} {
          alienShoot $i
        }
      }
    }
  } elseif {$::alien_target($i) == "escape"} {
    # move to top of screen
    set ::alien_target_x($i) $::alien_x($i)
    set ::alien_target_y($i) 1

    set ih $::alien_human($i)

    set human $::humans($ih)

    if {[moveAlienToTarget $i]} {
      # escaped with human, become mutant
      mutateAlien $i
    } else {
      # move human with alien
      set ::human_x($ih) $::alien_x($i)
      set ::human_y($ih) [expr {$::alien_y($i) - 0.1}]

      setHumanPosition $ih
    }
  } elseif {$::alien_target($i) == "mutant"} {
    set ::alien_target_x($i) $::ship_x
    set ::alien_target_y($i) $::ship_y

    if {[moveAlienToTarget $i]} {
      # hit ship
      alienHitShip $i
    }
  }
}

proc hitAliens { pos } {
  set px [lindex $pos 0]
  set py [lindex $pos 1]

  for {set ii 0} {$ii < $::num_aliens} {incr ii} {
    if {[info exists ::alien($ii)]} {
      set visible [$::alien($ii) get visible]
      if {! $visible} { continue }

      set x1 [expr {$::alien_x($ii) - $::alien_w/2}]
      set y1 [expr {$::alien_y($ii) - $::alien_h/2}]
      set x2 [expr {$x1 + $::alien_w}]
      set y2 [expr {$y1 + $::alien_h}]

      if {$px >= $x1 && $px < $x2 && $py >= $y1 && $py < $y2} {
        return $ii
      }
    }
  }

  return ""
}

proc alienShoot { ind } {
  set x [expr {$::alien_x($ind)}]
  set y [expr {$::alien_y($ind) - $::alien_h/2}]

  if {! [info exists ::alien_bullet($ind)]} {
    set ::alien_bullet($ind) [sb::particle [list $x $y]]

    $::alien_bullet($ind) set image     "defender/particle1.png"
    $::alien_bullet($ind) set pen.color "white"
    $::alien_bullet($ind) set meta      "alien $ind"
  } else {
    set dead [$::alien_bullet($ind) get dead]
    if {! $dead} return

    $::alien_bullet($ind) set position {$x $y}
    $::alien_bullet($ind) set dead 0
  }

  set pos [$::alien_bullet($ind) get position]

  set target_dx [expr {$::ship_x - [lindex $pos 0]}]
  set target_dy [expr {$::ship_y - [lindex $pos 1]}]

  set l [hypot $target_dx $target_dy]

  if {$l > 0} {
    set vx [expr {1.0*$target_dx/$l}]
    set vy [expr {1.0*$target_dy/$l}]

    $::alien_bullet($ind) set velocity [list $vx $vy 0]
  }
}

proc positionAlien { ind } {
  set pos [list $::alien_x($ind) $::alien_y($ind)]

  $::alien($ind) set center $pos

  $::scanner_alien($ind) set center $pos
}
  
# ---

# Ship

proc addShip { } {
  if {! [info exists ::ship_obj]} {
    set ::ship_obj [sb::image {0 0 px} "defender/ship.png"]

    $::ship_obj set scale {0.2 0.2}

    sb::canvas set view $::scanner

    set ::scanner_ship [sb::circle {0 0} 4px]

    $::scanner_ship set brush.color yellow

    sb::canvas set view $::main_view
  }
}

proc hitShip { pos } {
  set px [lindex $pos 0]
  set py [lindex $pos 1]

  set x1 [expr {$::ship_x - $::ship_w/2}]
  set y1 [expr {$::ship_y - $::ship_h/2}]
  set x2 [expr {$x1 + $::ship_w}]
  set y2 [expr {$y1 + $::ship_h}]

  if {$px >= $x1 && $px < $x2 && $py >= $y1 && $py < $y2} {
    return 1
  }

  return 0
}

proc gameOver { } {
  $::ship_obj set visible 0

  $::game_over_text set visible 1

  sb::canvas set play 0
}

proc nextLevel { } {
  restartGame 1
}

proc restartGame { nextLevel } {
  sb::canvas set play 0

  #---

  initGround

  #---

  set ::num_aliens  8
  set ::dead_aliens 0
  set ::alien_dx    0
  set ::alien_dy    0
  set ::alien_w     0.1
  set ::alien_h     0.1
  set ::alien_v     0.02

  for {set i 0} {$i < $::num_aliens} {incr i} {
    if {[info exists ::alien_bullet($i)]} {
      $::alien_bullet($i) set dead 0
    }

    if {[info exists ::alien($i)]} {
      $::alien($i) set meta "dead"
      $::alien($i) set visible 0

      set ::alien($i) $::alien1($i)

      $::alien($i) set meta "dead"
      $::alien($i) set visible 0
    }

    set ::alien_target($i) "human"
    set ::alien_human($i) ""
  }

  set ::num_init_aliens 2

  for {set i 0} {$i < $::num_init_aliens} {incr i} {
    addAlien
  }

  #---

  if {$nextLevel == 0} {
    set ::lives 3
    set ::score 0
    set ::level 1

    $::lives_text set text "Lives: $::lives"
    $::score_text set text "Score: $::score"
    $::level_text set text "Level: $::level"

    if {[info exists ::ship_dir]} {
      if {$::ship_dir != 1} {
        $::ship_obj set flip_x
      }
    }
  } else {
    incr ::level

    $::level_text set text "Level: $::level"
  }

  #---

  addShip

  if {$nextLevel == 0} {
    set ::ship_dx     0.1
    set ::ship_w      0.1
    set ::ship_h      0.1
    set ::ship_dir    1
    set ::ship_vx     0.05
    set ::ship_ax     0.05
    set ::ship_min_vx 0.01
    set ::ship_max_vx 0.2
    set ::ship_vy     0.04

    set ::ship_target_x ""
    set ::ship_turning  0

    set ::ship_bullet_x1   0
    set ::ship_bullet_xmax 2

    set ::ship_x -$::ship_dx
    set ::ship_y 0

    # ---

    set ::ship_num_bullets 4

    for {set i 0} {$i < $::ship_num_bullets} {incr i} {
      set ::ship_bullet($i) ""
    }
  }

  updateShipPosition

  # ---

  set ::num_humans 8

  for {set i 0} {$i < $::num_humans} {incr i} {
    addHuman $i [randIn -4.5 4.5]
  }

  # ---

  foreach particle [sb::canvas get particles] {
    $particle set dead 1
  }

  # ---

  # ""         set bbox [list -40 -40 40 40]
  # $::scanner set bbox [list -40 -40 40 40]

  # ---

  sb::canvas set play 1
}

proc shipLoseLife { } {
  if {$::lives >= 1} {
    set ::lives [expr {$::lives - 1}]
  }

  if {$::lives == 0} {
    gameOver
  }

  $::lives_text set text "Lives: $::lives"
}

proc shipUp { } {
  echo "shipUp"

  set ::ship_y [limitShipY [expr {$::ship_y + $::ship_vy}]]

  updateShipPosition
}

proc shipDown { } {
  echo "shipDown"

  set ::ship_y [limitShipY [expr {$::ship_y - $::ship_vy}]]

  updateShipPosition
}

proc shipTurn { } {
  echo "shipTurn"

  if {! $::ship_turning} {
    set ::ship_turning 1

    $::ship_obj set flip_x 1

    set ::ship_dir [expr {-$::ship_dir}]
  # set ::ship_vx  [expr {-$::ship_vx}]

    if {$::ship_dir > 0} {
      set ::ship_target_x -$::ship_dx
    } else {
      set ::ship_target_x $::ship_dx
    }

    updateShipPosition
  }
}

proc shipThrust { } {
  if {$::ship_dir > 0} {
    set ::ship_vx [expr {$::ship_vx + $::ship_ax}]
  } else {
    set ::ship_vx [expr {$::ship_vx - $::ship_ax}]
  }

  if {$::ship_vx > $::ship_max_vx} {
    set ::ship_vx $::ship_max_vx
  }
  if {$::ship_vx < -$::ship_max_vx} {
    set ::ship_vx -$::ship_max_vx
  }

  if {$::ship_vx <= 0 && $::ship_vx > -$::ship_min_vx} {
    set ::ship_vx -$::ship_min_vx
  }
  if {$::ship_vx >= 0 && $::ship_vx < $::ship_min_vx} {
    set ::ship_vx $::ship_min_vx
  }

  if {0} {
  if {$::ship_vx > 0} {
    set ::ship_x -$::ship_dx
  } else {
    set ::ship_x $::ship_dx
  }

  updateShipPosition
  }
}

proc shipShoot { } {
  # get free bullet
  set bullet_num ""

  for {set i 0} {$i < $::ship_num_bullets} {incr i} {
    if {$::ship_bullet($i) != ""} {
      set dead [$::ship_bullet($i) get dead]

      if {$dead} {
        set bullet_num $i
        break
      }
    } else {
      set bullet_num $i
    }
  }

  if {$bullet_num == ""} {
    return
  }

  # ---

  # set bullet start position
  if {$::ship_dir > 0} {
    set ::ship_bullet_x1 [expr {$::ship_x + 0.1}]
  } else {
    set ::ship_bullet_x1 [expr {$::ship_x - 0.1}]
  }

  set ::ship_bullet_y1 [expr {$::ship_y + [randIn -0.01 0.01]}]

  # ---

  # add bullet particle if needed
  if {$::ship_bullet($bullet_num) == ""} {
    set ::ship_bullet($bullet_num) [sb::particle [list $::ship_bullet_x1 $::ship_bullet_y1]]

    $::ship_bullet($bullet_num) set image     "defender/particle1.png"
    $::ship_bullet($bullet_num) set pen.color "white"
    $::ship_bullet($bullet_num) set meta      "ship"
  }

  # ---

  # reset bullet state
  $::ship_bullet($bullet_num) set dead     0
  $::ship_bullet($bullet_num) set position [list $::ship_bullet_x1 $::ship_bullet_y1]

  # ---

  # set bullet velocity
  set bullet_vx 10

  set shoot_vel [expr {$::ship_vx + $bullet_vx*$::ship_dir}]

  set ::ship_shoot_vel [list $shoot_vel 0]

  $::ship_bullet($bullet_num) set velocity $::ship_shoot_vel
}

# ---

# Ground

proc addGround { x } {
  set ground_obj [sb::image {0 0 px} "defender/ground.png"]

  $ground_obj set scale {0.5 0.5}

  return $ground_obj
}

proc initGround { } {
  set ::ground_dx   0.5
  set ::ground_xmin -4.5
  set ::ground_xmax 4.5
  set ::ground_w    9.0
  set ::ground_y    -0.5

  set ::num_ground 18

  set ::ground_x(0) $::ground_xmin

  for {set i 0} {$i < $::num_ground} {incr i} {
    if {$i > 0} {
      set i1 [expr {$i - 1}]

      set ::ground_x($i) [expr {$::ground_x($i1) + $::ground_dx}]
    }

    if {! [info exists ::ground_obj($i)]} {
      set ::ground_obj($i) [addGround $::ground_x($i)]
    }
  }
}

# ---

# Human

proc addHuman { i x } {
  set ::human_x($i) $x
  set ::human_y($i) $::ground_y

  if {! [info exists ::humans($i)]} {
    set ::humans($i) [sb::image {0 0 px} "defender/human.png"]

    $::humans($i) set scale {0.2 0.2}

    sb::canvas set view $::scanner

    set ::scanner_human($i) [sb::circle {0 0} 4px]
  
    $::scanner_human($i) set brush.color green

    sb::canvas set view $::main_view

    setHumanPosition $i
  }

  $::humans($i) set meta    "ground"
  $::humans($i) set visible 1

  return $::humans($i)
}

# ---

proc wrapX { x } {
  if       {$x >= $::ground_xmax} {
    set x [expr {$x - $::ground_w}]
  } elseif {$x <= $::ground_xmin} {
    set x [expr {$x + $::ground_w}]
  }
  return $x
}

proc limitShipY { y } {
  if       {$y > 48} {
    set y 48
  } elseif {$y < $::ground_y} {
    set y $::ground_y
  }
  return $y
}

# ---

proc hitAlien { } {
}

proc updateShipBullet { particle } {
  set pos [$particle get position]

  # if bullet hit invader then destroy alien and bullet and update any grabbed human
  set ii [hitAliens $pos]

  if {$ii != ""} {
    set alien $::alien($ii)

    echo "Hit Alien: $alien"

    $particle set dead 1

    killAlien $ii

    if {$::alien_human($ii) != ""} {
      set human $::humans($::alien_human($ii))

      $human set meta "falling"
    }

    set ::alien_target($ii) ""
    set ::alien_human($ii) ""

    return
  }

  set x [lindex $pos 0]

  set vel [$particle get velocity]

  set vx [lindex $vel 0]

  # off screen so destroy bullet
  set dist [expr {abs($x - $::ship_bullet_x1)}]

  if {$dist > $::ship_bullet_xmax} {
    $particle set dead 1
    return
  }
}

proc updateAlienBullet { particle } {
  set pos [$particle get position]

  if {[hitShip $pos]} {
    $particle set dead 1

    shipLoseLife

    return
  }

  set y [lindex $pos 1]

  if {$y < -80} {
    $particle set dead 1

    return
  }
}

proc updateGround { } {
  # Move Ground
  for {set i 0} {$i < $::num_ground} {incr i} {
    set ::ground_x($i) [wrapX [expr {$::ground_x($i) - $::ship_vx}]]

    $::ground_obj($i) set position [list $::ground_x($i) $::ground_y]
  }
}

proc updateHumans { } {
  for {set i 0} {$i < $::num_humans} {incr i} {
    set meta [$::humans($i) get meta]

    if       {$meta == "ground"} {
      # Move Humans (on Ground)
      set ::human_x($i) [wrapX [expr {$::human_x($i) - $::ship_vx}]]

      setHumanPosition $i
    } elseif {$meta == "captured"} {
      # Updated by moveAlien
    } elseif {$meta == "falling"} {
      set ::human_y($i) [expr {$::human_y($i) - 0.1}]

      setHumanPosition $i

      if {$::human_y($i) <= $::ground_y } {
        $::humans($i) set meta "ground"
      }
    }
  }
}

proc setHumanPosition { ind } {
  $::humans($ind) set center [list $::human_x($ind) $::human_y($ind)]

  $::scanner_human($ind) set center [list $::human_x($ind) $::human_y($ind)]
}

proc moveShip { } {
  if {$::ship_turning} {
    set target_dx [expr {$::ship_target_x - $::ship_x}]
    set target_dy 0

    set l [hypot $target_dx $target_dy]

    if {$l > 1} {
      set v [expr {0.015*sqrt($l)}]

      set ::ship_x [expr {$::ship_x + $v*$target_dx/$l}]

      updateShipPosition
    } else {
      set ::ship_target_x ""
      set ::ship_turning  0
    }
  }
}

proc updateShipPosition { } {
  $::ship_obj set center [list $::ship_x $::ship_y]

  $::scanner_ship set center [list $::ship_x $::ship_y]
}

proc update { args } {
  set ::ticks [sb::canvas get ticks]

  if       {[sb::canvas get key "q"]} {
    shipUp
  } elseif {[sb::canvas get key "z"]} {
    shipDown
  }

  if {[sb::canvas get key "a"]} {
    shipTurn
  }

  if {[sb::canvas get key "o"]} {
    shipThrust
  }

  if {[sb::canvas get key "p"]} {
    shipShoot
  }

  #---

  moveShip

  # process particles (bullets)
  foreach particle [sb::canvas get particles] {
    set dead [$particle get dead]
    if {$dead} { continue }

    set meta [$particle get meta]

    if {$meta == "ship"} {
      updateShipBullet $particle
    } else {
      updateAlienBullet $particle
    }
  }

  # update aliens
  for {set ii 0} {$ii < $::num_aliens} {incr ii} {
    if {[alienExists $ii]} {
      moveAlien $ii
    }
  }

  # add new alien
  if {[randIn 0 1] < 0.1} {
    addAlien
  }

  # move ground
  updateGround

  # move humans (with ground)
  updateHumans
}

proc keyPress { args } {
  # echo "keyPress"

  set key [lindex $args 0]

  set ctrl  [lindex $args 1]
  set shift [lindex $args 2]

  # echo "keyPress ($key) ($ctrl) ($shift)"

  if {0} {
  if       {$key == "q" || $key == "Q"} {
    shipUp
  } elseif {$key == "z" || $key == "Z"} {
    shipDown
  } elseif {$key == "a" || $key == "A"} {
    shipTurn
  } elseif {$key == "o" || $key == "O"} {
    shipThrust
  } elseif {$key == "p" || $key == "P"} {
    shipShoot
  }
  }

  if       {$key == "d" || $key == "D"} {
    set ::debug [expr {1 - $::debug}]
  } elseif {$key == "r" || $key == "R"} {
    restartGame 0
  }
}

restartGame 0
