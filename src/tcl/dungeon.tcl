proc init { } {
  set ::dungeon [sb3d::dungeon]

  $::dungeon set texture.wall dungeon/wall1.jpg
  $::dungeon set texture.door dungeon/door1.jpg
  $::dungeon set texture.floor dungeon/floor.gif

  $::dungeon set filename dungeon/maze.xml

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
