proc randIn { min max } {
  return [expr {rand()*($max - $min) + $min}]
}

proc irandIn { min max } {
  return [expr {int(rand()*($max - $min) + $min + 0.5)}]
}

proc loadModel { filename name { s 1.0 } } {
  # echo "$filename $name"

  set obj [sb3d::model $filename]

  $obj set id      $name
  $obj set visible 0

  if {$s != 1.0} {
    $obj set scale $s
  }

  return $obj
}

proc loadRefModel { name } {
  set obj [loadModel "${::model_dir}/${name}.obj" "${name}_ref"  ]
  return $obj
}

proc mapPos { pos } {
  set x [lindex $pos 0]
  set y [lindex $pos 1]
  set z [lindex $pos 2]

  set x1 [expr {$x*$::tileDx - $::mapDx}]
  set y1 [expr {$y*$::tileDy - $::mapDy}]
  set z1 [expr {$z*$::tileDz - $::mapDz}]

  return [list $x1 $y1 $z1]
}

proc addFloorTiles { } {
  # echo "addTiles $model"

  set ::tile_group [sb3d::group "tile_group"]

  for {set iy 0} {$iy < $::ny} {incr iy} {
    for {set ix 0} {$ix < $::nx} {incr ix} {
      set im [irandIn 0 3]

      set tile($ix,$iy) [$::floorRefObj($im) get ref_object]

      set pos [mapPos [list $ix 0 $iy]]

      $tile($ix,$iy) exec translate $pos

      $tile($ix,$iy) set visible 1

      # echo [$tile($ix,$iy) get bbox]

      $tile($ix,$iy) set group $::tile_group
    }
  }
}

proc addWalls { wallObj cornerObj } {
  # echo "addWalls $wallObj $cornerObj"

  set ix1 [expr {$::nx - 1}]
  set iy1 [expr {$::ny - 1}]

  for {set iy 0} {$iy < $::ny} {incr iy} {
    set tb [expr {$iy == 0 || $iy == $iy1}]

    for {set ix 0} {$ix < $::nx} {incr ix} {
      set lr [expr {$ix == 0 || $ix == $ix1}]
      if {! $lr && ! $tb} { continue }

      if {$::wall_map($ix,$iy) != ""} {
        set ::item_map($ix,$iy) $::wall_map($ix,$iy)
        $::search set empty 0 [list $ix $iy]
        continue
      }

      if {$lr && $tb} {
        set tile($ix,$iy) [$cornerObj get ref_object]

        if       {$ix == 0 && $iy == 0} {
          $tile($ix,$iy) exec rotate [list 0 1 0] 90
        } elseif {$ix == 0 && $iy == $iy1} {
          $tile($ix,$iy) exec rotate [list 0 1 0] 180
        } elseif {$ix == $ix1 && $iy == 0} {
          $tile($ix,$iy) exec rotate [list 0 1 0] 0
        } elseif {$ix == $ix1 && $iy == $iy1} {
          $tile($ix,$iy) exec rotate [list 0 1 0] 270
        }
      } else {
        set tile($ix,$iy) [$wallObj get ref_object]

        if {$lr} {
          $tile($ix,$iy) exec rotate [list 0 1 0] 90
        }
      }

      set pos [mapPos [list $ix 0 $iy]]

      $tile($ix,$iy) exec translate $pos

      $tile($ix,$iy) set visible 1

      # echo [$tile($ix,$iy) get bbox]

      set ::wall_map($ix,$iy) $tile($ix,$iy)
      set ::item_map($ix,$iy) $tile($ix,$iy)
      $::search set empty 0 [list $ix $iy]
    }
  }
}

proc addWall { model x y {a 0} } {
  set pos [list $x 0 $y]

  set obj [addObject $model $pos]

  if {$a != 0} {
    $obj exec rotate [list 0 1 0] $a
  }

  set ::wall_map($x,$y) $obj

  return $obj
}

proc addItem { model x y } {
  set pos [list $x 0 $y]

  set obj [addObject $model $pos]

  set ::item_map($x,$y) $obj
  $::search set empty 0 [list $x $y]

  return $obj
}

