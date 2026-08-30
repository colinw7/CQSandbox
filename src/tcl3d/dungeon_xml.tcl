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

  if {! [info exists ::floorRefObj(0)]} {
    setModelDir "tcl3d/Dungeon_Assets/obj"

    set ::floorRefObj(0) [loadRefModel "floor_wood_large"             ]
    set ::floorRefObj(1) [loadRefModel "floor_wood_large_dark"        ]
    set ::floorRefObj(2) [loadRefModel "floor_dirt_large"             ]
    set ::floorRefObj(3) [loadRefModel "floor_dirt_large_rocky"       ]
  }

  set ::tile_group [sb3d::group "tile_group"]

  for {set iy 0} {$iy < $::ny} {incr iy} {
    for {set ix 0} {$ix < $::nx} {incr ix} {
      if {$::tile_map($ix,$iy) == ""} {
        continue
      }

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

proc adjustedWallPos { ix iy } {
  set map_type $::tile_map($ix,$iy)

  set ixx $ix
  if       {$map_type == "W" || $map_type == "NW" || $map_type == "SW" || $map_type == "DW"} {
    set ixx [expr {$ixx - 0.5}]
  } elseif {$map_type == "E" || $map_type == "NE" || $map_type == "SE" || $map_type == "DE"} {
    set ixx [expr {$ixx + 0.5}]
  }

  set iyy $iy
  if       {$map_type == "N" || $map_type == "NW" || $map_type == "NE" || $map_type == "DN"} {
    set iyy [expr {$iyy - 0.5}]
  } elseif {$map_type == "S" || $map_type == "SW" || $map_type == "SE" || $map_type == "DS"} {
    set iyy [expr {$iyy + 0.5}]
  }

  return [list $ixx 0 $iyy]
}

proc addWalls { } {
  # echo "addWalls"

  if {! [info exists ::wallRefObj]} {
    setModelDir "tcl3d/Dungeon_Assets/obj"

    set ::wallRefObj       [loadRefModel "wall"                         ]
    set ::wallHalfRefObj   [loadRefModel "wall_half"                    ]
    set ::wallCornerRefObj [loadRefModel "wall_corner"                  ]
    set ::doorWallRefObj   [loadRefModel "wall_doorway"                 ]
  # set ::doorRefObj       [loadRefModel "wall_doorway_door"            ]
  # set ::wallPillarObj    [loadRefModel "wall_pillar"                  ]
  }

  for {set iy 0} {$iy < $::ny} {incr iy} {
    for {set ix 0} {$ix < $::nx} {incr ix} {
      set map_type $::tile_map($ix,$iy)

      if {$map_type == "" || $map_type == "F" || $map_type == "C"} {
        continue
      }

      if {$::wall_map($ix,$iy) != ""} {
        #set ::item_map($ix,$iy) $::wall_map($ix,$iy)
        #$::search set empty 0 [list $ix $iy]
        continue
      }

      set apos [adjustedWallPos $ix $iy]

      set pos [mapPos $apos]

      if {$map_type == "NW" || $map_type == "NE" || $map_type == "SW" || $map_type == "SE"} {
        # corner

        set tile($ix,$iy) [$::wallCornerRefObj get ref_object]

        if       {$map_type == "NW"} {
          $tile($ix,$iy) exec rotate [list 0 1 0] 90
        } elseif {$map_type == "SW"} {
          $tile($ix,$iy) exec rotate [list 0 1 0] 180
        } elseif {$map_type == "NE"} {
          $tile($ix,$iy) exec rotate [list 0 1 0] 0
        } elseif {$map_type == "SE"} {
          $tile($ix,$iy) exec rotate [list 0 1 0] 270
        }

        set obj1 [$::wallHalfRefObj get ref_object]
        set obj2 [$::wallHalfRefObj get ref_object]

        $obj2 exec rotate [list 0 1 0] 90

        set x [lindex $pos 0]
        set y [lindex $pos 2]

        if {$map_type == "NW" || $map_type == "SW"} {
          set pos1 [list [expr {$x + $::tileDx/2}] 0 $y]
        } else {
          set pos1 [list [expr {$x - $::tileDx}] 0 $y]
        }

        if {$map_type == "NW" || $map_type == "NE"} {
          set pos2 [list $x 0 [expr {$y + $::tileDz}]]
        } else {
          set pos2 [list $x 0 [expr {$y - $::tileDz/2}]]
        }

        $obj1 exec translate $pos1
        $obj2 exec translate $pos2
      } elseif {$map_type == "N" || $map_type == "S" ||
                $map_type == "W" || $map_type == "E"} {
        # wall

        set tile($ix,$iy) [$::wallRefObj get ref_object]

        if {$map_type == "W" || $map_type == "E"} {
          $tile($ix,$iy) exec rotate [list 0 1 0] 90
        }
      } elseif {$map_type == "DN" || $map_type == "DS" ||
                $map_type == "DW" || $map_type == "DE"} {
        # door

        set tile($ix,$iy) [$::doorWallRefObj get ref_object]

        if {$map_type == "DW" || $map_type == "DE"} {
          $tile($ix,$iy) exec rotate [list 0 1 0] 90
        }
      } else {
        echo "Invalid map type $map_type"
      }

      $tile($ix,$iy) exec translate $pos

      # echo [$tile($ix,$iy) get bbox]

      set ::wall_map($ix,$iy) $tile($ix,$iy)

      #set ::item_map($ix,$iy) $tile($ix,$iy)
      #$::search set empty 0 [list $ix $iy]
    }
  }
}

proc addWall { model ix iy {a 0} } {
  set pos [adjustedWallPos $ix $iy]

  set obj [addObject $model $pos]

  if {$a != 0} {
    $obj exec rotate [list 0 1 0] $a
  }

  set ::wall_map($ix,$iy) $obj

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
  $obj set visible 1

  set pos [mapPos $pos]

  $obj exec translate $pos

  # echo [$obj get bbox]

  return $obj
}

proc setModelDir { dir } {
  set ::model_dir $dir

  sb3d::canvas set model_dir $::model_dir
}

proc init { } {
  loadXml

  set ::cameraSet 0

  set ::tileDx 4.1
  set ::tileDy 0
  set ::tileDz 4.1

if {0} {
  setModelDir "tcl3d/Dungeon_Assets/obj"

  set ::barrelRefObj [loadRefModel "barrel_large_decorated"       ]
  set ::chest1RefObj [loadRefModel "chest_mimic"                  ]
  set ::chest2RefObj [loadRefModel "chest_mimic_lid"              ]
}

if {0} {
  setModelDir "tcl3d/Dungeon_Assets/obj"

  set ::windowRefObj  [loadRefModel "wall_window_open"             ]
  set ::shelvesRefObj [loadRefModel "wall_inset_shelves_decoratedA"]
}

  #---

  set ::search [sb3d::astar $::nx $::ny]

  for {set iy 0} {$iy < $::ny} {incr iy} {
    for {set ix 0} {$ix < $::nx} {incr ix} {
      set ::wall_map($ix,$iy) ""
    }
  }

  set ::mapDx [expr {$::nx*$::tileDx/2.0}]
  set ::mapDy 0
  set ::mapDz [expr {$::ny*$::tileDz/2.0}]

  #---

  addFloorTiles

  #---

if {0} {
  set ::windowWallObj   [addWall $::windowRefObj  3 9 180]
  set ::shelvesWall1Obj [addWall $::shelvesRefObj 6 0   0]
  set ::shelvesWall2Obj [addWall $::shelvesRefObj 6 9 180]
}

  addWalls

  #---

if {0} {
  addItem $::barrelRefObj 4 4
  addItem $::chest1RefObj 5 5

  addObject $::chest2RefObj [list 5 1 5]
}

  #---

  set ::playerObj [createPlayerObj]

  set ::enemyObj [createEnemyObj]

  #---

  sb3d::canvas set mode game

  #---

  set ::player_x   0
  set ::player_y   0
  set ::player_h   3
  set ::player_dir "N"

  while {1} {
    set ix [irandIn 0 [expr {$::nx - 1}]]
    set iy [irandIn 0 [expr {$::ny - 1}]]

    if {$::tile_map($ix,$iy) == "F"} {
      set ::player_x $ix
      set ::player_y $iy

      break
    }
  }

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

  set ::enemy_chase 0

  #---

  set ::current_model "player"

  #---

  set ::camera_x 0
  set ::camera_y 0

  #---

  # updatePlayer

  sb3d::canvas set loop.enabled 1
  sb3d::canvas set loop.timeout 100

  sb3d::camera set disable_roll 1

  #sb3d::canvas set camera.type first_person
}

proc createPlayerObj { } {
  if {! [info exists ::playerRefObj]} {
    setModelDir "tcl3d/Dungeon_Characters/gltf"

    set ::playerRefObj [loadModel "$::model_dir/Barbarian.glb" "player_ref"]

    $::playerRefObj set child.visible "Barbarian_Hat"  0
    $::playerRefObj set child.visible "Mug"            0
    $::playerRefObj set child.visible "1H_Axe"         0
    $::playerRefObj set child.visible "1H_Axe_Offhand" 0
  }

  set obj [addObject $::playerRefObj]
  #echo "$obj [$obj get transformed_model_bbox]"

  $obj set anim.name "Idle"
  $obj set anim.step 0.1

  $obj set child.visible "Barbarian_Hat"  0
  $obj set child.visible "Mug"            0
  $obj set child.visible "1H_Axe"         0
  $obj set child.visible "1H_Axe_Offhand" 0

  return $obj
}

proc createEnemyObj { } {
  if {! [info exists ::enemyRefObj]} {
    setModelDir "tcl3d/Dungeon_Skeletons/gltf"

    set ::enemyRefObj [loadModel "$::model_dir/Skeleton_Warrior.glb" "enemy_ref"]

    setModelDir "tcl3d/Character_Animations/gltf/Rig_Medium"

    set ::enemyAnim1Obj [loadModel "$::model_dir/Rig_Medium_General.glb" "enemy_anim"]
    set ::enemyAnim2Obj [loadModel "$::model_dir/Rig_Medium_MovementBasic.glb" "enemy_anim"]

    $::enemyRefObj exec add_anim $::enemyAnim1Obj
    $::enemyRefObj exec add_anim $::enemyAnim2Obj
  }

  set obj [addObject $::enemyRefObj]

  $obj set anim.name "Idle_A"
  $obj set anim.step 0.1

  return $obj
}

proc loadXml { } {
  set ::xml [sb3d::xml "tcl3d/dungeon.xml"]

  if {! [$::xml exec load]} {
    echo "Xml load failed"
    exit 1
  }

  set options [$::xml get tag_options]

  foreach option $options {
    set name  [lindex $option 0]
    set value [lindex $option 1]

    if {$name == "size"} {
      set ::nx [lindex $value 0]
      set ::ny [lindex $value 1]
    } else {
      echo " Invalid name $name"
      exit 1
    }
  }

  for {set iy 0} {$iy < $::ny} {incr iy} {
    for {set ix 0} {$ix < $::nx} {incr ix} {
      set ::tile_map($ix,$iy) ""
    }
  }

  set tags [$::xml get tag_inds]

  foreach tag $tags {
    set name [$::xml get name $tag]

    if {$name == "room"} {
      set options [$::xml get tag_options $tag]

      set x 0
      set y 0
      set w 1
      set h 1

      foreach option $options {
        set name  [lindex $option 0]
        set value [lindex $option 1]

        if       {$name == "pos"} {
          set x [lindex $value 0]
          set y [lindex $value 1]
        } elseif {$name == "size"} {
          set w [lindex $value 0]
          set h [lindex $value 1]
        } elseif {$name == "ind"} {
        } else {
          echo " Invalid room name $name"
          exit 1
        }
      }

      set w1 [expr {$w/2}]
      set h1 [expr {$h/2}]

      set ix1 [expr {int($x - $w1)}]
      set iy1 [expr {int($y - $h1)}]
      set ix2 [expr {int($x + $w1)}]
      set iy2 [expr {int($y + $h1)}]

      echo "$x $y $w $h : $ix1 $iy1 $ix2 $iy2"

      for {set iy $iy1} {$iy <= $iy2} {incr iy} {
        for {set ix $ix1} {$ix <= $ix2} {incr ix} {
          if     {$iy == $iy1} {
            if       {$ix == $ix1} {
              set ::tile_map($ix,$iy) "NW"
            } elseif {$ix == $ix2} {
              set ::tile_map($ix,$iy) "NE"
            } else {
              set ::tile_map($ix,$iy) "N"
            }
          } elseif {$iy == $iy2} {
            if       {$ix == $ix1} {
              set ::tile_map($ix,$iy) "SW"
            } elseif {$ix == $ix2} {
              set ::tile_map($ix,$iy) "SE"
            } else {
              set ::tile_map($ix,$iy) "S"
            }
          } else {
            if       {$ix == $ix1} {
              set ::tile_map($ix,$iy) "W"
            } elseif {$ix == $ix2} {
              set ::tile_map($ix,$iy) "E"
            } else {
              set ::tile_map($ix,$iy) "F"
            }
          }
        }
      }
    } elseif {$name == "cell"} {
      set options [$::xml get tag_options $tag]

      set x 0
      set y 0

      foreach option $options {
        set name  [lindex $option 0]
        set value [lindex $option 1]

        if       {$name == "pos"} {
          set x [lindex $value 0]
          set y [lindex $value 1]
        } elseif {$name == "type"} {
        } else {
          echo " Invalid room name $name"
          exit 1
        }
      }

      set ::tile_map($x,$y) "C"
    } else {
      echo " Invalid tag $name"
      exit 1
    }
  }

  for {set iy 0} {$iy < $::ny} {incr iy} {
    for {set ix 0} {$ix < $::nx} {incr ix} {
      if {$::tile_map($ix,$iy) == "N" ||
          $::tile_map($ix,$iy) == "S" ||
          $::tile_map($ix,$iy) == "W" ||
          $::tile_map($ix,$iy) == "E"} {
        set ix1 [expr {$ix - 1}]
        set ix2 [expr {$ix + 1}]
        set iy1 [expr {$iy - 1}]
        set iy2 [expr {$iy + 1}]

        if {$::tile_map($ix1,$iy) == "C" ||
            $::tile_map($ix2,$iy) == "C" ||
            $::tile_map($ix,$iy1) == "C" ||
            $::tile_map($ix,$iy2) == "C"} {
          set ::tile_map($ix,$iy) "D$::tile_map($ix,$iy)"
        }
      }
    }
  }
}

proc tick { args } {
  set mode [sb3d::canvas get mode]

  if {$mode == "game"} {
    updatePlayer
  }
}

proc rotatePlayer { } {
  # echo "Rotate Player"

  set yaw [sb3d::camera get yaw]

  set d [expr {$::player_rot/$::player_nrot}]

  sb3d::camera set yaw [expr {$yaw + $d}]
# sb3d::camera set pitch -15

  incr ::player_irot -1

  updatePlayerPos

  updateCamera
}

proc movePlayer { } {
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

proc rotateEnemy { } {
  # echo "Rotate Enemy"

  set d [expr {$::enemy_rot/$::enemy_nrot}]

  set ::enemy_angle [expr {$::enemy_angle + $d}]

  $::enemyObj exec rotate [list 0 1 0] [expr {90 - $::enemy_angle}]

  incr ::enemy_irot -1

  updateEnemyPos
}

proc moveEnemy { } {
  set d [expr {$::enemy_move/$::enemy_nmove}]

  set ::enemy_x [expr {$::enemy_x + $::enemy_dx*$d}]
  set ::enemy_y [expr {$::enemy_y + $::enemy_dy*$d}]

  incr ::enemy_imove -1

  updateEnemyPos

  if {$::enemy_imove < 0} {
    set ::enemy_x [expr {int($::enemy_x + 0.5)}]
    set ::enemy_y [expr {int($::enemy_y + 0.5)}]

    $::enemyObj set anim.name "Idle_A"
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
    rotatePlayer
  } elseif {$::player_imove >= 0} {
    movePlayer
  }

  if {$::player_ianim >= 0} {
    incr ::player_ianim -1

    if {$::player_ianim < 0} {
      $::playerObj set anim.name "Idle"
    }
  }

  if       {$::enemy_irot >= 0} {
    rotateEnemy
  } elseif {$::enemy_imove >= 0} {
    moveEnemy
  }

  updateLight
}

proc chasePlayer { } {
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
    # echo "$from -> $to : $to1"
  } else {
    set to1 $to
  }
}

  set dx [expr {[lindex $to1 0] - [lindex $from 0]}]
  set dy [expr {[lindex $to1 1] - [lindex $from 1]}]

  if {abs($dx) > 0 || abs($dy) > 0} {
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

      $::enemyObj set anim.name "Walking_A"
    }

    set ::enemy_moved 1
  } else {
    set ::enemy_dx 0
    set ::enemy_dy 0
  }
}

