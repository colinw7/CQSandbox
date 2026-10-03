proc init { } {
  set ::skybox [sb3d::skybox]

  set images [list \
space_cube_map/right.png \
space_cube_map/left.png \
space_cube_map/top.png \
space_cube_map/bottom.png \
space_cube_map/front.png \
space_cube_map/back.png \
]

  $::skybox set images $images

# set ::model [sb3d::model models/v3d/InfLoopShip.V3D]

  set ::shape1 [sb3d::shape]
  $::shape1 set cube [list 1.0 1.0 1.0]
  $::shape1 set position {-1 0 0}

  set ::shape2 [sb3d::shape]
  $::shape2 set cube [list 1.0 1.0 1.0]
  $::shape2 set position {1 0 0}

  sb3d::canvas set reflection_map 1
  sb3d::canvas set refraction_map 1

  #$::shape1 set reflectivity 0.75
  #$::shape2 set refractivity 0.75

# sb3d::canvas set reflectivity 1
# sb3d::canvas set refractivity 1

  $::shape1 set transparency 0.8
  $::shape1 set layer        1

  $::shape2 set transparency 0.8
  $::shape2 set layer        2
}
