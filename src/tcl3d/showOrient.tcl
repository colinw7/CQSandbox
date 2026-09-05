proc showOrient { } {
  set objects [sb3d::canvas get objects]

  foreach object $objects {
    set faces [$object get faces]

    foreach face $faces {
      set orient [$object get face.orient $face]
      set center [$object get face.center $face]

      set point [sb3d::point]

      $point set position $center
      $point set size     8

      if {$orient == "clockwise"} {
        $point set color red
      } else {
        $point set color green
      }
    }
  }
}
