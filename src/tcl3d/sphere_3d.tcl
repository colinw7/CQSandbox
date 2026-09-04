source "tcl3d/addNormals.tcl"

proc init { } {
  set ::shape [sb3d::shape]

  $::shape set sphere 0.4

# $::shape set wireframe 1
}

proc bboxChanged { } {
  addNormals
}
