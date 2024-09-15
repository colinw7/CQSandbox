proc init { } {
  set ::whale [sb3d::sprite]

  set ::whale_dx 0.05

  $::whale set position {0.7 -0.2 1.0}
  $::whale set scale 0.2
  $::whale set add_image oceanblast/whale_01_l.png
  $::whale set add_image oceanblast/whale_02_l.png
  $::whale set add_image oceanblast/whale_03_l.png
  $::whale set add_image oceanblast/whale_04_l.png
  $::whale set add_image oceanblast/whale_01_r.png
  $::whale set add_image oceanblast/whale_02_r.png
  $::whale set add_image oceanblast/whale_03_r.png
  $::whale set add_image oceanblast/whale_04_r.png
  $::whale set velocity  [list -$::whale_dx 0.0]
  $::whale set image_start 0
  $::whale set image_end   3

  set ::squid [sb3d::sprite]

  set ::squid_dy 0.05

  $::squid set position {0.3 0.0 2.0}
  $::squid set scale 0.1
  $::squid set add_image oceanblast/squid_01.png
  $::squid set add_image oceanblast/squid_02.png
  $::squid set add_image oceanblast/squid_03.png
  $::squid set add_image oceanblast/squid_04.png
  $::squid set add_image oceanblast/squid_05.png
  $::squid set velocity  [list 0.0 -$::squid_dy]

  set ::starfish [sb3d::sprite]

  $::starfish set position {0.1 0.0 2.0}
  $::starfish set scale 0.05
  $::starfish set add_image oceanblast/starfish.png

  set ::fish1 [sb3d::sprite]
  
  $::fish1 set position {-0.1 0.0 2.0}
  $::fish1 set scale 0.05 
  $::fish1 set add_image oceanblast/fish1.png

  set ::fish2 [sb3d::sprite]
  
  $::fish2 set position {-0.3 0.0 2.0}
  $::fish2 set scale 0.05 
  $::fish2 set add_image oceanblast/fish2.png

  set ::fish3 [sb3d::sprite]
  
  $::fish3 set position {-0.5 0.0 2.0}
  $::fish3 set scale 0.05 
  $::fish3 set add_image oceanblast/fish3.png

  set ::sub [sb3d::sprite]

  set ::sub_x 0.0
  set ::sub_y 0.0
  set ::sub_z 0.0

  $::sub set position [list $::sub_x $::sub_y $::sub_z]
  $::sub set scale 0.1
  $::sub set add_image oceanblast/sub_01.png
  $::sub set add_image oceanblast/sub_02.png

  updateSubImages 0

  set ::max_torpedos 3

  for {set i 0} {$i < $::max_torpedos} {incr i} {
    set ::torpedo($i) [sb3d::sprite]

    $::torpedo($i) set scale 0.1
    $::torpedo($i) set add_image oceanblast/torpedo_l.png
    $::torpedo($i) set add_image oceanblast/torpedo_r.png

    $::torpedo($i) set image_start 0
    $::torpedo($i) set image_end   0

    $::torpedo($i) set visible 0
  }
}

proc updateSubImages { d } {
  set ::sub_dir $d

  $::sub set image_start $::sub_dir
  $::sub set image_end   $::sub_dir
}

proc tick { args } {
  set whale_pos [$::whale get position]

  set wx [lindex $whale_pos 0]

  if       {$wx < -1.0} {
    $::whale set image_start 4
    $::whale set image_end   7
    $::whale set velocity    [list $::whale_dx 0.0]
  } elseif {$wx > 1.0} {
    $::whale set image_start 0
    $::whale set image_end   3
    $::whale set velocity    [list -$::whale_dx 0.0]
  }

  set squid_pos [$::squid get position]

  set sy [lindex $squid_pos 1]

  if       {$sy < -1.0} {
    $::squid set velocity [list 0.0 $::squid_dy]
  } elseif {$sy > 1.0} {
    $::squid set velocity [list 0.0 -$::squid_dy]
  }
}

proc keyPress { key } {
  if       {$key == "left"} {
    set ::sub_x [expr {$::sub_x - 0.01}]

    $::sub set position [list $::sub_x $::sub_y $::sub_z]

    updateSubImages 1
  } elseif {$key == "right"} {
    set ::sub_x [expr {$::sub_x + 0.01}]

    $::sub set position [list $::sub_x $::sub_y $::sub_z]

    updateSubImages 0
  } elseif {$key == "up"} {
    set ::sub_y [expr {$::sub_y + 0.01}]

    $::sub set position [list $::sub_x $::sub_y $::sub_z]
  } elseif {$key == "down"} {
    set ::sub_y [expr {$::sub_y - 0.01}]

    $::sub set position [list $::sub_x $::sub_y $::sub_z]
  } elseif {$key == "space"} {
    set ind 0

    $::torpedo($ind) set position [list $::sub_x $::sub_y 1]
    $::torpedo($ind) set visible 1

    if {$::sub_dir == 0} {
      $::torpedo($ind) set velocity [list 0.03 0.0]
      $::torpedo($ind) set image_start 1
      $::torpedo($ind) set image_end   1
    } else {
      $::torpedo($ind) set velocity [list -0.03 0.0]
      $::torpedo($ind) set image_start 0
      $::torpedo($ind) set image_end   0
    }
  } else {
    puts "keyPress $key"
  }
}
