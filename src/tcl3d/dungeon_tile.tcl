proc loadModel { filename name { s 1.0 } } {
  set obj [sb3d::model $filename]

  $obj set id $name

  if {$s != 1.0} {
    $obj set scale $s
  }

  return $obj
}


proc init { } {
  set model_dir "tcl3d/Dungeon_Assets/obj"

  sb3d::canvas set model_dir $model_dir

  set ::tileObj [loadModel "$model_dir/floor_wood_large.obj" "tile"]
}
