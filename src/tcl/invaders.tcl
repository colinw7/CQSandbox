# TODO:
#  . invader shoot
#  . alien animation
#  . base destroy

proc randIn { min max } {
  return [expr {rand()*($max - $min) + $min}]
}

proc loadImage { filename name scale } {
  set obj [sb::image {0 0} $filename]

  $obj set id $name
  $obj set visible 0
  $obj set scale [list $scale $scale]

  return $obj
}

proc addTextLabel { str pos align } {
  set text [sb::text $pos $str]

  $text set pen.color white
  $text set align     $align

  return $text
}

proc getImage { filename name scale } {
  if {! [info exists ::images($name)]} {
    set ::images($name) [loadImage $filename $name $scale]
  }

  set obj [sb::image {0 0} ""]

  $obj set id      $name
  $obj set image   $::images($name)
  $obj set visible 0

  return $obj
}

proc addInvader { ix iy } {
  # echo "addInvader $ix $iy"

  set ind [expr {$iy*$::invader_ny + $ix}]

  set meta ""

  set scale 1.7

  if       {$iy == 0 || $iy == 1} {
    set ::invader($ind,0) [getImage "invaders/invader1a.png" "invader1a" $scale]
    set ::invader($ind,1) [getImage "invaders/invader1b.png" "invader1b" $scale]
    set meta invader1
  } elseif {$iy == 2 || $iy == 3} {
    set ::invader($ind,0) [getImage "invaders/invader2a.png" "invader2a" $scale]
    set ::invader($ind,1) [getImage "invaders/invader2b.png" "invader2b" $scale]
    set meta invader2
  } elseif {$iy == 4} {
    set ::invader($ind,0) [getImage "invaders/invader3a.png" "invader3a" $scale]
    set ::invader($ind,1) [getImage "invaders/invader3b.png" "invader3b" $scale]
    set meta invader3
  }

  set ::invader_x($ind) [expr {($ix - $::invader_nx)*$::invader_dw}]
  set ::invader_y($ind) [expr {$iy*$::invader_dh + 32}]

  $::invader($ind,0) set id "invader.${ind}"
  $::invader($ind,0) set meta $meta

  $::invader($ind,1) set id "invader.${ind}"
  $::invader($ind,1) set meta $meta

  $::invader($ind,$::invader_ind) set center [list $::invader_x($ind) $::invader_y($ind)]

  $::invader($ind,$::invader_ind) set visible 1
}

proc invaderScore { invader } {
  set meta [$invader get meta]

  if {$meta == "invader1"} { return 100 }
  if {$meta == "invader2"} { return 200 }
  if {$meta == "invader3"} { return 300 }

  return 0
}

proc moveInvader { ix iy } {
  set ind [expr {$iy*$::invader_ny + $ix}]

  set ::invader_x($ind) [expr {$::invader_x($ind) + $::invader_dir}]

  $::invader($ind,$::invader_ind) set center [list $::invader_x($ind) $::invader_y($ind)]
}

proc updateInvaderDx { } {
  set flip 0

  for {set iy 0} {$iy < $::invader_nx} {incr iy} {
    for {set ix 0} {$ix < $::invader_ny} {incr ix} {
      set ind [expr {$iy*$::invader_ny + $ix}]

      if {$::invader_dir > 0} {
        if {$::invader_x($ind) > $::invader_edge} {
          set flip 1
          break
        }
      } else {
        if {$::invader_x($ind) < -$::invader_edge} {
          set flip 1
          break
        }
      }
    }
  }

  if {$flip} {
    set ::invader_dir [expr {-$::invader_dir}]

    for {set iy 0} {$iy < $::invader_nx} {incr iy} {
      for {set ix 0} {$ix < $::invader_ny} {incr ix} {
        set ind [expr {$iy*$::invader_ny + $ix}]

        set ::invader_y($ind) [expr {$::invader_y($ind) - 4}]

        $::invader($ind,$::invader_ind) set center [list $::invader_x($ind) $::invader_y($ind)]
      }
    }
  }
}

proc loadBase { filename name } {
  set obj [sb::image {0 0} $filename]

  $obj set id $name
  $obj set visible 0

  return $obj
}

