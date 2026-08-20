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

  #$::model set anim.name "Walking_A"
  $::model set anim.name "Idle"
  $::model set anim.step 0.1

  $::model set child.visible "Barbarian_Hat"  0
  $::model set child.visible "Mug"            0
  $::model set child.visible "1H_Axe"         0
  $::model set child.visible "1H_Axe_Offhand" 0

  sb3d::canvas set loop.enabled 1
  sb3d::canvas set loop.timeout 100

  sb3d::camera set disable_roll 1

  $::model exec translate [list 0 0 0]

  # sb3d::canvas set mode game
}
