proc showOrient { } {
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
      }

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
}