proc addBase { ib } {
  # echo "addBase $ib"

  set ::base_damage($ib) 0

  set ::base($ib,0,0) [loadBase "invaders/base1a_1_1.png" "base1"]
  set ::base($ib,0,1) [loadBase "invaders/base1a_2_1.png" "base1"]
  set ::base($ib,0,2) [loadBase "invaders/base1a_3_1.png" "base1"]
  set ::base($ib,0,3) [loadBase "invaders/base1a_4_1.png" "base1"]
  set ::base($ib,0,4) [loadBase "invaders/base1a_1_2.png" "base1"]
  set ::base($ib,0,5) [loadBase "invaders/base1a_2_2.png" "base1"]
  set ::base($ib,0,6) [loadBase "invaders/base1a_3_2.png" "base1"]
  set ::base($ib,0,7) [loadBase "invaders/base1a_4_2.png" "base1"]

  set ::base($ib,1,0) [loadBase "invaders/base1b_1_1.png" "base2"]
  set ::base($ib,1,1) [loadBase "invaders/base1b_2_1.png" "base2"]
  set ::base($ib,1,2) [loadBase "invaders/base1b_3_1.png" "base2"]
  set ::base($ib,1,3) [loadBase "invaders/base1b_4_1.png" "base2"]
  set ::base($ib,1,4) [loadBase "invaders/base1b_1_2.png" "base2"]
  set ::base($ib,1,5) [loadBase "invaders/base1b_2_2.png" "base2"]
  set ::base($ib,1,6) [loadBase "invaders/base1b_3_2.png" "base2"]
  set ::base($ib,1,7) [loadBase "invaders/base1b_4_2.png" "base2"]

  set ::base($ib,2,0) [loadBase "invaders/base1c_1_1.png" "base3"]
  set ::base($ib,2,1) [loadBase "invaders/base1c_2_1.png" "base3"]
  set ::base($ib,2,2) [loadBase "invaders/base1c_3_1.png" "base3"]
  set ::base($ib,2,3) [loadBase "invaders/base1c_4_1.png" "base3"]
  set ::base($ib,2,4) [loadBase "invaders/base1c_1_2.png" "base3"]
  set ::base($ib,2,5) [loadBase "invaders/base1c_2_2.png" "base3"]
  set ::base($ib,2,6) [loadBase "invaders/base1c_3_2.png" "base3"]
  set ::base($ib,2,7) [loadBase "invaders/base1c_4_2.png" "base3"]

  set ::base($ib,3,0) [loadBase "invaders/base1d_1_1.png" "base4"]
  set ::base($ib,3,1) [loadBase "invaders/base1d_2_1.png" "base4"]
  set ::base($ib,3,2) [loadBase "invaders/base1d_3_1.png" "base4"]
  set ::base($ib,3,3) [loadBase "invaders/base1d_4_1.png" "base4"]
  set ::base($ib,3,4) [loadBase "invaders/base1d_1_2.png" "base4"]
  set ::base($ib,3,5) [loadBase "invaders/base1d_2_2.png" "base4"]
  set ::base($ib,3,6) [loadBase "invaders/base1d_3_2.png" "base4"]
  set ::base($ib,3,7) [loadBase "invaders/base1d_4_2.png" "base4"]

  set ::base_x($ib) [expr {($ib - $::num_bases/2)*$::base_dx + $::base_dx/2}]

  set y $::base_y

  for {set iy 0} {$iy < 2} {incr iy} {
    set x $::base_x($ib)

    for {set ix 0} {$ix < 4} {incr ix} {
      set ind [expr {$iy*4 + $ix}]

      $::base($ib,$::base_damage($ib),$ind) set visible 1
      $::base($ib,$::base_damage($ib),$ind) set center [list $x $y]

      set x [expr {$x + 2}]
    }

    set y [expr {$y - 4}]
  }
}

proc hitInvaders { pos } {
  set px [lindex $pos 0]
  set py [lindex $pos 1]

  for {set iy 0} {$iy < $::invader_nx} {incr iy} {
    for {set ix 0} {$ix < $::invader_ny} {incr ix} {
      set ind [expr {$iy*$::invader_ny + $ix}]

      set visible [$::invader($ind,$::invader_ind) get visible]
      if {! $visible} { continue }

      set x1 [expr {$::invader_x($ind) - $::invader_w/2}]
      set y1 [expr {$::invader_y($ind) - $::invader_h/2}]
      set x2 [expr {$x1 + $::invader_w}]
      set y2 [expr {$y1 + $::invader_h}]

      if {$px >= $x1 && $px < $x2 && $py >= $y1 && $py < $y2} {
        return $::invader($ind,$::invader_ind)
      }
    }
  }

  return ""
}

