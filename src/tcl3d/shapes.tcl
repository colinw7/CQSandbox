source "tcl3d/addNormals.tcl"
source "tcl3d/showOrient.tcl"

proc init { } {
  set cone [sb3d::shape]
  $cone set cone [list 1.0 2.0]
  $cone set position [list -2 -2 0]
  $cone set color red

  set cube [sb3d::shape]
  $cube set cube [list 1.0 1.0 1.0]
  $cube set position [list 2 -2 0]
  $cube set color green

  set cylinder [sb3d::shape]
  $cylinder set cylinder [list 1.0 2.0]
  $cylinder set position [list -2 2 0]
  $cylinder set color blue

  set sphere [sb3d::shape]
  $sphere set sphere [list 1.0]
  $sphere set position [list 2 2 0]
  $sphere set color orange

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
