proc addNormals { } {
  set size  [sb3d::canvas get bbox.max_size]
  set size1 [expr {$size/50.0}]

  set objects [sb3d::canvas get objects]

  foreach object $objects {
    set faces [$object get faces]
    # echo $faces

    foreach face $faces {
      set center [$object get face.center $face]
      set normal [$object get face.normal $face]

      set p [addVectors $center $normal $size1]

      set path [sb3d::path]

      $path set path [list [list M $center] [list L $p]]
    }
  }
}

proc addVectors { v1 v2 s } {
  set x1 [lindex $v1 0]
  set y1 [lindex $v1 1]
  set z1 [lindex $v1 2]

  set x2 [lindex $v2 0]
  set y2 [lindex $v2 1]
  set z2 [lindex $v2 2]

  return [list [expr {$x1 + $s*$x2}] [expr {$y1 + $s*$y2}] [expr {$z1 + $s*$z2}]]
}
