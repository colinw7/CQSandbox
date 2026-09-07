proc showOrient { } {
  if {! [info exists ::orient_point_list]} {
    set ::orient_point_list [sb3d::point_list]

    $::orient_point_list set point.size 0.3

    $::orient_point_list set cull_face 1

    $::orient_point_list set pseudo 1
  }

  set objects [sb3d::canvas get objects]

  set n 0

  foreach object $objects {
    set pseudo [$object get pseudo]
    if {$pseudo} { continue }

    set faces [$object get faces]

    set n [expr {$n + [llength $faces]}]
  }

  $::orient_point_list set list.size $n

  set front_face [sb3d::canvas get front_face]

  if {$front_face} {
    set expected_orient "anticlockwise"
  } else {
    set expected_orient "clockwise"
  }

  set front_face [sb3d::canvas get front_face]

  set i 0

  foreach object $objects {
    set pseudo [$object get pseudo]
    if {$pseudo} { continue }

    foreach face $faces {
      set center [$object get face.center $face]

      $::orient_point_list set point.position $center $i

      set orient [$object get face.orient $face]

      if {$orient == $expected_orient} {
        set color green
      } else {
        set color red
      }

      $::orient_point_list set point.color $color $i

      incr i
    }
  }

  sb3d::canvas exec update
}

proc hideOrient { } {
  $::orient_point_list set list.size 0

  sb3d::canvas exec update
}
