source "tcl3d/addNormals.tcl"
source "tcl3d/showOrient.tcl"

proc addCone  { cap patches xy r h pos c } {
  set shape [sb3d::shape]

  $shape set cap_type    $cap
  $shape set num_patches $patches
  $shape set num_xy      $xy

  $shape set cone [list $r $h]

  $shape set color $c

  $shape set position $pos

  return $shape
}

proc init { } {
  set ::shape1 [addCone NONE 16  8 0.2 0.5 {0 0 0} "#ff0000"]
  set ::shape2 [addCone NGON 8  16 0.3 0.5 {1 0 0} "#00ff00"]
  set ::shape3 [addCone FAN  10 10 0.4 0.5 {2 0 0} "#0000ff"]

# $::shape1 set wireframe 1

  sb3d::ui create "\
<qxml>\n\
<QVBoxLayout>
<QPushButton text=\"Normals\" onClicked=\"normalsProc\"/>\n\
<QPushButton text=\"Orientation\" onClicked=\"orientSlot\"/>\n\
</QVBoxLayout>
<QLayoutItem stretch=\"1\"/>\n\
</qxml>"
}

proc normalsProc { } {
  addNormals
} 

proc orientSlot { } {
  showOrient
} 
