proc randIn { min max } {
  return [expr {rand()*($max - $min) + $min}]
}

proc irandIn { min max } {
  return [expr {int(rand()*($max - $min) + 0.5) + $min}]
}

proc loadRefModel { name } {
  set obj [loadModel "${::model_dir}/${name}.obj" "${name}_ref"  ]
  return $obj
}

proc loadModel { filename name } {
  # echo "$filename $name"

  set obj [sb3d::model $filename]

  $obj set id      $name
  $obj set visible 0

  return $obj
}

proc loadLog { } {
  if {! [info exists ::logRefObj]} {
    setModelDir "tcl3d/Frogger"

    set ::logRefObj [loadRefModel "log"]
  }
}

proc addLog { pos } {
  loadLog

  set il -1

  for {set i 0} {$i < $::num_logs} {incr i} {
    if {[$::log_obj($i) get visible] == 0} {
      set il $i
      break
    }
  }

  if {$il < 0} {
    if {$::num_logs < $::max_logs} {
      set il $::num_logs

      set ::log_obj($il) [$::logRefObj get ref_object]

      $::log_obj($il) set id "log.$il"

      $::log_obj($il) set scales [list 0.6 0.3 0.6]

      set ::num_logs [expr {$il + 1}]
    }
  }

  if {$il < 0} {
    return ""
  }

  set ::log_pos($il) $pos

  $::log_obj($il) set position $pos

  $::log_obj($il) set visible 1

  return $::log_obj($il)
}

proc addNewLog { ix iy } {
  # echo "addNewLog $ix $iy"

  set x [expr {$ix*$::log_dx - 8}]
  set y 1
  set z [expr {$iy*$::log_dz + $::log_z}]

  set obj [addLog [list $x $y $z]]

  if {$obj == ""} {
    return 0
  } else {
    return 1
  }
}

#---

proc loadCar { color } {
  if {! [info exists ::carRefObj($color)]} {
    setModelDir "data"

    set ::carRefObj($color) [loadRefModel "race_car"]

    set carMaterial [sb3d::canvas get material "CarPaint.2"]
    if {$carMaterial == ""} {
      set carMaterial [sb3d::canvas get material "CarPaint.1"]
    }
    if {$carMaterial == ""} {
      set carMaterial [sb3d::canvas get material "CarPaint"]
    }
    # echo "$carMaterial"

    set carMaterial1 [sb3d::material]
    $carMaterial1 set name "carMaterial($color)"
    # echo "$carMaterial1"

    $carMaterial1 set diffuse $color

    $::carRefObj($color) exec replace_material $carMaterial $carMaterial1
echo "$::carRefObj($color) $color"
  }
}

proc addCar { pos color } {
  loadCar $color

  set il -1

  for {set i 0} {$i < $::num_cars} {incr i} {
    if {[$::car_obj($i) get visible] == 0} {
      set il $i
      break
    }
  }
  
  if {$il < 0} {
    if {$::num_cars < $::max_cars} {
      set il $::num_cars
  
      set ::car_obj($il) [$::carRefObj($color) get ref_object]
  
      $::car_obj($il) set id "car.$il"

      $::car_obj($il) set angles [list 0 90 0]
      $::car_obj($il) set scale 0.8
  
      set ::num_cars [expr {$il + 1}]
    }
  }

  if {$il < 0} { 
    return ""
  }
  
  set ::car_pos($il) $pos

  $::car_obj($il) set position $pos

  $::car_obj($il) set visible 1

  return $::car_obj($il)
}

proc addNewCar { ix iy } {
  # echo "addNewCar $ix $iy"

  set x [expr {$ix*$::car_dx - 8}]
  set y 1
  set z [expr {$iy*$::car_dz + $::car_z}]

  set ic [irandIn 0 3]

  set color "red"

  if       {$ic == 0} {
    set color red
  } elseif {$ic == 1} {
    set color green
  } elseif {$ic == 2} {
    set color blue
  } elseif {$ic == 3} {
    set color orange
  }
echo "Car Color $ic $color"

  set obj [addCar [list $x $y $z] $color]

  if {$obj == ""} {
    return 0
  } else {
    return 1
  }
}

#---

proc addFrog { } {
  setModelDir "tcl3d/Frogger"

  set obj [loadModel "tcl3d/Frogger/frog.obj" "frog"]

  $obj set angles [list 0 180 0]
  $obj set scale 0.5

  $obj set visible 1

  return $obj
}

#---

proc addBase { } {
  setModelDir "tcl3d/Frogger"

  set obj [loadModel "tcl3d/Frogger/frogger_base.obj" "base"]

  $obj set visible 1

  # echo [sb3d::canvas get materials]

  set grassMaterial [sb3d::canvas get material "Grass"]
  # echo $grassMaterial

  set grass_texture [sb3d::texture "textures/grass_texture.jpg"]
  $grass_texture set name "grass_texture"

  $grassMaterial set diffuse_texture $grass_texture

  return $obj
}

#---