proc addObject { model { pos {0 0 0} } } {
  # echo "addObject $model $pos"

  set obj [$model get ref_object]

  set pos [mapPos $pos]

  $obj exec translate $pos

  $obj set visible 1

  # echo [$obj get bbox]

  return $obj
}

proc setModelDir { dir } {
  set ::model_dir $dir

  sb3d::canvas set model_dir $::model_dir
}

proc init { } {
  set ::cameraSet 0

  set ::tileDx 4.1
  set ::tileDy 0
  set ::tileDz 4.1

  setModelDir "tcl3d/Dungeon_Assets/obj"

  set ::floorRefObj(0) [loadRefModel "floor_wood_large"             ]
  set ::floorRefObj(1) [loadRefModel "floor_wood_large_dark"        ]
  set ::floorRefObj(2) [loadRefModel "floor_dirt_large"             ]
  set ::floorRefObj(3) [loadRefModel "floor_dirt_large_rocky"       ]
  set ::barrelRefObj   [loadRefModel "barrel_large_decorated"       ]
  set ::chest1RefObj   [loadRefModel "chest_mimic"                  ]
  set ::chest2RefObj   [loadRefModel "chest_mimic_lid"              ]
  set ::wallObj        [loadRefModel "wall"                         ]
# set ::wallPillarObj  [loadRefModel "wall_pillar"                  ]
  set ::wallCornerObj  [loadRefModel "wall_corner"                  ]
  set ::doorWallRefObj [loadRefModel "wall_doorway"                 ]
  set ::doorRefObj     [loadRefModel "wall_doorway_door"            ]
  set ::windowRefObj   [loadRefModel "wall_window_open"             ]
  set ::shelvesRefObj  [loadRefModel "wall_inset_shelves_decoratedA"]

  setModelDir "tcl3d/Dungeon_Characters/gltf"

  set ::playerRefObj [loadModel "$::model_dir/Barbarian.glb" "player_ref"]

  $::playerRefObj set child.visible "Barbarian_Hat"  0
  $::playerRefObj set child.visible "Mug"            0
  $::playerRefObj set child.visible "1H_Axe"         0
  $::playerRefObj set child.visible "1H_Axe_Offhand" 0

  setModelDir "tcl3d/Dungeon_Skeletons/gltf"

  set ::enemyRefObj [loadModel "$::model_dir/Skeleton_Warrior.glb" "enemy_ref"]

  setModelDir "tcl3d/Character_Animations/gltf/Rig_Medium"

  set ::enemyAnimObj [loadModel "$::model_dir/Rig_Medium_MovementBasic.glb" "enemy_anim"]
# set ::enemyAnimObj [loadModel "$::model_dir/Rig_Medium_General.glb" "enemy_anim"]

  $::enemyRefObj exec add_anim $::enemyAnimObj

  #---

  set ::nx 10
  set ::ny 10

  set ::search [sb3d::astar $::nx $::ny]

  for {set iy 0} {$iy < $::ny} {incr iy} {
    for {set ix 0} {$ix < $::nx} {incr ix} {
      set ::wall_map($ix,$iy) ""
    }
  }

  set ::mapDx [expr {$::nx*$::tileDx/2.0}]
  set ::mapDy 0
  set ::mapDz [expr {$::ny*$::tileDz/2.0}]

  addFloorTiles

  set ::doorWallObj     [addWall $::doorWallRefObj 4 9 180]
  set ::windowWallObj   [addWall $::windowRefObj   3 9 180]
  set ::shelvesWall1Obj [addWall $::shelvesRefObj  6 0   0]
  set ::shelvesWall2Obj [addWall $::shelvesRefObj  6 9 180]

  set ::doorObj [addObject $::doorRefObj [list 4 0 9]]

  addWalls $::wallObj $::wallCornerObj

  addItem $::barrelRefObj 4 4
  addItem $::chest1RefObj 5 5

  addObject $::chest2RefObj [list 5 1 5]

  set ::playerObj [addObject $::playerRefObj]
  #echo "$::playerObj [$::playerObj get transformed_model_bbox]"

  $::playerObj set anim.name "Idle"
  $::playerObj set anim.step 0.1

  $::playerObj set child.visible "Barbarian_Hat"  0
  $::playerObj set child.visible "Mug"            0
  $::playerObj set child.visible "1H_Axe"         0
  $::playerObj set child.visible "1H_Axe_Offhand" 0

  set ::enemyObj [addObject $::enemyRefObj]

# $::enemyObj set anim.name "Idle"
  $::enemyObj set anim.name "Walking_A"
  $::enemyObj set anim.step 0.1

  # setViewportValue "" bbox [list -10 -10 -10 10 10 10]

  sb3d::canvas set mode game

  #---

  set ::player_x   2
  set ::player_y   2
  set ::player_h   3
  set ::player_dir "N"

  set ::player_move  1.0
  set ::player_imove 0
  set ::player_nmove 250
  set ::player_dx    0
  set ::player_dy    0

  set ::player_rot  0
  set ::player_irot 0
  set ::player_nrot 250

  set ::player_ianim 0
  set ::player_nanim 300

  set ::player_moved 1

  #---

  set ::enemy_x   1
  set ::enemy_y   1 
  set ::enemy_h   3 
  set ::enemy_dir "N"

  set ::enemy_move  1.0
  set ::enemy_imove 0
  set ::enemy_nmove 500
  set ::enemy_dx    0
  set ::enemy_dy    0

  set ::enemy_rot   0
  set ::enemy_irot  0
  set ::enemy_nrot  500
  set ::enemy_angle 0

  set ::enemy_moved 1

  #---

  set ::camera_x 0
  set ::camera_y 0

  #---

  # updatePlayer

  sb3d::canvas set loop.enabled 1
  sb3d::canvas set loop.timeout 100

  sb3d::camera set disable_roll 1
}

