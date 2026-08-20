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

proc mapPos { pos } {
  set x [lindex $pos 0]
  set y [lindex $pos 1]
  set z [lindex $pos 2]

  set x1 [expr {$x*$::tileDx - $::mapDx}]
  set y1 [expr {$y*$::tileDy - $::mapDy}]
  set z1 [expr {$z*$::tileDz - $::mapDz}]

  return [list $x1 $y1 $z1]
}

proc addTiles { model } {
  # echo "addTiles $model"

  set ::tile_group [sb3d::group "tile_group"]

  for {set iy 0} {$iy < $::ny} {incr iy} {
    for {set ix 0} {$ix < $::nx} {incr ix} {
      set tile($ix,$iy) [$model get ref_object]

      set pos [mapPos [list $ix 0 $iy]]

    # $tile($ix,$iy) set position $pos
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

      # $tile($ix,$iy) set position $pos
      $tile($ix,$iy) exec translate $pos

      $tile($ix,$iy) set visible 1

      # echo [$tile($ix,$iy) get bbox]
    }
  }
}

proc addObject { model ind pos } {
  # echo "addObject $model $ind $pos"

  set ::obj($ind) [$model get ref_object]

  set pos [mapPos $pos]

  # $::obj($ind) set position $pos
  $::obj($ind) exec translate $pos

  $::obj($ind) set visible 1

  # echo [$::obj($ind) get bbox]

  return $::obj($ind)
}

proc init { } {
  set ::cameraSet 0

  set ::tileDx 4.1
  set ::tileDy 0
  set ::tileDz 4.1

  set model_dir "tcl3d/Dungeon_Assets/obj"

  sb3d::canvas set model_dir $model_dir

  if {0} {
  loadModel "$model_dir/floor_dirt_large.obj"                  "floor"]
  loadModel "$model_dir/floor_dirt_large_rocky.obj"            "floor"]
  loadModel "$model_dir/floor_tile_extralarge_grates.obj"      "floor"]
  loadModel "$model_dir/floor_tile_extralarge_grates_open.obj" "floor"]
  loadModel "$model_dir/floor_tile_large.obj"                  "floor"]
  loadModel "$model_dir/floor_tile_large_rocks.obj"            "floor"]
  loadModel "$model_dir/floor_wood_large_dark.obj"             "floor"]
  loadModel "$model_dir/floor_wood_large.obj"                  "floor"]
  }

  set ::tileObj       [loadModel "$model_dir/floor_wood_large.obj"       "tile"  ]
  set ::barrelObj     [loadModel "$model_dir/barrel_large_decorated.obj" "barrel"]
  set ::chest1Obj     [loadModel "$model_dir/chest_mimic.obj"            "chest1"]
  set ::chest2Obj     [loadModel "$model_dir/chest_mimic_lid.obj"        "chest2"]
  set ::wallObj       [loadModel "$model_dir/wall.obj"                   "wall"]
  set ::wallPillarObj [loadModel "$model_dir/wall_pillar.obj"            "wall_pillar"]
  set ::wallCornerObj [loadModel "$model_dir/wall_corner.obj"            "wall_corner"]

  set model_dir "tcl3d/Dungeon_Characters/gltf"

  sb3d::canvas set model_dir $model_dir

  set ::playerRefObj [loadModel "$model_dir/Barbarian.glb" "player"]
  # echo "$::playerRefObj [$::playerRefObj get transformed_model_bbox]"
  # $::playerRefObj set visible 1

  $::playerRefObj set child.visible "Barbarian_Hat"  0
  $::playerRefObj set child.visible "Mug"            0
  $::playerRefObj set child.visible "1H_Axe"         0
  $::playerRefObj set child.visible "1H_Axe_Offhand" 0

  set ::nx 10
  set ::ny 10

  set ::mapDx [expr {$::nx*$::tileDx/2.0}]
  set ::mapDy 0
  set ::mapDz [expr {$::ny*$::tileDz/2.0}]

  addTiles $::tileObj
  addWalls $::wallObj $::wallCornerObj

  set ::ind 0

  addObject $::barrelObj $::ind [list 4 0 4] ; incr ::ind

  addObject $::chest1Obj $::ind [list 5 0 5] ; incr ::ind
  addObject $::chest2Obj $::ind [list 5 1 5] ; incr ::ind

  set ::playerObj [addObject $::playerRefObj $::ind [list 0 0 0]] ; incr ::ind
  #echo "$::playerObj [$::playerObj get transformed_model_bbox]"

  # set ::playerObj $::playerRefObj
  # $::playerObj set visible 1

  $::playerObj set anim.name "Idle"
  $::playerObj set anim.step 0.1

  $::playerObj set child.visible "Barbarian_Hat"  0
  $::playerObj set child.visible "Mug"            0
  $::playerObj set child.visible "1H_Axe"         0
  $::playerObj set child.visible "1H_Axe_Offhand" 0

  # setViewportValue "" bbox [list -10 -10 -10 10 10 10]

  sb3d::canvas set mode game

  set ::player_x   0
  set ::player_y   0
  set ::player_h   3
  set ::player_dir "N"

  set ::player_move  4.0
  set ::player_imove 0
  set ::player_nmove 250
  set ::player_dx    0
  set ::player_dy    0

  set ::player_rot  0
  set ::player_irot 0
  set ::player_nrot 250

  set ::player_ianim 0
  set ::player_nanim 300

  set ::player_moved 0

  set ::camera_x 0
  set ::camera_y 0

  # updatePlayer

  sb3d::canvas set loop.enabled 1
  sb3d::canvas set loop.timeout 100

  sb3d::camera set disable_roll 1

  set ::player_moved 1
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

  if {$::player_imove >= 0} {
    incr ::player_imove -1

    set d [expr {$::player_move/$::player_nmove}]

    set ::player_x [expr {$::player_x + $::player_dx*$d}]
    set ::player_y [expr {$::player_y + $::player_dy*$d}]
    
    updatePlayerPos

    updateCamera

    if {$::player_imove < 0} {
      $::playerObj set anim.name "Idle"
    }
  }

  if {$::player_irot >= 0} {
    # echo "Rotate Player"

    set yaw [sb3d::camera get yaw]

    set d [expr {$::player_rot/$::player_nrot}]

    sb3d::camera set yaw [expr {$yaw + $d}]
#   sb3d::camera set pitch -15

    incr ::player_irot -1

    updateCamera
  }

  if {$::player_ianim >= 0} {
    incr ::player_ianim -1

    if {$::player_ianim < 0} {
      $::playerObj set anim.name "Idle"
    }
  }

  updateLight
}

