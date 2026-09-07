proc addNormals { } {
  if {! [info exists ::normal_line_list]} {
    set ::normal_line_list [sb3d::line_list]

    $::normal_line_list set pseudo 1
  }

  set size  [sb3d::canvas get bbox.max_size]
  set size1 [expr {$size/50.0}]

  set objects [sb3d::canvas get objects]

  set n 0

  foreach object $objects {
    set pseudo [$object get pseudo]
    if {$pseudo} { continue }

    set faces [$object get faces]

    set n [expr {$n + [llength $faces]}]
  }

  $::normal_line_list set list.size $n

  set i 0

  foreach object $objects {
    set pseudo [$object get pseudo]
    if {$pseudo} { continue }

    set faces [$object get faces]

    foreach face $faces {
      set center [$object get face.center $face]
      set normal [$object get face.normal $face]

      set p [addVectors $center $normal $size1]

      $::normal_line_list set line.color yellow  $i
      $::normal_line_list set line.start $center $i
      $::normal_line_list set line.end   $p      $i

      incr i
    }
  }

  sb3d::canvas exec update
}

proc removeNormals { } {
  $::normal_line_list set list.size 0

  sb3d::canvas exec update
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