proc hitBase { pos } {
  set px [lindex $pos 0]
  set py [lindex $pos 1]

  for {set ix 0} {$ix < $::num_bases} {incr ix} {
    if {$::base_damage($ix) > $::base_max_damage} { continue }

    set x1 [expr {$::base_x($ix) - $::base_w/2}]
    set y1 [expr {$::base_y      - $::base_h/2}]
    set x2 [expr {$x1 + $::base_w}]
    set y2 [expr {$y1 + $::base_h}]

    if {$px >= $x1 && $px < $x2 && $py >= $y1 && $py < $y2} {
      return $ix
    }
  }

  return ""
}

proc damageBase { ix } {
  $::base($ix,$::base_damage($ix),0) set visible 0
    
  incr ::base_damage($ix)

  if {$::base_damage($ix) <= $::base_max_damage} {
    $::base($ix,$::base_damage($ix),0) set visible 1
    $::base($ix,$::base_damage($ix),0) set center [list $::base_x($ix) $::base_y 0]
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

proc invaderShoot { ind } {
  set x [expr {$::invader_x($ind)}]
  set y [expr {$::invader_y($ind) - $::invader_h/2}]

  if {! [info exists ::invader_bullet($ind)]} {
    set obj [getImage "invaders/bullet1a.png" "invader_bullet" 1.0]

    $obj set id    "invader_bullet.$ind"
    $obj set visible 1

    set ::invader_bullet($ind) [sb::particle [list $x $y]]

    $::invader_bullet($ind) set image    $obj
    $::invader_bullet($ind) set velocity $::invader_shoot_vel
    $::invader_bullet($ind) set meta     "invader $ind"
  } else {
    set dead [$::invader_bullet($ind) get dead]

    if {$dead} {
      $::invader_bullet($ind) set position [list $x $y]
      $::invader_bullet($ind) set dead 0
    }
  }
}

proc shipShoot { } {
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

  if {$bullet_num != ""} {
    if {$::ship_bullet($bullet_num) == ""} {
      set ::ship_bullet($bullet_num) [sb::particle [list $::ship_x $::ship_y]]

      $::ship_bullet($bullet_num) set meta ship
      $::ship_bullet($bullet_num) set velocity $::ship_shoot_vel
    }

    $::ship_bullet($bullet_num) set dead     0
    $::ship_bullet($bullet_num) set position [list $::ship_x $::ship_y 0]
  }
}

proc update { args } {
  if       {[sb::canvas get key "left"]} {
    set ::ship_x [expr {$::ship_x - 1}]
 
    $::ship_obj set center [list $::ship_x $::ship_y]
  } elseif {[sb::canvas get key "right"]} {
    set ::ship_x [expr {$::ship_x + 1}]

    $::ship_obj set center [list $::ship_x $::ship_y]
  }

  if {[sb::canvas get key "space"]} {
    shipShoot
  }

  #---

  foreach particle [sb::canvas get particles] {
    set dead [$particle get dead]
    if {$dead} { continue }

    set meta [$particle get meta]

    if {$meta == "ship"} {
      set pos [$particle get position]

      set ix [hitBase $pos]
  
      if {$ix != ""} {
        #echo "Hit Base: $ix"

        damageBase $ix

        $particle set dead 1

        continue
      }

      set invader [hitInvaders $pos]

      if {$invader != ""} {
        echo "Hit Invader: $invader"

        set ::score [expr {$::score + [invaderScore $invader]}]

        $::score_text set text "Score: $::score"

        $particle set dead 1
        $invader  set visible 0

        continue
      }

      set y [lindex $pos 1]

      if {$y > 80} {
        $particle set dead 1

        continue
      }
    } else {
      set pos [$particle get position]

      if {[hitShip $pos]} {
        $particle set dead 1

        if {$::lives >= 1} {
          set ::lives [expr {$::lives - 1}]
        }

        if {$::lives == 0} {
          $::ship_obj set visible 0

          $::game_over_text set visible 1

          sb::canvas set play 0
        }

        $::lives_text set text "Lives: $::lives"

        continue
      } else {
        set ix [hitBase $pos]

        if {$ix != ""} {
          echo "Hit Base: $ix"

          damageBase $ix

          $particle set dead 1

          continue
        }
      }

      set y [lindex $pos 1]

      if {$y < -80} {
        $particle set dead 1

        continue
      }
    }
  }

  set ticks [sb::canvas get ticks]

  for {set iy 0} {$iy < $::invader_nx} {incr iy} {
    for {set ix 0} {$ix < $::invader_ny} {incr ix} {
      moveInvader $ix $iy

      if {[randIn 0 1] < 0.001} {
        set ind [expr {$iy*$::invader_ny + $ix}]

        invaderShoot $ind
      }
    }
  }

  incr ::invader_ticks

  if {$::invader_ticks == 10} {
    for {set iy 0} {$iy < $::invader_nx} {incr iy} {
      for {set ix 0} {$ix < $::invader_ny} {incr ix} {
        set ind [expr {$iy*$::invader_ny + $ix}]

        $::invader($ind,$::invader_ind) set visible 0
      }
    }

    set ::invader_ind [expr {1 - $::invader_ind}]
        
    for {set iy 0} {$iy < $::invader_nx} {incr iy} { 
      for {set ix 0} {$ix < $::invader_ny} {incr ix} {
        set ind [expr {$iy*$::invader_ny + $ix}]
        
        $::invader($ind,$::invader_ind) set visible 1
        
        $::invader($ind,$::invader_ind) set center [list $::invader_x($ind) $::invader_y($ind)]
      }
    }

    set ::invader_ticks 0
  }

  updateInvaderDx
}

proc keyPress { args } {
  # echo "keyPress"

  set key [lindex $args 0]

  set ctrl  [lindex $args 1]
  set shift [lindex $args 2]

  # echo "keyPress ($key) ($ctrl) ($shift)"

  if {0} {
    if       {$key == "left"} {
      set ::ship_x [expr {$::ship_x - 1}]

      $::ship_obj set center [list $::ship_x $::ship_y]
    } elseif {$key == "right"} {
      set ::ship_x [expr {$::ship_x + 1}]

      $::ship_obj set center [list $::ship_x $::ship_y]
    } elseif {$key == "space"} {
      shipShoot
    }
  }
}

proc init { } {
  sb::canvas set range {-100 -100 100 100}

  set ::invader_w 11
  set ::invader_h 9

  set ::invader_dw [expr {$::invader_w + 1}]
  set ::invader_dh [expr {$::invader_h + 2}]

  sb::canvas set brush.color "#444444"

  set ::invader_nx  5
  set ::invader_ny  11
  set ::invader_dir 1

  set ::invader_shoot_vel {0 -250}

# set ::invader_edge 128
  set ::invader_edge 100

  set ::invader_ticks 0
  set ::invader_ind   0

  set ::lives 3
  set ::score 0
  set ::level 1

  set ::lives_text [addTextLabel "Lives: $::lives" {-120 110} left  ]
  set ::score_text [addTextLabel "Score: $::score" {   0 110} center]
  set ::level_text [addTextLabel "Level: $::level" { 120 110} right ]

  set ::game_over_text [addTextLabel "GAME OVER" { 0 0 0} center ]
  $::game_over_text set visible 0

  for {set iy 0} {$iy < $::invader_nx} {incr iy} {
    for {set ix 0} {$ix < $::invader_ny} {incr ix} {
      addInvader $ix $iy
    }
  }

  set ::num_bases 4
  set ::base_dx   48
  set ::base_y    -64
  set ::base_w    32
  set ::base_h    32

  set ::base_max_damage 3

  for {set ix 0} {$ix < $::num_bases} {incr ix} {
    addBase $ix
  }

  set ::ship_obj [getImage "invaders/player1a.png" "ship" 1.5]

  $::ship_obj set visible 1

  set ::ship_x 0
  set ::ship_y -72
  set ::ship_w 16
  set ::ship_h 16

  set ::ship_shoot_vel {0 250}

  $::ship_obj set center [list $::ship_x $::ship_y]

  set ::ship_num_bullets 4

  for {set i 0} {$i < $::ship_num_bullets} {incr i} {
    set ::ship_bullet($i) ""
  }

  sb::canvas set play 1
}
