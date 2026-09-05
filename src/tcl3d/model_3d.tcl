source "tcl3d/addNormals.tcl"
source "tcl3d/showOrient.tcl"

proc init { } {
  set ::model [sb3d::model models/v3d/F15.V3D]
  # set ::model [sb3d::model models/3ds/batwing.3ds]
  # set ::model [sb3d::model models/ply/shield.ply]
  # set ::model [sb3d::model models/gltf/Earth.gltf]
  # set ::model [sb3d::model models/gltf/DamagedHelmet.glb]

  # cd /work/colinw/packages/glTF-Sample-Assets-main/Models/ABeautifulGame/glTF
  # set ::model [sb3d::model ABeautifulGame.gltf]

  # $::model set diffuse_texture  models/ply/shield_diffuse.png
  # $::model set specular_texture models/ply/shield_spec.png
  # $::model set normal_texture   models/ply/shield_normal.png

  # sb3d::custom_form string -label "Model" -proc modelProc

  # sb3d::camera set position [list 5 5 19]

  sb3d::ui create "\
<qxml>\n\
<QVBoxLayout>
<QPushButton text=\"Reset\" onClicked=\"resetProc\"/>\n\
<QPushButton text=\"Normals\" onClicked=\"normalsProc\"/>\n\
<QPushButton text=\"Orientation\" onClicked=\"orientSlot\"/>\n\
</QVBoxLayout>
<QLayoutItem stretch=\"1\"/>\n\
</qxml>"
}

proc bboxChanged { } {
  resetProc
}

proc resetProc { } {
  sb3d::camera exec reset
  sb3d::light  exec reset 1
}

proc normalsProc { } {
  addNormals
}

proc orientSlot { } {
  showOrient
}
