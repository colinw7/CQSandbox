proc init { } {
  set earthmap [sb3d::texture "textures/earthmap1k.jpg"]
  $earthmap set name "earth_texture"

  set material [sb3d::material]
  $material set name "earth_material"

  $material set texture $earthmap

  set sphere [sb3d::shape]
  $sphere set sphere [list 1.0]
  $sphere set position [list 2 2 0]
  $sphere set color orange
  $sphere set material $material
}