proc addRoad { pos } {
  set obj [sb3d::plane]

  $obj set color "#333"

  set road_texture [sb3d::texture "textures/road_texture.jpg"]
  $road_texture set name "road_texture"

  $obj set texture $road_texture

  $obj set tex_coords [list [list 0 0] [list 6 3]]

  $obj set id road

  $obj set scales [list 24 7.5 1]
  $obj set angles [list -90 0 0]

  $obj set position $pos

  return $obj
}

proc addWater { pos } {
  set obj [sb3d::plane]

  $obj set color blue

  set water_texture [sb3d::texture "textures/water_texture.png"]
  $water_texture set name "water_texture"

  $obj set texture $water_texture

  set ::shader [sb3d::shader]

  $::shader set vertex_shader   custom_shaders/plane_water_shader.vs
  $::shader set fragment_shader custom_shaders/plane_water_shader.fs

  $obj set shader $::shader

  $obj set id water

  $obj set scales [list 24 8 1]
  $obj set angles [list -90 0 0]

  $obj set position $pos

  return $obj
}

#---

proc setModelDir { dir } {
  set ::model_dir $dir

  sb3d::canvas set model_dir $::model_dir
}

#---

proc bboxChanged { } {
  sb3d::light exec reset 1

  sb3d::light set position [list $::frog_x 5 $::frog_z] 1

  sb3d::camera set pitch 85
}

#---

proc init { } {
  addBase

  addRoad  [list 0 0  5]
  addWater [list 0 0 -5]

  # ---

  set ::frog_x 0
  set ::frog_y 1
  set ::frog_z 10

  set ::frog_obj [addFrog]

  $::frog_obj set position [list $::frog_x $::frog_y $::frog_z]

  # ---

  set ::num_logs 0
  set ::max_logs 20
  set ::log_vx   -0.3

  set ::log_dx 4
  set ::log_dz 2

  set ::log_z -7.0

  set ::log_last_iy -1

  # ---

  set ::num_cars 0
  set ::max_cars 20
  set ::car_vx  -0.5

  set ::car_dx 4
  set ::car_dz 2

  set ::car_z 3.0

  set ::car_last_iy -1

  # ---
    
  sb3d::canvas set camera.type first_person
    
  sb3d::camera set distance 9
  sb3d::camera set position {0 9 10}
}

#---

proc tick { } {
  if {[randIn 0 1] < 0.08} {
    while {1} {
      set iy [irandIn 0 2]

      if {$iy != $::log_last_iy} {
        break
      }
    }

    if {[addNewLog 4 $iy]} {
      set ::log_last_iy $iy
    }
  }

  #---

  if {[randIn 0 1] < 0.03} {
    while {1} {
      set iy [irandIn 0 2]

      if {$iy != $::car_last_iy} {
        break
      }
    }

    if {[addNewCar 4 $iy]} {
      set ::car_last_iy $iy
    }
  }

  #---

  for {set i 0} {$i < $::num_logs} {incr i} {
    set x [expr {[lindex $::log_pos($i) 0] + $::log_vx}]
    set y [lindex $::log_pos($i) 1]
    set z [lindex $::log_pos($i) 2]

    if {$x < -10} {
      $::log_obj($i) set visible 0
    }

    set ::log_pos($i) [list $x $y $z]

    $::log_obj($i) set position $::log_pos($i)
  }

  for {set i 0} {$i < $::num_cars} {incr i} {
    set x [expr {[lindex $::car_pos($i) 0] + $::car_vx}]
    set y [lindex $::car_pos($i) 1]
    set z [lindex $::car_pos($i) 2]

    if {$x < -10} {
      set x 10
    }

    set ::car_pos($i) [list $x $y $z]

    $::car_obj($i) set position $::car_pos($i)
  }

  #---

  for {set i 0} {$i < $::num_logs} {incr i} {
    if {! [$::log_obj($i) get visible]} {
      continue
    }

if {0} {
    echo "Intersect $::log_obj($i) $::frog_obj"

    if {[$::log_obj($i) exec intersect $::frog_obj]} {
      echo "Hit Log"
    }
}
  }

  for {set i 0} {$i < $::num_cars} {incr i} {
    if {! [$::car_obj($i) get visible]} {
      continue
    }

if {0} {
    echo "Intersect $::car_obj($i) $::frog_obj"

    if {[$::car_obj($i) exec intersect $::frog_obj]} {
      echo "Hit Car"
    }
}
  }
}

#---

proc keyPress { k } {
  if       {$k == "up"} {
    set ::frog_z [expr {$::frog_z - 1}]

    $::frog_obj set position [list $::frog_x $::frog_y $::frog_z]
  } elseif {$k == "down"} {
    set ::frog_z [expr {$::frog_z + 1}]

    $::frog_obj set position [list $::frog_x $::frog_y $::frog_z]
  } elseif {$k == "left"} {
    set ::frog_x [expr {$::frog_x - 1}]

    $::frog_obj set position [list $::frog_x $::frog_y $::frog_z]
  } elseif {$k == "right"} {
    set ::frog_x [expr {$::frog_x + 1}]

    $::frog_obj set position [list $::frog_x $::frog_y $::frog_z]
  }

  sb3d::light set position [list $::frog_x 5 $::frog_z] 1
}
