proc init { } {
  set ::surface [sb3d::surface]

  $::surface set flag [list 50 50]

  $::surface set wireframe 1
}
