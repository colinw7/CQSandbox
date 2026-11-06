source "tcl3d/addNormals.tcl"

proc init { } {
  set ::shape [sb3d::shape]

  $::shape set sphere 0.4

# $::shape set wireframe 1

  set earthmap [sb3d::texture "textures/earthmap1k.jpg"]
  $earthmap set name "earthmap"

  $::shape set texture $earthmap
}

proc bboxChanged { } {
  addNormals
}
