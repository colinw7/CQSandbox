proc randIn { min max } {
  return [expr {rand()*($max - $min) + $min}]
}

proc addPlayer { } {
  $::player_obj set visible 1

  set ::player_x1 0
  set ::player_y1 [expr {$::SCREEN_HEIGHT - $::PLAYER_AREA_HEIGHT}]
  set ::player_x2 [expr {$::SCREEN_WIDTH  - $::BLOCK_WIDTH}]
  set ::player_y2 [expr {$::SCREEN_HEIGHT - $::BLOCK_HEIGHT}]

  set ::player_x [expr {($::player_x1 + $::player_x2)/2}]
  set ::player_y $::player_y2

  $::player_obj set center [list $::player_x $::player_y]

  set ::player_lives $::INIT_LIVES
}

proc hitPlayer { } {
  incr ::player_lives -1

  if {$::player_lives <= 0} {
    gameOver
  }
}

proc playerLeft { } {
  set x [expr {$::player_x - $::PLAYER_DX}]

  set mushroom_obj [mushroomAt $x $::player_y]
  if {$mushroom_obj != ""} { return }

  set ::player_x $x

  if {$::player_x < $::player_x1} {
    set ::player_x $::player_x1
  }

  $::player_obj set center [list $::player_x $::player_y]
}

proc playerRight { } {
  set x [expr {$::player_x + $::PLAYER_DX}]

  set mushroom_obj [mushroomAt $x $::player_y]
  if {$mushroom_obj != ""} { return }

  set ::player_x $x

  if {$::player_x > $::player_x2} {
    set ::player_x $::player_x2
  }

  $::player_obj set center [list $::player_x $::player_y]
}

proc playerDown { } {
  set y [expr {$::player_y + $::PLAYER_DY}]

  set mushroom_obj [mushroomAt $::player_x $y]
  if {$mushroom_obj != ""} { return }

  set ::player_y $y

  if {$::player_y > $::player_y2} {
    set ::player_y $::player_y2
  }

  $::player_obj set center [list $::player_x $::player_y]
}

proc playerUp { } {
  set y [expr {$::player_y - $::PLAYER_DY}]

  set mushroom_obj [mushroomAt $::player_x $y]
  if {$mushroom_obj != ""} { return }

  set ::player_y $y

  if {$::player_y < $::player_y1} {
    set ::player_y $::player_y1
  }

  $::player_obj set center [list $::player_x $::player_y]
}

proc playerFire { } {
  if {! [$::bullet_obj get visible]} {
    $::bullet_obj set visible 1

    set ::bullet_x $::player_x
    set ::bullet_y $::player_y

    $::bullet_obj set center [list $::bullet_x $::bullet_y]
  }
}

proc addSegment { } {
  set obj [sb::image]

  $obj set image $::centipede_image(0)

  return $obj
}

proc addCentipede { } {
  set ind $::centipede_ind

  incr ::centipede_ind

  set ::centipede_x($ind) [expr {$::BLOCK_WIDTH*($::NUM_COLS/2.0)}]
  set ::centipede_y($ind) [expr {-$::BLOCK_HEIGHT}]

  set x $::centipede_x($ind)
  set y $::centipede_y($ind)

  for {set i 0} {$i < $::MAX_SEGMENTS} {incr i} {
    set ::segment_obj($ind,$i) [addSegment]

    $::segment_obj($ind,$i) set visible 1

    $::segment_obj($ind,$i) set center [list $x $y]

    set ::segment_x($ind,$i) $x
    set ::segment_y($ind,$i) $y

    set ::segment_dx($ind,$i) [expr {-$::BLOCK_WIDTH/4.0}]
    set ::segment_dy($ind,$i) 0

    set x [expr {$x - $::BLOCK_WIDTH}]

    $::segment_obj($ind,$i) set meta [list $ind $i]
  }

  set ::centipede_tick 0
  set ::centipede_iind 0
}

