proc init { } {
  set ::path [sb3d::path]

  set p1 [list 0 0 0]
  set p2 [list 1 1 1]

  $::path set path [list [list M $p1] [list L $p2]]
}
