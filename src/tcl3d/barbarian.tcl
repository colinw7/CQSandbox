proc loadModel { filename name { s 1.0 } } {
  set obj [sb3d::model $filename]

  $obj set id      $name
# $obj set visible 0

  if {$s != 1.0} {
    $obj set scale $s
  }
  
  return $obj
}

proc init { } {
  set model_dir "tcl3d/Dungeon_Characters/gltf"

  sb3d::canvas set model_dir $model_dir

  set ::model [loadModel "$model_dir/Barbarian.glb" "model"]
}
