proc init { } {
  set ::field_runners [sb3d::field_runners]

  $::field_runners set map             field_runners/maps/grasslands.map
  $::field_runners set texture.border  field_runners/images/tree.png
# $::field_runners set texture.block   field_runners/images/block.png
  $::field_runners set texture.gun     field_runners/images/gun1.png
  $::field_runners set texture.grass   field_runners/images/field.png
# $::field_runners set texture.stone   field_runners/images/stone.png
  $::field_runners set texture.soldier field_runners/images/soldier1.png
}
