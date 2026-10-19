proc init { } {
  set ::gizmo [sb3d::gizmo]

  set ::cube [sb3d::shape]

  $::cube set cube   [list 1 1 1]
  $::cube set scales [list 0.1 0.1 0.1]
}

proc keyPress { key } {
  if       {$key == "m"} {
    $::gizmo set edit_type move
  } elseif {$key == "s"} {
    $::gizmo set edit_type scale
  } elseif {$key == "r"} {
    $::gizmo set edit_type rotate
  }
}
