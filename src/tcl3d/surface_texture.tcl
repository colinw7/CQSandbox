proc init { } {
  set ::surface [sb3d::surface]

  $::surface set position {-0.5 0 -0.5}

  $::surface set size {256 256}
# $::surface set size {32 32}

# set height_map [sb3d::texture "water_textures/heightmap.png"]
  set height_map [sb3d::texture "pics/Catwoman.jpg"]

# set earthmap [sb3d::texture "textures/earthmap1k.jpg"]
# $earthmap set name "earth_texture"

  set material [sb3d::material]
  $material set name "earth_material"

# $material set texture $earthmap
  $material set texture $height_map

  $::surface set map.channel gray
  $::surface set map.scale   0.01
  $::surface set map.texture $height_map

  $::surface set material $material

  # $::surface set scales {1 0.1 1}

  $::surface set cull_face 0
}
