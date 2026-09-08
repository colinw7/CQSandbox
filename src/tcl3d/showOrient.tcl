proc showOrient { } {
  if {! [info exists ::orient_points]} {
    set ::orient_points { }
  }

  foreach point $::orient_points {
    $point set visible 0
  }

  set objects [sb3d::canvas get objects]

  set i 0

  foreach object $objects {
    set type [$object get type_name]

    if {$type == "point" || $type == "path"} {
      continue
    }

    set faces [$object get faces]

    set j 0

    foreach face $faces {
      if {! [info exists ::orient_point($i,$j)]} {
        set ::orient_point($i,$j) [sb3d::point]

        $::orient_point($i,$j) set size 24

        lappend ::orient_points $::orient_point($i,$j)
      }

      $::orient_point($i,$j) set visible 1

      set center [$object get face.center $face]

      $::orient_point($i,$j) set position $center

      set orient [$object get face.orient $face]

      if {$orient == "clockwise"} {
        $::orient_point($i,$j) set color red
      } else {
        $::orient_point($i,$j) set color green
      }

      incr j
    }

    incr i
  }

  sb3d::canvas exec update
}

proc hideOrient { } {
  foreach point $::orient_points {
    $point set visible 0
  }

  sb3d::canvas exec update
}
