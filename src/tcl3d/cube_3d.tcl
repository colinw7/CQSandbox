source "tcl3d/addNormals.tcl"
source "tcl3d/showOrient.tcl"

proc init { } {
  set ::cube [sb3d::cube]

  $::cube set texture "textures/container.jpg"

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

proc cameraChanged { } {
  if {$::orient} {
    showOrient
  }
}

proc normalsProc { } {
  set ::normals [expr {1 - $::normals}]

  if {$::normals} {
    addNormals
  } else {
    removeNormals
  }
}

proc orientSlot { } {
  set ::orient [expr {1 - $::orient}]

  if {$::orient} {
    showOrient
  } else {
    hideOrient
  }
}
