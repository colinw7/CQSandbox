proc init { } {
  if {0} {
    set ::moveObj [sb3d::shape]

    $::moveObj set cube

    $::moveObj set color    white
    $::moveObj set position [list 0 0 -20]
  } else {
    set ::model_dir data

    sb3d::canvas set model_dir $::model_dir

    set ::moveObj [loadModel "$::model_dir/last_war_wall.obj" "last_war_wall"]

    $::moveObj set position [list 0 0 -20]
  }

  sb3d::canvas set bg "#aaaaaa"
  
  sb3d::canvas set camera.type first_person

  sb3d::camera set pitch    -90
  sb3d::camera set distance 18
  sb3d::camera set position {0 3 5}
}

proc loadModel { filename name } {
  set obj [sb3d::model $filename]

  $obj set id $name

  return $obj
}

proc tick { } {
  set pos [$::moveObj get position]

  set z [expr {[lindex $pos 2] + 0.1}]

  if {$z < 5} {
    $::moveObj set position [list [lindex $pos 0] [lindex $pos 1] $z]
  } else {
    $::moveObj set position [list [lindex $pos 0] [lindex $pos 1] -20]
  }
}
