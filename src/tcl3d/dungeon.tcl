proc init { } {
  set idir "tcl3d/dungeon"

  set ::dungeon [sb3d::dungeon]

  $::dungeon set texture.wall  $idir/wall1.jpg
  $::dungeon set texture.door  $idir/door1.jpg
  $::dungeon set texture.floor $idir/floor.gif

  $::dungeon set filename $idir/maze.xml

  sb3d::canvas set cull_face 0

  sb3d::camera set near 0.01

  sb3d::canvas set mode game
}

proc setMode { mode } {
  if {$mode == "game"} {
    $::dungeon set player_camera 1
  } else {
    $::dungeon set player_camera 0
  }
}

proc keyPress { k } {
  #puts "keyPress $k"

  if       {$k == "left"} {
    $::dungeon set player.left 1
  } elseif {$k == "right"} {
    $::dungeon set player.right 1
  } elseif {$k == "up"} {
    $::dungeon set player.up 1
  } elseif {$k == "down"} {
    $::dungeon set player.down 1
  }
}
