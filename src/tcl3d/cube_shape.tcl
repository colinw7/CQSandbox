source "tcl3d/addNormalList.tcl"
source "tcl3d/showOrientList.tcl"

proc init { } {
  set cube [sb3d::shape]
  $cube set cube [list 1.0 1.0 1.0]
  $cube set color green

  sb3d::ui create "\
<qxml>\n\
<QVBoxLayout>
<QPushButton text=\"Normals\" onClicked=\"normalsProc\"/>\n\
<QPushButton text=\"Orientation\" onClicked=\"orientSlot\"/>\n\
</QVBoxLayout>
<QLayoutItem stretch=\"1\"/>\n\
</qxml>"

  set ::orient  0
  set ::normals 0
}

proc normalsProc { args } {
  set ::normals [expr {1 - $::normals}]

  if {$::normals} {
    addNormals
  } else {
    removeNormals
  }
}

proc orientSlot { args } {
  set ::orient [expr {1 - $::orient}]

  if {$::orient} {
    showOrient
  } else {
    hideOrient
  }
}