proc tick { args } {
  set mode [sb3d::canvas get mode]

  if {$mode == "game"} {
    updatePlayer
  }
}

proc updatePlayer { } {
  if {$::player_moved} {
    # echo "Move Player"

    updatePlayerPos

    updateCamera

    set ::player_moved 0
  }

  if {$::enemy_moved} {
    # echo "Move Enemy"

    updateEnemyPos

    set ::enemy_moved 0
  }

  if       {$::player_irot >= 0} {
    # echo "Rotate Player"

    set yaw [sb3d::camera get yaw]

    set d [expr {$::player_rot/$::player_nrot}]

    sb3d::camera set yaw [expr {$yaw + $d}]
#   sb3d::camera set pitch -15

    incr ::player_irot -1

    updatePlayerPos

    updateCamera
  } elseif {$::player_imove >= 0} {
    incr ::player_imove -1

    set d [expr {$::player_move/$::player_nmove}]

    set ::player_x [expr {$::player_x + $::player_dx*$d}]
    set ::player_y [expr {$::player_y + $::player_dy*$d}]

    updatePlayerPos

    updateCamera

    if {$::player_imove < 0} {
      set ::player_x [expr {int($::player_x + 0.5)}]
      set ::player_y [expr {int($::player_y + 0.5)}]

      $::playerObj set anim.name "Idle"
    }
  }

  if {$::player_ianim >= 0} {
    incr ::player_ianim -1

    if {$::player_ianim < 0} {
      $::playerObj set anim.name "Idle"
    }
  }

  if       {$::enemy_irot >= 0} {
    # echo "Rotate Enemy"

    set d [expr {$::enemy_rot/$::enemy_nrot}]

    set ::enemy_angle [expr {$::enemy_angle + $d}]

    $::enemyObj exec rotate [list 0 1 0] $::enemy_angle

    incr ::enemy_irot -1
  
    updateEnemyPos
  } elseif {$::enemy_imove >= 0} {
    set d [expr {$::enemy_move/$::enemy_nmove}]

    set ::enemy_x [expr {$::enemy_x + $::enemy_dx*$d}]
    set ::enemy_y [expr {$::enemy_y + $::enemy_dy*$d}]

    incr ::enemy_imove -1

    updateEnemyPos

    if {$::enemy_imove < 0} {
      set ::enemy_x [expr {int($::enemy_x + 0.5)}]
      set ::enemy_y [expr {int($::enemy_y + 0.5)}]
    }
  }

  updateLight
}

