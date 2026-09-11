# TODO:
# . jump/climb
# . anim on move
# . hit (aabb)

proc randIn { min max } {
  return [expr {rand()*($max - $min) + $min}]
}

proc loadModel { filename name { s 1.0 } } {
  # echo "$filename $name"

  set obj [sb3d::model $filename]

  $obj set id      $name
  $obj set visible 0

  if {$s != 1.0} {
    $obj set scale $s
  }

  $obj set id $name

  return $obj
}

proc loadRefModel { name } {
  set obj [loadModel "${::model_dir}/${name}.obj" "${name}_ref"  ]
  return $obj
}

proc loadFloor { } {
  if {! [info exists ::floorRefObj]} {
    setModelDir "tcl3d/Dungeon_Assets/obj"

    set ::floorRefObj [loadRefModel "floor_wood_large"]
  }
}

proc loadBarrel { } {
  if {! [info exists ::barrelRefObj]} {
    setModelDir "tcl3d/Dungeon_Assets/obj"

    set ::barrelRefObj [loadRefModel "barrel_large"]
  }
}

proc addObject { model name { pos {0 0 0} } } {
  # echo "addObject $model $name $pos"

  set obj [$model get ref_object]
  $obj set visible 1

  $obj exec translate $pos

  $obj set id $name

  # echo [$obj get bbox]

  return $obj
}

proc loadPlayerObj { } {
  if {! [info exists ::playerRefObj]} {
    setModelDir "tcl3d/Dungeon_Characters/gltf"

    set ::playerRefObj [loadModel "$::model_dir/Barbarian.glb" "player_ref"]

    $::playerRefObj set child.visible "Barbarian_Hat"  0
    $::playerRefObj set child.visible "Mug"            0
    $::playerRefObj set child.visible "1H_Axe"         0
    $::playerRefObj set child.visible "1H_Axe_Offhand" 0
  }

  set obj [addObject $::playerRefObj player]
  #echo "$obj [$obj get transformed_model_bbox]"

  $obj set anim.name "Idle"
  $obj set anim.step 0.1

  $obj set child.visible "Barbarian_Hat"  0
  $obj set child.visible "Mug"            0
  $obj set child.visible "1H_Axe"         0
  $obj set child.visible "1H_Axe_Offhand" 0

  return $obj
}

proc setModelDir { dir } {
  set ::model_dir $dir

  sb3d::canvas set model_dir $::model_dir
}

