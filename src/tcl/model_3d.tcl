proc init { } {
  set ::model [sb3d::model models/v3d/F15.V3D]
  #set ::model [sb3d::model models/3ds/batwing.3ds]
  #set ::model [sb3d::model models/ply/shield.ply]
  #set ::model [sb3d::model models/gltf/Earth.gltf]

  #cd /work/colinw/packages/glTF-Sample-Assets-main/Models/ABeautifulGame/glTF
  #set ::model [sb3d::model ABeautifulGame.gltf]

  #$::model set diffuse_texture  models/ply/shield_diffuse.png
  #$::model set specular_texture models/ply/shield_spec.png
  #$::model set normal_texture   models/ply/shield_normal.png

  sb3d::custom_form string -label "Model" -proc modelProc
}
