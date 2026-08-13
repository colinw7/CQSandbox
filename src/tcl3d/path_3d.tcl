proc init { } {
  set ::path [sb3d::path]

  set p1 [list 0.0 0.0 0.0]
  set p2 [list 1.0 0.0 0.0]
  set p3 [list 1.0 1.0 0.0]
  set p4 [list 0.0 1.0 0.0]

  $::path set path [list [list M $p1] [list L $p2] [list L $p3] [list L $p4]]
}
