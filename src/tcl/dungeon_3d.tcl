proc loadModel { filename name { s 1.0 } } {
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
  echo "addTiles $model"

  for {set iy 0} {$iy < $::ny} {incr iy} {
    for {set ix 0} {$ix < $::nx} {incr ix} {
      set tile($ix,$iy) [$model get ref_object]

      set pos [mapPos [list $ix 0 $iy]]

      # $tile($ix,$iy) set position $pos
      $tile($ix,$iy) exec translate $pos

      $tile($ix,$iy) set visible 1

      # echo [$tile($ix,$iy) get bbox]
    }
  }
}

proc addWalls { wallObj cornerObj } {
  echo "addWalls $wallObj $cornerObj"

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
  set ::obj($ind) [$model get ref_object]

  set pos [mapPos $pos]

  # $::obj($ind) set position $pos
  $::obj($ind) exec translate $pos

  $::obj($ind) set visible 1

  # echo [$::obj($ind) get bbox]
}

proc init { } {
  set ::cameraSet 0

  set ::tileDx 4.1
  set ::tileDy 0
  set ::tileDz 4.1

  sb3d::canvas set model_dir "tcl/Dungeon_Assets/obj"

  if {0} {
  loadModel "tcl/Dungeon_Assets/obj/floor_dirt_large.obj"                  "floor"]
  loadModel "tcl/Dungeon_Assets/obj/floor_dirt_large_rocky.obj"            "floor"]
  loadModel "tcl/Dungeon_Assets/obj/floor_tile_extralarge_grates.obj"      "floor"]
  loadModel "tcl/Dungeon_Assets/obj/floor_tile_extralarge_grates_open.obj" "floor"]
  loadModel "tcl/Dungeon_Assets/obj/floor_tile_large.obj"                  "floor"]
  loadModel "tcl/Dungeon_Assets/obj/floor_tile_large_rocks.obj"            "floor"]
  loadModel "tcl/Dungeon_Assets/obj/floor_wood_large_dark.obj"             "floor"]
  loadModel "tcl/Dungeon_Assets/obj/floor_wood_large.obj"                  "floor"]
  }

  set ::tileObj       [loadModel "tcl/Dungeon_Assets/obj/floor_wood_large.obj"       "tile"  ]
  set ::barrelObj     [loadModel "tcl/Dungeon_Assets/obj/barrel_large_decorated.obj" "barrel"]
  set ::chest1Obj     [loadModel "tcl/Dungeon_Assets/obj/chest_mimic.obj"            "chest1"]
  set ::chest2Obj     [loadModel "tcl/Dungeon_Assets/obj/chest_mimic_lid.obj"        "chest2"]
  set ::wallObj       [loadModel "tcl/Dungeon_Assets/obj/wall.obj"                   "wall"]
  set ::wallPillarObj [loadModel "tcl/Dungeon_Assets/obj/wall_pillar.obj"            "wall_pillar"]
  set ::wallCornerObj [loadModel "tcl/Dungeon_Assets/obj/wall_corner.obj"            "wall_corner"]

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

  # setViewportValue "" bbox [list -10 -10 -10 10 10 10]

  sb3d::canvas set mode game

  set ::player_x   0
  set ::player_y   0
  set ::player_h   1
  set ::player_dir "N"

  updatePlayerCamera
}

proc tick { args } {
  if {! $::cameraSet} {
    set ::cameraSet 1

    # sb3d::camera set camera.distance 20

    # sb3d::camera set origin [list 0 0 0]

    sb3d::camera set position [list 0 10 20]
  }
}

proc updatePlayerCamera { } {
  sb3d::camera set position [list $::player_x $::player_h $::player_y]

  if       {$::player_dir == "N"} {
    sb3d::camera set yaw 90
  } elseif {$::player_dir == "S"} {
    sb3d::camera set yaw -90
  } elseif {$::player_dir == "W"} {
    sb3d::camera set yaw 180
  } elseif {$::player_dir == "E"} {
    sb3d::camera set yaw 0
  }

  sb3d::camera set pitch 0
}

proc keyPress { k } {
  #puts "keyPress $k"

  if       {$k == "left"} {
    echo "left"

    if       {$::player_dir == "N"} {
      set ::player_dir "W"
    } elseif {$::player_dir == "W"} {
      set ::player_dir "S"
    } elseif {$::player_dir == "S"} {
      set ::player_dir "E"
    } elseif {$::player_dir == "E"} {
      set ::player_dir "N"
    }

    updatePlayerCamera
  } elseif {$k == "right"} {
    echo "right"

    if       {$::player_dir == "N"} {
      set ::player_dir "E"
    } elseif {$::player_dir == "E"} {
      set ::player_dir "S"
    } elseif {$::player_dir == "S"} {
      set ::player_dir "W"
    } elseif {$::player_dir == "W"} {
      set ::player_dir "N"
    }

    updatePlayerCamera
  } elseif {$k == "up"} {
    echo "up"

    if       {$::player_dir == "N"} {
      set ::player_y [expr {$::player_y + 1}]
    } elseif {$::player_dir == "E"} {
      set ::player_x [expr {$::player_x + 1}]
    } elseif {$::player_dir == "S"} {
      set ::player_y [expr {$::player_y - 1}]
    } elseif {$::player_dir == "W"} {
      set ::player_x [expr {$::player_x - 1}]
    }

    updatePlayerCamera
  } elseif {$k == "down"} {
    echo "down"

    if      {$::player_dir == "N"} {
      set ::player_y [expr {$::player_y - 1}]
    } elseif {$::player_dir == "E"} {
      set ::player_x [expr {$::player_x - 1}]
    } elseif {$::player_dir == "S"} {
      set ::player_y [expr {$::player_y + 1}]
    } elseif {$::player_dir == "W"} {
      set ::player_x [expr {$::player_x + 1}]
    }

    updatePlayerCamera
  }
}