proc updateEnemyPos { } {
  set pos [mapPos [list $::enemy_x 0 $::enemy_y]]

  $::enemyObj exec translate $pos

  if {$::enemy_irot < 0} {
    set a [dirToAngle $::enemy_dir]

    $::enemyObj exec rotate [list 0 1 0] [expr {90 - $a}]
  }

  if {$::enemy_imove < 0 && $::enemy_irot < 0} {
    if {$::current_model == "player"} {
      if {$::enemy_chase} {
        chasePlayer
      }
    }
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

  set map_type $::tile_map($x,$y)
  if {$map_type == ""} {
    return 0
  }

  return 1
}

proc enemyRotateLeft { } {
  # echo "turn left"

  if {$::enemy_irot <= 0} {
    set ::enemy_rot  -90.0
    set ::enemy_irot $::enemy_nrot
    set ::enemy_angle [dirToAngle $::enemy_dir]

    if       {$::enemy_dir == "N"} {
      set ::enemy_dir "W"
    } elseif {$::enemy_dir == "W"} {
      set ::enemy_dir "S"
    } elseif {$::enemy_dir == "S"} {
      set ::enemy_dir "E"
    } elseif {$::enemy_dir == "E"} {
      set ::enemy_dir "N"
    }

    echo "Dir: $::enemy_dir"
  }
}

proc enemyRotateRight { } {
  # echo "turn right"

  if {$::enemy_irot <= 0} {
    set ::enemy_rot 90.0
    set ::enemy_irot $::enemy_nrot
    set ::enemy_angle [dirToAngle $::enemy_dir]

    if       {$::enemy_dir == "N"} {
      set ::enemy_dir "E"
    } elseif {$::enemy_dir == "E"} {
      set ::enemy_dir "S"
    } elseif {$::enemy_dir == "S"} {
      set ::enemy_dir "W"
    } elseif {$::enemy_dir == "W"} {
      set ::enemy_dir "N"
    }

    echo "Dir: $::enemy_dir"
  }
}

proc enemyMoveForward { } {
  # echo "move forward"

  if {$::enemy_imove <= 0} {
    $::enemyObj set anim.name "Walking_A"

    if       {$::enemy_dir == "N"} {
      set ::enemy_dx 0
      set ::enemy_dy -1
    } elseif {$::enemy_dir == "E"} {
      set ::enemy_dx 1
      set ::enemy_dy 0
    } elseif {$::enemy_dir == "S"} {
      set ::enemy_dx 0
      set ::enemy_dy 1
    } elseif {$::enemy_dir == "W"} {
      set ::enemy_dx -1
      set ::enemy_dy 0
    }

    if {! [enemyCanMove]} {
      return
    }

    set ::enemy_imove $::enemy_nmove
  }
}

proc enemyMoveBack { } {
  # echo "move back"

  if {$::enemy_imove <= 0} {
    $::enemyObj set anim.name "Walking_A"

    if       {$::enemy_dir == "N"} {
      set ::enemy_dx 0
      set ::enemy_dy 1
    } elseif {$::enemy_dir == "E"} {
      set ::enemy_dx -1
      set ::enemy_dy 0
    } elseif {$::enemy_dir == "S"} {
      set ::enemy_dx 0
      set ::enemy_dy -1
    } elseif {$::enemy_dir == "W"} {
      set ::enemy_dx 1
      set ::enemy_dy 0
    }

    if {! [enemyCanMove]} {
      return
    }

    set ::enemy_imove $::enemy_nmove
  }
}

proc enemyStrafeLeft { } {
  # echo "strafe left"

  if {$::enemy_imove <= 0} {
    $::enemyObj set anim.name "Walking_A"

    if       {$::enemy_dir == "N"} {
      set ::enemy_dx -1
      set ::enemy_dy 0
    } elseif {$::enemy_dir == "E"} {
      set ::enemy_dx 0
      set ::enemy_dy -1
    } elseif {$::enemy_dir == "S"} {
      set ::enemy_dx 1
      set ::enemy_dy 0
    } elseif {$::enemy_dir == "W"} {
      set ::enemy_dx 0
      set ::enemy_dy 1
    }

    if {! [enemyCanMove]} {
      return
    }

    set ::enemy_imove $::enemy_nmove
  }
}

proc enemyStrafeRight { } {
  # echo "strafe right"

  if {$::enemy_imove <= 0} {
    $::enemyObj set anim.name "Walking_A"

    if       {$::enemy_dir == "N"} {
      set ::enemy_dx 1
      set ::enemy_dy 0
    } elseif {$::enemy_dir == "E"} {
      set ::enemy_dx 0
      set ::enemy_dy 1
    } elseif {$::enemy_dir == "S"} {
      set ::enemy_dx -1
      set ::enemy_dy 0
    } elseif {$::enemy_dir == "W"} {
      set ::enemy_dx 0
      set ::enemy_dy -1
    }

    if {! [enemyCanMove]} {
      return
    }

    set ::enemy_imove $::enemy_nmove
  }
}
proc enemyCanMove { } {
  set x [expr {int($::enemy_x + $::enemy_dx)}]
  set y [expr {int($::enemy_y + $::enemy_dy)}]

  if {[info exists ::item_map($x,$y)] && $::item_map($x,$y) != ""} {
    # echo "$::item_map($x,$y) $::enemy_x $::enemy_y $::enemy_dx $::enemy_dy $x $y"
    set ::enemy_dx 0
    set ::enemy_dy 0
    return 0
  }

  set map_type $::tile_map($x,$y)
  if {$map_type == ""} {
    return 0
  }

  return 1
}

proc keyPress { k } {
  # puts "keyPress $k"

  if       {$k == "q" || $k == "Q" || $k == "left"} {
    if {$::current_model == "player"} {
      playerRotateLeft
    } else {
      enemyRotateLeft
    }
  } elseif {$k == "e" || $k == "E" || $k == "right"} {
    if {$::current_model == "player"} {
      playerRotateRight
    } else {
      enemyRotateRight
    }
  } elseif {$k == "w" || $k == "W" || $k == "up"} {
    if {$::current_model == "player"} {
      playerMoveForward
    } else {
      enemyMoveForward
    }
  } elseif {$k == "s" || $k == "S" || $k == "down"} {
    if {$::current_model == "player"} {
      playerMoveBack
    } else {
      enemyMoveBack
    }
  } elseif {$k == "a" || $k == "A"} {
    if {$::current_model == "player"} {
      playerStrafeLeft
    } else {
      enemyStrafeLeft
    }
  } elseif {$k == "d" || $k == "D"} {
    if {$::current_model == "player"} {
      playerStrafeRight
    } else {
      enemyStrafeRight
    }
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
  } elseif {$k == "c" || $k == "C"} {
    set ::enemy_chase [expr {1 - $::enemy_chase}]
    set ::enemy_moved 1
  } elseif {$k == "backspace"} {
    if {$::current_model == "player"} {
      set ::current_model "enemy"
    } else {
      set ::current_model "player"
    }

    echo "Current: $::current_model"
  }
}