proc init { } {
  loadFloor

  loadBarrel

  # ---

  set ::playerObj [loadPlayerObj]

  $::playerObj set angles [list 0 90 0]

  set ::player_iy 0
  set ::player_t  0
  set ::player_t1 0
  set ::player_dt 0.1

  set ::player_anim_dx [sb3d::anim_real 0]
  $::player_anim_dx set id "player_anim_dx"

  set ::player_anim_dy [sb3d::anim_real 0]
  $::player_anim_dy set id "player_anim_dy"

  # ---

  set ::barrelPath [sb3d::path]
  $::barrelPath set id "barrelPath"

  $::barrelPath set visible 0
  $::barrelPath set color   green

  set ::barrel_path_dy 0.1

  set dir 1

  set dx   4.0
  set dx1  [expr {$dx + 0.2}]
  set ::dy 4.1
  set dy1  0.4

  set z -8

  set ::nx 5
  set ::ny 5

  set w  [expr {$::nx*$dx1}]
  set w2 [expr {$w/2.0}]

  for {set iy 0} {$iy < $::ny} {incr iy} {
    set ::playerPath($iy) [sb3d::path]
    $::playerPath($iy) set id "playerPath.${iy}"

    $::playerPath($iy) set visible 0
    $::playerPath($iy) set color   yellow
  }

  set y 0

  for {set iy 0} {$iy < $::ny} {incr iy} {
    set x [expr {-$dir*$w2}]

    for {set ix 0} {$ix < $::nx} {incr ix} {
      set ::floor($ix,$iy) [$::floorRefObj get ref_object]

      $::floor($ix,$iy) set id "floor.${ix}.${iy}"

      set x1 [expr {$x + $dir*$dx/2}]

      $::floor($ix,$iy) set position [list $x1 $y $z]

      $::floor($ix,$iy) set visible 1

      set y1 [expr {$y + $::barrel_path_dy}]

      if {$ix == 0 && $iy == 0} {
        $::barrelPath exec moveTo [list $x1 $y1 $z]
      } else {
        $::barrelPath exec lineTo [list $x1 $y1 $z]
      }

      if {$ix == 0} {
        $::playerPath($iy) exec moveTo [list $x1 $y1 $z]
      } else {
        $::playerPath($iy) exec lineTo [list $x1 $y1 $z]
      }

      set ::playerDir($iy) $dir

      set x [expr {$x + $dir*$dx1}]
      set y [expr {$y + $dy1}]
    }

    set y [expr {$y + $::dy}]

    set dir [expr {-1*$dir}]
  }

  set ::nb 2

  for {set ib 0} {$ib < $::nb} {incr ib} {
    set ::barrel_obj($ib) [$::barrelRefObj get ref_object]

    $::barrel_obj($ib) set id "barrel.${ib}"

    $::barrel_obj($ib) set position [list 0 0 0]
    $::barrel_obj($ib) set angles   [list 90 0 0]

    $::barrel_obj($ib) set visible 1

    set ::barrel_t($ib) [randIn 0 0.2]
    set ::barrel_a($ib) [randIn 0 360]

    set ::barrel_pos($ib) {0 0 0}

    set ::barrel_dt($ib) [randIn 0.003 0.006]
    set ::barrel_da($ib) [randIn 0.2 0.7]
  }

  # ---

  applyPlayerPos

  # ---

  sb3d::canvas set camera.type first_person

  sb3d::camera set pitch    -90
  sb3d::camera set distance 18
  sb3d::camera set position {0 12 18}

  sb3d::canvas set mode game

  sb3d::canvas set loop.enabled 1
  sb3d::canvas set loop.timeout 30
}

proc bboxChanged { } {
  sb3d::light exec reset 1
}

proc tick { } {
  updateBarrelPos
  updatePlayerPos

  # ---

  for {set ib 0} {$ib < $::nb} {incr ib} {
    if {[$::barrel_obj($ib) exec intersect $::playerObj]} {
      echo "Hit Barrel"
    }
  }
}

proc updatePlayerPos { } {
  if {[$::player_anim_dx get can_step]} {
    $::player_anim_dx exec step

    if {! [$::player_anim_dx get can_step]} {
      set target [$::player_anim_dx get target]

      set ::player_t $::player_t1

      $::player_anim_dx exec reset
    }

    applyPlayerPos
  }

  if {[$::player_anim_dy get can_step]} {
    $::player_anim_dy exec step

    if {! [$::player_anim_dy get can_step]} {
      set target [$::player_anim_dy get target]

      if {$target > 0} {
        set ::player_iy [expr {$::player_iy + 1}]
      } else {
        set ::player_iy [expr {$::player_iy - 1}]
      }

      set ::player_t [expr {1.0 - $::player_t}]

      $::player_anim_dy exec reset
    }

    applyPlayerPos
  }
}

proc applyPlayerPos { } {
  # echo "IY: $::player_iy T: $::player_t"

  if {$::player_iy < 0 || $::player_iy >= $::ny} {
    return
  }

  if {$::player_t < 0 || $::player_t > 1} {
    return
  }

  set ::player_pos [$::playerPath($::player_iy) get tpos $::player_t]

  set x [lindex $::player_pos 0]
  set y [lindex $::player_pos 1]
  set z [lindex $::player_pos 2]

  if {[$::player_anim_dx get can_step]} {
    set dx [$::player_anim_dx get value]

    set ::player_pos [list [expr {$x + $dx}] $y $z]
  }

  if {[$::player_anim_dy get can_step]} {
    set dy [$::player_anim_dy get value]

    set ::player_pos [list $x [expr {$y + $dy}] $z]
  }

  # echo "Pos: $::player_pos"

  $::playerObj set position $::player_pos
}

