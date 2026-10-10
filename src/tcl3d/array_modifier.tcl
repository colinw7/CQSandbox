proc init { } {
  #set ::model [sb3d::model models/v3d/F15.V3D]
  #set ::modifier [sb3d::::modifier $::model]

  set ::cube [sb3d::shape]
  $::cube set cube [list 1.0 1.0 1.0]
  set ::modifier [sb3d::::modifier $::cube]

  $::modifier set count  3
  $::modifier set offset {1 1 1}
}
