proc loadModel { filename name { s 1.0 } } {
  set obj [sb3d::model $filename]

  $obj set id      $name
# $obj set visible 0

  if {$s != 1.0} {
    $obj set scale $s
  }
  
  return $obj
}

proc setModelDir { dir } {
  set ::model_dir $dir
  
  sb3d::canvas set model_dir $::model_dir
}

proc init { } {
  setModelDir "tcl3d/Dungeon_Skeletons/gltf"

  set ::modelObj [loadModel "$::model_dir/Skeleton_Warrior.glb" "model"]

  setModelDir "tcl3d/Character_Animations/gltf/Rig_Medium"

  set ::animObj [loadModel "$::model_dir/Rig_Medium_MovementBasic.glb" "enemy_anim"]
# set ::animObj [loadModel "$::model_dir/Rig_Medium_General.glb" "enemy_anim"]

  $::modelObj exec add_anim $::animObj

# $::modelObj set anim.name "Idle"
  $::modelObj set anim.name "Walking_A"
  $::modelObj set anim.step 0.1

  sb3d::canvas set loop.enabled 1
  sb3d::canvas set loop.timeout 100

  sb3d::camera set disable_roll 1

  $::modelObj exec translate [list 0 0 0]

  # sb3d::canvas set mode game
}
