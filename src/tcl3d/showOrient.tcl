proc showOrient { } {
  set objects [sb3d::canvas get objects]

  set i 0

  foreach object $objects {
    set type [$object get type_name]

    if {$type == "point"} {
      continue
    }

    set faces [$object get faces]

    foreach face $faces {
      if {! [info exists ::orient_point($i,$face)]} {
        set ::orient_point($i,$face) [sb3d::point]

        $::orient_point($i,$face) set size     24
      }

      set center [$object get face.center $face]

      $::orient_point($i,$face) set position $center

      set orient [$object get face.orient $face]

      if {$orient == "clockwise"} {
        $::orient_point($i,$face) set color red
      } else {
        $::orient_point($i,$face) set color green
      }
    }

    incr i
  }
}