proc mushroomAt { x y } {
  set objs [$::mushroom_quad get object.at_point [list $x $y]]

  set obj ""

  if {[llength $objs] > 0} {
    set obj [lindex $objs 0]
  }

  if {$obj != "" && ! [$obj get visible]} {
    set obj ""
  }

  return $obj
if {0} {
  set col [expr {int($x/$::BLOCK_WIDTH )}]
  set row [expr {int($y/$::BLOCK_HEIGHT)}]
  
  if {$row < 0 || $row >= $::NUM_ROWS || $col < 0 || $col >= $::NUM_COLS} {
    return ""
  }

  set obj $::mushroom_obj($r,$c)

  if {$obj != "" && ! [$obj get visible]} {
    set obj ""
  }

  if {0} {
  if {$obj != ""} {
    echo "Mushroom $obj [$obj get meta] $x $y $col $row"
  }
  }

  return $obj
}
}

proc segmentAt { x y } {
  set col [expr {int($x/$::BLOCK_WIDTH )}]
  set row [expr {int($y/$::BLOCK_HEIGHT)}]

  for {set i 0} {$i < $::centipede_ind} {incr i} {
    for {set j 0} {$j < $::MAX_SEGMENTS} {incr j} {
      set obj $::segment_obj($i,$j)

      if {! [$obj get visible]} {
        continue
      }

      set x1 $::segment_x($i,$j)
      set y1 $::segment_y($i,$j)

      set col1 [expr {int($x1/$::BLOCK_WIDTH )}] 
      set row1 [expr {int($y1/$::BLOCK_HEIGHT)}]

      if {$col == $col1 && $row == $row1} {
        return $obj
      }
    }
  }

  return ""
}

proc hitCentipede { ind i } {
  set vis -1

  for {set j 0} {$j < $::MAX_SEGMENTS} {incr j} {
    set obj $::segment_obj($ind,$j)
  
    if {! [$obj get visible]} {
      continue
    }

    set vis $j
  }

  if {$vis >= 0} {
    $::segment_obj($ind,$i) set visible 0

    set x $::segment_x($ind,$i)
    set y $::segment_y($ind,$i)
  
    set c [expr {int($x/$::BLOCK_WIDTH )}]
    set r [expr {int($y/$::BLOCK_HEIGHT)}]

    $::mushroom_obj($r,$c) set visible 1

    turnSegment $ind $i
  }

  if {$vis == 0} {
    echo "kill centipede"
  }
}

proc moveSegment { i j } {
  set ::segment_x($i,$j) [expr {$::segment_x($i,$j) + $::segment_dx($i,$j)}]

  if       {$::segment_dx($i,$j) > 0 && $::segment_x($i,$j) > $::SCREEN_WIDTH} {
    set ::segment_x($i,$j) $::SCREEN_WIDTH

    turnSegment $i $j
  } elseif {$::segment_dx($i,$j) < 0 && $::segment_x($i,$j) < 0} {
    set ::segment_x($i,$j) 0

    turnSegment $i $j
  } else {
    set mushroom_obj [mushroomAt $::segment_x($i,$j) $::segment_y($i,$j)]

    if {$mushroom_obj != ""} {
      turnSegment $i $j
    }
  }

  if {$::segment_y($i,$j) > $::SCREEN_HEIGHT} {
    $::segment_obj($i,$j) set visible 0

    gameOver
  }

  $::segment_obj($i,$j) set center [list $::segment_x($i,$j) $::segment_y($i,$j)]
}

proc turnSegment { i j } {
  set ::segment_y($i,$j) [expr {$::segment_y($i,$j) + $::BLOCK_HEIGHT}]

  set ::segment_dx($i,$j) [expr {-$::segment_dx($i,$j)}]
}

