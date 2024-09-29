proc init { } {
  #set ::model [sb3d::model models/v3d/F15.V3D]
  #set ::model [sb3d::model models/3ds/batwing.3ds]
  set ::model [sb3d::model models/ply/shield.ply]

  $::model set diffuse_texture  models/ply/shield_diffuse.png
  $::model set specular_texture models/ply/shield_spec.png
  $::model set normal_texture   models/ply/shield_normal.png
}
