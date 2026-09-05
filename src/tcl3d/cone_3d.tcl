source "tcl3d/addNormals.tcl"
source "tcl3d/showOrient.tcl"

proc init { } {
  set ::shape [sb3d::shape]

  $::shape set cone [list 0.2 0.5]

# $::shape set wireframe 1

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