proc moveCentipede { } {
  for {set i 0} {$i < $::centipede_ind} {incr i} {
    for {set j 0} {$j < $::MAX_SEGMENTS} {incr j} {
      moveSegment $i $j
    }

    incr ::centipede_tick

    if {$::centipede_tick > 7} {
      set ::centipede_tick 0
    }

    set iind $::centipede_iind

    if {$::centipede_tick > 3} {
      set ::centipede_iind 1
    } else {
      set ::centipede_iind 0
    }

    if {$iind != $::centipede_iind} {
      for {set j 0} {$j < $::MAX_SEGMENTS} {incr j} {
        $::segment_obj($i,$j) set image $::centipede_image($::centipede_iind)
      }
    }
  }
}

proc addMushroom { } {
  set obj [sb::image]

  $obj set image $::mushroom_image(0)
  
  return $obj
}

proc initMushrooms { } {
  if {! [info exists ::mushroom_obj(0,0)]} {
    set y 0

    for {set r 0} {$r < $::NUM_ROWS} {incr r} {
      set x 0

      for {set c 0} {$c < $::NUM_COLS} {incr c} {
        set ::mushroom_obj($r,$c) [addMushroom]

        $::mushroom_obj($r,$c) set visible 0

        $::mushroom_obj($r,$c) set meta [list $r $c]

        set ::mushroom_x($r,$c) $x
        set ::mushroom_y($r,$c) $y

        $::mushroom_obj($r,$c) set center [list $x $y]

        set ::mushroom_damage($r,$c) 0

        set x [expr {$x + $::BLOCK_WIDTH}]
      }

      set y [expr {$y + $::BLOCK_HEIGHT}]
    }
  }

  set im 0

  for {set r 0} {$r < $::NUM_ROWS} {incr r} {
    for {set c 0} {$c < $::NUM_COLS} {incr c} {
      if {$im < $::MAX_MUSHROOMS && [randIn 0.0 1.0] < 0.05} {
        $::mushroom_obj($r,$c) set visible 1
        incr im
      } else {
        $::mushroom_obj($r,$c) set visible 0
      }
    }
  }

  addMushroomsToQuad
} 

proc addMushroomsToQuad { } {
  $::mushroom_quad set reset 1

  for {set r 0} {$r < $::NUM_ROWS} {incr r} { 
    for {set c 0} {$c < $::NUM_COLS} {incr c} {
      if {[$::mushroom_obj($r,$c) get visible]} {
        $::mushroom_quad set object.add $::mushroom_obj($r,$c)
      }
    }
  }
}

proc hitMushroom { obj } {
  set pos [$obj get meta]
  set r   [lindex $pos 0]
  set c   [lindex $pos 1]

  incr :::mushroom_damage($r,$c)

  if {$:::mushroom_damage($r,$c) < 3} {
    $obj set image $::mushroom_image($::mushroom_damage($r,$c))
  } else {
    $obj set visible 0
  }
}

proc moveBullet { } {
  if {[$::bullet_obj get visible]} {
    set ::bullet_y [expr {$::bullet_y - $::BULLET_DY}]

    $::bullet_obj set center [list $::bullet_x $::bullet_y]

    if {$::bullet_y < 0} {
      $::bullet_obj set visible 0
      return
    }

    set mushroom_obj [mushroomAt $::bullet_x $::bullet_y]

    if {$mushroom_obj != ""} {
      hitMushroom $mushroom_obj

      $::bullet_obj set visible 0

      return
    }

    set segment_obj [segmentAt $::bullet_x $::bullet_y]

    if {$segment_obj != ""} {
      set meta [$segment_obj get meta]

      hitCentipede [lindex $meta 0] [lindex $meta 1]
    }
  }
}

proc loadImage { file } {
  set image [sb::image {0 0} $file]

  $image set scale [list $::game_scale $::game_scale]

  $image set visible 0 

  return $image
}

proc addTextLabel { str pos align } {
  set text [sb::text $pos $str]
  
  $text set pen.color white
  $text set align     $align
  
  return $text
}

