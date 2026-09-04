proc addNormals { } {
  set objects [sb3d::canvas get objects]

  foreach object $objects {
    set type [$object get type_name]

    set faces [$object get faces]

echo $faces

    foreach face $faces {
      set center [$object get face.center $face]
      set normal [$object get face.normal $face]

      set p [addVectors $center $normal]

      set path [sb3d::path]

      $path set path [list [list M $center] [list L $p]]
    }
  }
}

proc addVectors { v1 v2 } {
  set x1 [lindex $v1 0]
  set y1 [lindex $v1 1]
  set z1 [lindex $v1 2]

  set x2 [lindex $v2 0]
  set y2 [lindex $v2 1]
  set z2 [lindex $v2 2]

  return [list [expr {$x1 + $x2}] [expr {$y1 + $y2}] [expr {$z1 + $z2}]]
}
