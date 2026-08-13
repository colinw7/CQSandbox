proc init { } {
  set ::skybox [sb3d::skybox]

  set images [list \
cube_map/cube_right.jpg \
cube_map/cube_left.jpg \
cube_map/cube_top.jpg \
cube_map/cube_bottom.jpg \
cube_map/cube_front.jpg \
cube_map/cube_back.jpg \
]

  $::skybox set images $images
}
