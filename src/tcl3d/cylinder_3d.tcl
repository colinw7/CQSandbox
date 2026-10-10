proc addCylinder { cap patches xy r h pos c } {
  set shape [sb3d::shape]
  
  $shape set cap_type    $cap
  $shape set num_patches $patches
  $shape set num_xy      $xy

  $shape set cylinder [list $r $h]

  $shape set color $c

  $shape set position $pos

  return $shape
}

proc init { } {
  set ::shape1 [addCylinder NONE 16  8 0.2 0.5 {0 0 0} "#ff0000"]
  set ::shape2 [addCylinder NGON 8  16 0.3 0.5 {1 0 0} "#00ff00"]
  set ::shape3 [addCylinder FAN  10 10 0.4 0.5 {2 0 0} "#0000ff"]

# $::shape1 set wireframe 1
}