proc updateEnemyPos { } {
  set pos [mapPos [list $::enemy_x 0 $::enemy_y]]

  $::enemyObj exec translate $pos

  if {$::enemy_irot < 0} {
    set a [dirToAngle $::enemy_dir]

    $::enemyObj exec rotate [list 0 1 0] [expr {90 - $a}]
  }

  if {$::enemy_imove >= 0 || $::enemy_irot >= 0} {
    return
  }

if {0} {
  set target [mapPos [list $::player_x 0 $::player_y]]

  set dx [expr {[lindex $target 0] - [lindex $pos 0]}]
  set dy [expr {[lindex $target 2] - [lindex $pos 2]}]
} else {
  set enemy_x1 [expr {int($::enemy_x + 0.5)}]
  set enemy_y1 [expr {int($::enemy_y + 0.5)}]

  set player_x1 [expr {int($::player_x + 0.5)}]
  set player_y1 [expr {int($::player_y + 0.5)}]

  set from [list $enemy_x1  $enemy_y1]
  set to   [list $player_x1 $player_y1]

  if {[lindex $from 0] != [lindex $to 0] || [lindex $from 1] != [lindex $to 1]} {
    set to1 [$::search get next $from $to]
    echo "$from -> $to : $to1"
  } else {
    set to1 $to
  }
}

  set dx [expr {[lindex $to1 0] - [lindex $from 0]}]
  set dy [expr {[lindex $to1 1] - [lindex $from 1]}]

  if {$dx > 0 || $dy > 0} {
    set dir $::enemy_dir

    if {abs($dx) > abs($dy)} {
      if {$dx < 0} {
        set ::enemy_dir "W"
      } else {
        set ::enemy_dir "E"
      }
    } else {
      if {$dy > 0} {
        set ::enemy_dir "S"
      } else {
        set ::enemy_dir "N"
      } 
    } 

    if {$dir != $::enemy_dir} {
      set ::enemy_irot  $::enemy_nrot
      set ::enemy_angle [dirToAngle $dir]

      if       {$dir == "N"} {
        if {$::enemy_dir == "E" || $::enemy_dir == "S"} {
          set ::enemy_rot  -90.0
        } else {
          set ::enemy_rot  90.0
        }
      } elseif {$dir == "S"} { 
        if {$::enemy_dir == "E" || $::enemy_dir == "N"} {
          set ::enemy_rot  -90.0
        } else {
          set ::enemy_rot  90.0
        }
      } elseif {$dir == "W"} {
        if {$::enemy_dir == "S" || $::enemy_dir == "E"} {
          set ::enemy_rot  -90.0
        } else {
          set ::enemy_rot  90.0
        }
      } elseif {$dir == "E"} {
        if {$::enemy_dir == "N" || $::enemy_dir == "W"} {
          set ::enemy_rot  -90.0
        } else {
          set ::enemy_rot  90.0
        }
      }
    } else {
      if {abs($dx) > abs($dy)} {
        if {$dx < 0} {
          set ::enemy_dx -1
          set ::enemy_dy 0
        } else {
          set ::enemy_dx 1
          set ::enemy_dy 0
        }
      } else {
        if {$dy > 0} {
          set ::enemy_dx 0
          set ::enemy_dy 1
        } else {
          set ::enemy_dx 0
          set ::enemy_dy -1
        }
      }

      set ::enemy_imove $::enemy_nmove
    }

    set ::enemy_moved 1
  } else {
    set ::enemy_dx 0
    set ::enemy_dy 0
  }
}

