proc randIn { min max } {
  return [expr {rand()*($max - $min) + $min}]
}

proc init { } {
  set road [sb3d::shape]

  $road set cube

  $road set color    "#222222"
  $road set scales   [list 2 0.1 1000]
  $road set position [list 0 0 0]

  # ---

  set ::game_over 0

  set ::far_x -20.0

  # ---

  set ::player_x    0.0
  set ::player_y    0.2
  set ::player_z    3.7
  set ::player_dx   0.025
  set ::player_dead 0

  set ::maxPlayerObjs 20

  for {set i 0} {$i < $::maxPlayerObjs} {incr i} {
    set ::playerObjs($i) ""

    set ::playerOffsetX($i) 0
    set ::playerOffsetZ($i) 0
  }

  addPlayerObj

  # ---

  set ::enemy_dz 0.1

  set ::maxEnemyObjs 20

  for {set i 0} {$i < $::maxEnemyObjs} {incr i} {
    set ::enemyObjs($i) ""
  }

  # ---

  set ::axe_da 10
  set ::axe_dz 0.25

  set ::maxAxeObjs 25

  for {set i 0} {$i < $::maxAxeObjs} {incr i} {
    set ::axeObjs($i) ""
  }

  # ---

  set ::wall_dz 0.1
  set ::text_dy 0.25

  set ::maxWallObjs 10

  for {set i 0} {$i < $::maxWallObjs} {incr i} {
    set ::wallObjs($i) ""
  }

  for {set i 0} {$i < $::maxWallObjs} {incr i} {
    set ::wallTextObjs($i) ""
  }

  # ---

  sb3d::canvas set bg "#aaaaaa"

  sb3d::canvas set camera.type first_person
    
  sb3d::camera set pitch    -90
  sb3d::camera set distance 18
  sb3d::camera set position {0 1 5}

  sb3d::canvas set mode game

  sb3d::canvas set loop.enabled 1
  sb3d::canvas set loop.timeout 30
}

proc bboxChanged { } {
  sb3d::light exec reset 1

  sb3d::light set position [list 0 0.5 4] 1
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

  $obj set position [list 0.0 $::player_y $::player_z]
  $obj set scales   [list 0.1 0.1 0.1]
  $obj set angles   [list 0 180 0]

  $obj set anim.name   "Idle"
  $obj set anim.repeat 1
  $obj set anim.step   0.1
    
  $obj set child.visible "Barbarian_Hat"  0
  $obj set child.visible "Mug"            0
  $obj set child.visible "1H_Axe"         0
  $obj set child.visible "1H_Axe_Offhand" 0

  return $obj
} 

proc loadEnemyObj { } {
  if {! [info exists ::enemyRefObj]} {
    setModelDir "tcl3d/Dungeon_Skeletons/gltf"

    set ::enemyRefObj [loadModel "$::model_dir/Skeleton_Warrior.glb" "enemy_ref"]

    setModelDir "tcl3d/Character_Animations/gltf/Rig_Medium"

    set ::enemyAnim1Obj [loadModel "$::model_dir/Rig_Medium_General.glb" "enemy_anim"]
    set ::enemyAnim2Obj [loadModel "$::model_dir/Rig_Medium_MovementBasic.glb" "enemy_anim"]

    $::enemyRefObj exec add_anim $::enemyAnim1Obj
    $::enemyRefObj exec add_anim $::enemyAnim2Obj
  }

  set obj [addObject $::enemyRefObj enemy]

  $obj set position [list 0 0.1 3.0]
  $obj set scales   [list 0.1 0.1 0.1]
  $obj set angles   [list 0 0 0]

  $obj set anim.name "Idle_A"
  $obj set anim.step 0.1

  return $obj
}

proc loadAxeObj { } {
  if {! [info exists ::axeRefObj]} {
    setModelDir "data"

    set ::axeRefObj [loadModel "$::model_dir/Barbarian_Axe.obj" "axe_ref"]
  }

  set obj [addObject $::axeRefObj axe]

  $obj set position [list 0 0.1 3.0]
  $obj set scales   [list 0.2 0.2 0.2]
  $obj set angles   [list 0 0 0]

  return $obj
}