proc updateBarrelPos { } {
  for {set ib 0} {$ib < $::nb} {incr ib} {
    if {$::barrel_t($ib) >   1.0} { set ::barrel_t($ib) 0.0 }
    if {$::barrel_a($ib) > 360.0} { set ::barrel_a($ib) 0.0 }

    set t1 [expr {1.0 - $::barrel_t($ib)}]

    set old_pos $::barrel_pos($ib)

    set ::barrel_pos($ib) [$::barrelPath get tpos $t1]

    set x [lindex $::barrel_pos($ib) 0]
    set y [lindex $::barrel_pos($ib) 1]
    set z [lindex $::barrel_pos($ib) 2]

    set y1 [expr {$y + 1.0}]

    $::barrel_obj($ib) set position [list $x $y1 $z]
    $::barrel_obj($ib) set angles   [list 90 $::barrel_a($ib) 0]

    set ::barrel_t($ib) [expr {$::barrel_t($ib) + $::barrel_dt($ib)}]

    if {[lindex $old_pos 0] > [lindex $::barrel_pos($ib) 0]} {
      set ::barrel_a($ib) [expr {$::barrel_a($ib) + $::barrel_da($ib)}]
    } else {
      set ::barrel_a($ib) [expr {$::barrel_a($ib) - $::barrel_da($ib)}]
    }
  }
}

proc keyPress { k } {
  if {[isAnimating]} {
    return
  }

  if       {$k == "left"} {
    $::playerObj set angles [list 0 -90 0]

    set ::player_t1 $::player_t

    if {$::playerDir($::player_iy) > 0} {
      if {$::player_t >= $::player_dt} {
        set ::player_t1 [expr {$::player_t - $::player_dt}]
      } else {
        set ::player_t1 0.0
      }
    } else {
      if {$::player_t <= 1.0 - $::player_dt} {
        set ::player_t1 [expr {$::player_t + $::player_dt}]
      } else {
        set ::player_t1 1.0
      }
    }

if {0} {
    set ::player_t $::player_t1
} else {
    set pos [$::playerPath($::player_iy) get tpos $::player_t1]

    set dx [expr {[lindex $pos 0] - [lindex $::player_pos 0]}]

    $::player_anim_dx set value  0
    $::player_anim_dx set target $dx
    $::player_anim_dx set steps  10
}

    applyPlayerPos
  } elseif {$k == "right"} {
    $::playerObj set angles [list 0 90 0]

    set ::player_t1 $::player_t

    if {$::playerDir($::player_iy) > 0} {
      if {$::player_t <= 1.0 - $::player_dt} {
        set ::player_t1 [expr {$::player_t + $::player_dt}]
      } else {
        set ::player_t1 1.0
      }
    } else {
      if {$::player_t >= $::player_dt} {
        set ::player_t1 [expr {$::player_t - $::player_dt}]
      } else {
        set ::player_t1 0.0
      }
    }

if {0} {
    set ::player_t $::player_t1
} else {
    set pos [$::playerPath($::player_iy) get tpos $::player_t1]

    set dx [expr {[lindex $pos 0] - [lindex $::player_pos 0]}]
  
    $::player_anim_dx set value  0
    $::player_anim_dx set target $dx
    $::player_anim_dx set steps  10
}

    applyPlayerPos
  } elseif {$k == "up"} {
    $::player_anim_dy set value  0
    $::player_anim_dy set target 2
    $::player_anim_dy set steps  10

    applyPlayerPos
  } elseif {$k == "down"} {
    $::player_anim_dy set value  0
    $::player_anim_dy set target -2
    $::player_anim_dy set steps  10

    applyPlayerPos
  }
}

proc isAnimating { } {
  if {[$::player_anim_dx get can_step]} {
    return 1
  }

  if {[$::player_anim_dy get can_step]} {
    return 1
  }

  return 0
}