proc updatePlayerPos { } {
  set pos [mapPos [list $::player_x 0 $::player_y]]

  $::playerObj exec translate $pos

  set a [dirToAngle $::player_dir]

  $::playerObj exec rotate [list 0 1 0] [expr {90 - $a}]

  set ::enemy_moved 1
}

proc updateCamera { } {
if {0} {
  sb3d::camera set yaw $angle

  sb3d::camera set pitch 0
}

  set v [dirToVector $::player_dir]
  set vx [lindex $v 0]
  set vz [lindex $v 2]

  set pos [mapPos [list $::player_x 0 $::player_y]]

  set ::camera_x [expr {[lindex $pos 0] - 4*$vx}]
  set ::camera_y [expr {[lindex $pos 2] - 4*$vz}]

  sb3d::camera set position [list $::camera_x $::player_h $::camera_y]

# sb3d::camera set look_at [list 0 0 0]
}

proc updateLight { } {
  sb3d::light set current 1

  set v [dirToVector $::player_dir]
  set vx [lindex $v 0]
  set vz [lindex $v 2]

  set pos [mapPos [list $::player_x 0 $::player_y]]

  set ::light_x [expr {[lindex $pos 0] - 4*$vx}]
  set ::light_y [expr {[lindex $pos 2] - 4*$vz}]

  sb3d::light set position [list $::light_x $::player_h $::light_y]

  sb3d::light set direction [dirToVector $::player_dir]

  sb3d::light set point_radius 30
}

proc dirToAngle { dir } {
  if       {$dir == "N"} {
    return -90
  } elseif {$dir == "S"} {
    return 90
  } elseif {$dir == "W"} {
    return 180
  } elseif {$dir == "E"} {
    return 0
  } else {
    return 0
  }
}

proc dirToVector { dir } {
  if       {$dir == "N"} {
    return [list 0 0 -1]
  } elseif {$dir == "S"} {
    return [list 0 0 1]
  } elseif {$dir == "W"} {
    return [list -1 0 0]
  } elseif {$dir == "E"} {
    return [list 1 0 0]
  } else {
    return 0
  }
}

proc playerRotateLeft { } {
  # echo "turn left"

  if {$::player_irot <= 0} {
    set ::player_rot  -90.0
    set ::player_irot $::player_nrot

    if       {$::player_dir == "N"} {
      set ::player_dir "W"
    } elseif {$::player_dir == "W"} {
      set ::player_dir "S"
    } elseif {$::player_dir == "S"} {
      set ::player_dir "E"
    } elseif {$::player_dir == "E"} {
      set ::player_dir "N"
    }

    # echo "Dir: $::player_dir"
  }
}

proc playerRotateRight { } {
  # echo "turn right"

  if {$::player_irot <= 0} {
    set ::player_rot 90.0
    set ::player_irot $::player_nrot

    if       {$::player_dir == "N"} {
      set ::player_dir "E"
    } elseif {$::player_dir == "E"} {
      set ::player_dir "S"
    } elseif {$::player_dir == "S"} {
      set ::player_dir "W"
    } elseif {$::player_dir == "W"} {
      set ::player_dir "N"
    }

    # echo "Dir: $::player_dir"
  }
}

proc playerMoveForward { } {
  # echo "move forward"

  if {$::player_imove <= 0} {
    $::playerObj set anim.name "Walking_A"

    if       {$::player_dir == "N"} {
      set ::player_dx 0
      set ::player_dy -1
    } elseif {$::player_dir == "E"} {
      set ::player_dx 1
      set ::player_dy 0
    } elseif {$::player_dir == "S"} {
      set ::player_dx 0
      set ::player_dy 1
    } elseif {$::player_dir == "W"} {
      set ::player_dx -1
      set ::player_dy 0
    }

    if {! [playerCanMove]} {
      return
    }

    set ::player_imove $::player_nmove
  }
}

