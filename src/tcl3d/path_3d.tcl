proc init { } {
  set ::path [sb3d::path]

  set p1 [list 0.0 0.0 0.0]
  set p2 [list 1.0 1.0 1.0]; set p2a [list 1.5 1.2 1.0]
  set p3 [list 2.0 1.0 2.0]; set p3a [list 2.2 0.8 2.0]; set p3b [list 2.8 2.0 2.0]
  set p4 [list 3.0 2.0 1.0]
  set p5 [list 4.0 2.0 0.0]

  $::path set color red

  $::path set path [list \
    [list M $p1] \
    [list L $p2] \
    [list Q $p2a $p3] \
    [list C $p3a $p3b $p4] \
    [list L $p5]]
}