proc updatePlayerPos { } {
  set pos [list $::player_x 0 $::player_y]

  $::playerObj exec translate $pos

  set a [dirToAngle $::player_dir]

  $::playerObj exec rotate [list 0 1 0] [expr {90 - $a}]
}

proc updateCamera { } {
if {0} {
  sb3d::camera set yaw $angle

  sb3d::camera set pitch 0
}

  set v [dirToVector $::player_dir]
  set vx [lindex $v 0]
  set vz [lindex $v 2]

  set ::camera_x [expr {$::player_x - 4*$vx}]
  set ::camera_y [expr {$::player_y - 4*$vz}]

  sb3d::camera set position [list $::camera_x $::player_h $::camera_y]

# sb3d::camera set look_at [list 0 0 0]
}

proc updateLight { } {
  sb3d::light set current 1

  set v [dirToVector $::player_dir]
  set vx [lindex $v 0]
  set vz [lindex $v 2]

  set ::camera_x [expr {$::player_x - 4*$vx}]
  set ::camera_y [expr {$::player_y - 4*$vz}]

  sb3d::light set position [list $::camera_x $::player_h $::player_y]

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

    set ::player_moved 1

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

    set ::player_moved 1

    # echo "Dir: $::player_dir"
  }
}

proc playerMoveForward { } {
  # echo "move forward"

if {0} {
  set d 0.1

  if       {$::player_dir == "N"} {
    set ::player_y [expr {$::player_y - $d}]
  } elseif {$::player_dir == "E"} {
    set ::player_x [expr {$::player_x + $d}]
  } elseif {$::player_dir == "S"} {
    set ::player_y [expr {$::player_y + $d}]
  } elseif {$::player_dir == "W"} {
    set ::player_x [expr {$::player_x - $d}]
  }

  set ::player_moved 1
} else {
  if {$::player_imove <= 0} {
    $::playerObj set anim.name "Walking_A"

    set ::player_imove $::player_nmove

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
  }
}
}

proc playerMoveBack { } {
  # echo "move back"

if {0} {
  set d 0.1

  if       {$::player_dir == "N"} {
    set ::player_y [expr {$::player_y + $d}]
  } elseif {$::player_dir == "E"} {
    set ::player_x [expr {$::player_x - $d}]
  } elseif {$::player_dir == "S"} {
    set ::player_y [expr {$::player_y - $d}]
  } elseif {$::player_dir == "W"} {
    set ::player_x [expr {$::player_x + $d}]
  }

  set ::player_moved 1
} else {
  if {$::player_imove <= 0} {
    $::playerObj set anim.name "Walking_A"

    set ::player_imove $::player_nmove

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
  } 
}
}

proc playerStrafeLeft { } {
  # echo "strafe left"

if {0} {
  set d 0.1

  if       {$::player_dir == "N"} {
    set ::player_x [expr {$::player_x - $d}]
  } elseif {$::player_dir == "E"} {
    set ::player_y [expr {$::player_y - $d}]
  } elseif {$::player_dir == "S"} {
    set ::player_x [expr {$::player_x + $d}]
  } elseif {$::player_dir == "W"} {
    set ::player_y [expr {$::player_y + $d}]
  }

  set ::player_moved 1
} else {
  if {$::player_imove <= 0} {
    $::playerObj set anim.name "Walking_A"
    
    set ::player_imove $::player_nmove
  
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
  }
}
}

proc playerStrafeRight { } {
  # echo "strafe right"

if {0} {
  set d 0.1

  if       {$::player_dir == "N"} {
    set ::player_x [expr {$::player_x + $d}]
  } elseif {$::player_dir == "E"} {
    set ::player_y [expr {$::player_y + $d}]
  } elseif {$::player_dir == "S"} {
    set ::player_x [expr {$::player_x - $d}]
  } elseif {$::player_dir == "W"} {
    set ::player_y [expr {$::player_y - $d}]
  }

  set ::player_moved 1
} else {
  if {$::player_imove <= 0} {
    $::playerObj set anim.name "Walking_A"
    
    set ::player_imove $::player_nmove

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
  }
}
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