proc loadWallObj { } {
  if {! [info exists ::wallRefObj]} {
    setModelDir "data"

    set ::wallRefObj [loadModel "$::model_dir/last_war_wall.obj" "last_war_wall"]
  }

  set obj [addObject $::wallRefObj wall]

  $obj set position [list 0 0.1 3.0]
  $obj set scales   [list 0.2 0.2 0.2]
  $obj set angles   [list 0 0 0]

  return $obj
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

  # echo [$obj get anim.names]

  return $obj
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

proc setModelDir { dir } {
  set ::model_dir $dir 
  
  sb3d::canvas set model_dir $::model_dir
}

proc tick { } {
  if {$::game_over} {
    return
  }

  # ---

  if       {[sb3d::canvas get key "a"]} {
    playerMoveLeft
  } elseif {[sb3d::canvas get key "d"]} {
    playerMoveRight
  }

  # ---

  for {set i 0} {$i < $::maxAxeObjs} {incr i} {
    if {$::axeObjs($i) == ""} {
      continue
    }

    if {! [$::axeObjs($i) get visible]} {
      continue 
    }

    set pos    [$::axeObjs($i) get position]
    set angles [$::axeObjs($i) get angles]

    set z  [expr {[lindex $pos    2] - $::axe_dz}]
    set ay [expr {[lindex $angles 1] + $::axe_da}]

    if {$z > $::far_x} {
      $::axeObjs($i) set position [list [lindex $pos 0] [lindex $pos 1] $z]
    } else {
      $::axeObjs($i) set visible 0
    }

    $::axeObjs($i) set angles [list [lindex $angles 0] $ay [lindex $angles 2]]
  }

  # ---
  
  for {set i 0} {$i < $::maxWallObjs} {incr i} {
    if {$::wallObjs($i) == ""} {
      continue
    }

    if {! [$::wallObjs($i) get visible]} {
      continue 
    }

    set pos [$::wallObjs($i) get position]
  
    set x [lindex $pos 0]
    set y [lindex $pos 1]
    set z [expr {[lindex $pos 2] + $::wall_dz}]
  
    if {$z < 5} {
      $::wallObjs($i)     set position [list $x $y                       $z]
      $::wallTextObjs($i) set position [list $x [expr {$y + $::text_dy}] $z]
    } else {
      $::wallObjs($i)     set visible 0
      $::wallTextObjs($i) set visible 0
    }
  }

  # ---

  for {set i 0} {$i < $::maxEnemyObjs} {incr i} {
    if {$::enemyObjs($i) == ""} {
      continue
    }

    if {! [$::enemyObjs($i) get visible]} {
      continue
    }

    set pos [$::enemyObjs($i) get position]

    set z [expr {[lindex $pos 2] + $::enemy_dz}]

    if {$z < 5} {
      $::enemyObjs($i) set position [list [lindex $pos 0] [lindex $pos 1] $z]
    } else {
      $::enemyObjs($i) set visible 0
    }
  }

  # ---

  for {set i 0} {$i < $::maxAxeObjs} {incr i} {
    if {$::axeObjs($i) == ""} { 
      continue
    }
    
    if {! [$::axeObjs($i) get visible]} {
      continue
    }
    
    for {set j 0} {$j < $::maxWallObjs} {incr j} {
      if {$::wallObjs($j) == ""} {
        continue
      }
  
      if {! [$::wallObjs($j) get visible]} {
        continue
      }

      if {[$::axeObjs($i) exec intersect $::wallObjs($j)]} {
        echo "Hit Wall"
  
        incr ::wallCount($j)

        $::wallTextObjs($j) set text $::wallCount($j)

        $::axeObjs($i) set visible 0

        break
      }
    }

    if {! [$::axeObjs($i) get visible]} {
      continue
    }

    for {set j 0} {$j < $::maxEnemyObjs} {incr j} {
      if {$::enemyObjs($j) == ""} {
        continue
      }

      if {! [$::enemyObjs($j) get visible]} {
        continue
      }

      if {[$::axeObjs($i) exec intersect $::enemyObjs($j)]} {
        echo "Hit Enemy" 
  
        $::enemyObjs($j) set visible 0
        $::axeObjs($i)   set visible 0

        break
      }
    }
  }

  # ---

  if {[randIn 0 1] < 0.02} {
    addWallObj
  }

  if {[randIn 0 1] < 0.02} {
    addEnemyObj
  }
}

proc keyPress { k } {
  if       {$k == "space"} {
    playerShoot
  } elseif {$k == "c" || $k == "C"} {
    addPlayerObj
  } elseif {$k == "e" || $k == "E"} {
    addEnemyObj
  }
}

proc playerMoveLeft { } {
  if {$::player_dead} { return }

  set ::player_x [expr {$::player_x - $::player_dx}]

  if {$::player_x < -0.75} {
    set ::player_x -0.75
  }

  for {set i 0} {$i < $::maxPlayerObjs} {incr i} {
    if {$::playerObjs($i) == ""} {
      continue
    }

    set x [expr {$::player_x + $::playerOffsetX($i)}]
    set z [expr {$::player_z + $::playerOffsetZ($i)}]

    $::playerObjs($i) set position [list $x $::player_y $z]
  }
}

proc playerMoveRight { } {
  if {$::player_dead} { return }

  set ::player_x [expr {$::player_x + $::player_dx}]

  if {$::player_x > 0.75} {
    set ::player_x 0.75
  }

  for {set i 0} {$i < $::maxPlayerObjs} {incr i} {
    if {$::playerObjs($i) == ""} {
      continue
    }

    set x [expr {$::player_x + $::playerOffsetX($i)}]
    set z [expr {$::player_z + $::playerOffsetZ($i)}]

    $::playerObjs($i) set position [list $x $::player_y $z]
  }
}

proc playerShoot { } {
  if {$::player_dead} { return }

  for {set i 0} {$i < $::maxPlayerObjs} {incr i} {
    if {$::playerObjs($i) == ""} {
      continue
    }

    # get free axe
    set axeI -1

    for {set j 0} {$j < $::maxAxeObjs} {incr j} {
      if {$::axeObjs($j) == ""} {
        set axeI $j
        set ::axeObjs($j) [loadAxeObj]
        break
      }

      if {! [$::axeObjs($j) get visible]} {
        set axeI $j
        $::axeObjs($j) set visible 1
        break
      }
    }

    # add axe (if free)
    if {$axeI >= 0} {
      set pos [$::playerObjs($i) get position]

      set x [lindex $pos 0]

      $::axeObjs($axeI) set visible 1
      $::axeObjs($axeI) set position [list $x 0.2 3.0]
    }
  }
}

proc addWallObj { } {
  if {$::game_over} { return }

  set wallI -1

  for {set i 0} {$i < $::maxWallObjs} {incr i} {
    if {$::wallObjs($i) == ""} {
      set wallI $i
      set ::wallObjs($i)     [loadWallObj]
      set ::wallTextObjs($i) [sb3d::text]
      break
    }

    if {! [$::wallObjs($i) get visible]} {
      set wallI $i
      $::wallObjs($i)     set visible 1
      $::wallTextObjs($i) set visible 1
      break
    }
  }

  if {$wallI >= 0} {
    $::wallObjs($wallI) set visible 1
    $::wallObjs($wallI) set position [list 0.5 0.0 $::far_x]

    $::wallTextObjs($wallI) set visible 1
    $::wallTextObjs($wallI) set position [list 0.5 $::text_dy -20.0]

    set ::wallCount($wallI) -10

    $::wallTextObjs($wallI) set color white
    $::wallTextObjs($wallI) set text  $::wallCount($wallI)
    $::wallTextObjs($wallI) set size  0.4
  }
}

proc addPlayerObj { } {
  if {$::game_over} { return }

  set playerI -1

  for {set i 0} {$i < $::maxPlayerObjs} {incr i} {
    if {$::playerObjs($i) == ""} {
      set playerI $i
      set ::playerObjs($i) [loadPlayerObj]
      break
    }

    if {! [$::playerObjs($i) get visible]} {
      set playerI $i
      $::playerObjs($i) set visible 1
      break
    }
  }

  if {$playerI >= 0} {
    $::playerObjs($playerI) set visible 1

    placePlayers
  }
}

proc placePlayers { } {
  set r 0.1

  set n 0

  for {set i 0} {$i < $::maxPlayerObjs} {incr i} {
    if {$::playerObjs($i) == ""} {
      continue
    }

    if {! [$::playerObjs($i) get visible]} {
      continue
    } 

    if {$n > 0} {
      set x [randIn -$r $r]
      set y [randIn -$r $r]

      set ::playerOffsetX($i) $x
      set ::playerOffsetZ($i) $y
    } else {
      set ::playerOffsetX($i) 0
      set ::playerOffsetZ($i) 0
    }

    set x [expr {$::player_x + $::playerOffsetX($i)}]
    set z [expr {$::player_z + $::playerOffsetZ($i)}]

    $::playerObjs($i) set position [list $x $::player_y $z]

    incr n
  }
}

proc addEnemyObj { } {
  if {$::game_over} { return }

  set enemyI -1

  for {set i 0} {$i < $::maxEnemyObjs} {incr i} {
    if {$::enemyObjs($i) == ""} {
      set enemyI $i
      set ::enemyObjs($i) [loadEnemyObj]
      break
    }

    if {! [$::enemyObjs($i) get visible]} {
      set enemyI $i
      $::enemyObjs($i) set visible 1
      break
    }
  }

  if {$enemyI >= 0} {
    $::enemyObjs($enemyI) set visible 1
    $::enemyObjs($enemyI) set position [list -0.5 0.2 $::far_x]
  }
}