proc playerMoveBack { } {
  # echo "move back"

  if {$::player_imove <= 0} {
    $::playerObj set anim.name "Walking_A"

    if       {$::player_dir == "N"} {
      set ::player_dx 0
      set ::player_dy 1
    } elseif {$::player_dir == "E"} {
      set ::player_dx -1
      set ::player_dy 0
    } elseif {$::player_dir == "S"} {
      set ::player_dx 0
      set ::player_dy -1
    } elseif {$::player_dir == "W"} {
      set ::player_dx 1
      set ::player_dy 0
    }

    if {! [playerCanMove]} {
      return
    }

    set ::player_imove $::player_nmove
  }
}

proc playerStrafeLeft { } {
  # echo "strafe left"

  if {$::player_imove <= 0} {
    $::playerObj set anim.name "Walking_A"

    if       {$::player_dir == "N"} {
      set ::player_dx -1
      set ::player_dy 0
    } elseif {$::player_dir == "E"} {
      set ::player_dx 0
      set ::player_dy -1
    } elseif {$::player_dir == "S"} {
      set ::player_dx 1
      set ::player_dy 0
    } elseif {$::player_dir == "W"} {
      set ::player_dx 0
      set ::player_dy 1
    }

    if {! [playerCanMove]} {
      return
    }

    set ::player_imove $::player_nmove
  }
}

proc playerStrafeRight { } {
  # echo "strafe right"

  if {$::player_imove <= 0} {
    $::playerObj set anim.name "Walking_A"

    if       {$::player_dir == "N"} {
      set ::player_dx 1
      set ::player_dy 0
    } elseif {$::player_dir == "E"} {
      set ::player_dx 0
      set ::player_dy 1
    } elseif {$::player_dir == "S"} {
      set ::player_dx -1
      set ::player_dy 0
    } elseif {$::player_dir == "W"} {
      set ::player_dx 0
      set ::player_dy -1
    }

    if {! [playerCanMove]} {
      return
    }

    set ::player_imove $::player_nmove
  }
}

proc playerCanMove { } {
  set x [expr {int($::player_x + $::player_dx)}]
  set y [expr {int($::player_y + $::player_dy)}]

  if {[info exists ::item_map($x,$y)] && $::item_map($x,$y) != ""} {
    # echo "$::item_map($x,$y) $::player_x $::player_y $::player_dx $::player_dy $x $y"
    set ::player_dx 0
    set ::player_dy 0
    return 0
  }

  return 1
}

proc keyPress { k } {
  #puts "keyPress $k"

  if       {$k == "q" || $k == "Q" || $k == "left"} {
    playerRotateLeft
  } elseif {$k == "e" || $k == "E" || $k == "right"} {
    playerRotateRight
  } elseif {$k == "w" || $k == "W" || $k == "up"} {
    playerMoveForward
  } elseif {$k == "s" || $k == "S" || $k == "down"} {
    playerMoveBack
  } elseif {$k == "a" || $k == "A"} {
    playerStrafeLeft
  } elseif {$k == "d" || $k == "D"} {
    playerStrafeRight
  } elseif {$k == "<"} {
    set ::player_h [expr {$::player_h - 1}]

    set ::player_moved 1
  } elseif {$k == ">"} {
    set ::player_h [expr {$::player_h + 1}]

    set ::player_moved 1
  } elseif {$k == "\["} {
    set pitch [sb3d::camera get pitch]

    sb3d::camera set pitch [expr {$pitch + 1}]
  } elseif {$k == "]"} {
    set pitch [sb3d::camera get pitch]

    sb3d::camera set pitch [expr {$pitch - 1}]
  } elseif {$k == "l" || $k == "L"} {
    updateLight
  } elseif {$k == "1"} {
    $::playerObj exec rotate [list 0 1 0] 0
  } elseif {$k == "2"} {
    $::playerObj exec rotate [list 0 1 0] 90
  } elseif {$k == "3"} {
    $::playerObj exec rotate [list 0 1 0] 180
  } elseif {$k == "4"} {
    $::playerObj exec rotate [list 0 1 0] -90
  } elseif {$k == "space"} {
    $::playerObj set anim.name "1H_Melee_Attack_Chop"
    set ::player_ianim $::player_nanim
  }
}