proc gameOver { } {
  $::game_over_text set visible 1
  
  sb::canvas set play 0
}

proc init { } {
  set ::game_scale 2.5

  sb::canvas set brush.color black

  set ::MAX_MUSHROOMS      100
  set ::MAX_CENTIPEDES     12
  set ::NUM_ROWS           32
  set ::NUM_COLS           32
  set ::BLOCK_WIDTH        [expr {$::game_scale*16}]
  set ::BLOCK_HEIGHT       [expr {$::game_scale*24}]
  set ::SCREEN_WIDTH       [expr {$::BLOCK_WIDTH*$::NUM_COLS}]
  set ::SCREEN_HEIGHT      [expr {$::BLOCK_HEIGHT*$::NUM_ROWS}]
  set ::PLAYER_AREA_HEIGHT [expr {6*$::BLOCK_HEIGHT}]

  set ::INIT_LIVES 3

  set ::PLAYER_DX [expr {$::game_scale*4}]
  set ::PLAYER_DY [expr {$::game_scale*4}]

  set ::BULLET_DY [expr {$::game_scale*16}]

  set ::MAX_SEGMENTS 12

  set ::centipede_image(0) [loadImage "centipede/centipede1.gif"]
  set ::centipede_image(1) [loadImage "centipede/centipede2.gif"]

  set ::mushroom_image(0) [loadImage "centipede/mushroom_1.gif"]
  set ::mushroom_image(1) [loadImage "centipede/mushroom_2.gif"]
  set ::mushroom_image(2) [loadImage "centipede/mushroom_3.gif"]
  set ::mushroom_image(3) [loadImage "centipede/mushroom_4.gif"]

  set ::player_obj [loadImage "centipede/player.gif"]

  set ::bullet_obj [loadImage "centipede/bullet.gif"]

  #---

  sb::canvas set range [list 0 $::SCREEN_HEIGHT $::SCREEN_WIDTH 0]

  #---

  addPlayer

  set ::player_score 0
  set ::player_level 1

  #---

  set ::centipede_ind 0

  addCentipede

  #---

  set ::mushroom_quad [sb::quad_tree]

  initMushrooms

  #---

  set xm [expr {$::SCREEN_WIDTH/2}]
  set ym [expr {$::SCREEN_HEIGHT/2}]
  set xl 8
  set xr [expr {$::SCREEN_WIDTH - 8}]
  set yb [expr {$::SCREEN_HEIGHT - 8}]
  set yt 16

  set ::lives_text [addTextLabel "Lives: $::player_lives" [list $xl $yt] left  ]
  set ::score_text [addTextLabel "Score: $::player_score" [list $xm $yt] center]
  set ::level_text [addTextLabel "Level: $::player_level" [list $xr $yt] right ]

  set ::game_over_text [addTextLabel "GAME OVER" [list $xm $ym] center ]
  $::game_over_text set visible 0

  #---

  sb::canvas set play 1

  sb::canvas set window.size [list $::SCREEN_WIDTH $::SCREEN_HEIGHT]
}

if {0} {
proc mousePress { x y } {
  set objs [$::mushroom_quad get object.at_point [list $x $y]]

  if {[llength $objs] > 0} {
    echo $objs
  }
}
}

proc keyPress { args } {
  set key [lindex $args 0]

  if {$key == "p" || $key == "P"} {
    set play [sb::canvas get play]
    sb::canvas set play [expr {1 - $play}]
  }
}

proc update { } {
  if       {[sb::canvas get key "left"]} {
    playerLeft
  } elseif {[sb::canvas get key "right"]} {
    playerRight
  } elseif {[sb::canvas get key "up"]} {
    playerUp
  } elseif {[sb::canvas get key "down"]} {
    playerDown
  } elseif {[sb::canvas get key "space"]} {
    playerFire
  }

  moveBullet

  moveCentipede
}
