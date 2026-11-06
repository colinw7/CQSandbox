source "tcl3d/addNormals.tcl"
source "tcl3d/showOrient.tcl"

proc init { } {
  set earthmap [sb3d::texture "textures/earthmap1k.jpg"]
  $earthmap set name "earthmap"

if {1} {
  set cone [sb3d::shape]
  $cone set cone [list 1.0 2.0]
  $cone set position [list -2 -2 0]
  $cone set color red
  $cone set texture $earthmap
}

if {1} {
  set cube [sb3d::shape]
  $cube set cube [list 1.0 1.0 1.0]
  $cube set position [list 2 -2 0]
  $cube set color green
  $cube set texture $earthmap
}

if {1} {
  set cylinder [sb3d::shape]
  $cylinder set cylinder [list 1.0 2.0]
  $cylinder set position [list -2 2 0]
  $cylinder set color blue
  $cylinder set texture $earthmap
}

if {1} {
  set sphere [sb3d::shape]
  $sphere set sphere [list 1.0]
  $sphere set position [list 2 2 0]
  $sphere set color orange
  $sphere set texture $earthmap
}

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
