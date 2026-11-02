source "tcl3d/addNormalList.tcl"
source "tcl3d/showOrientList.tcl"

proc init { } {
  # v3d
#   set ::model [sb3d::model models/v3d/F15.V3D]

  # 3ds
#   set ::model [sb3d::model models/3ds/batwing.3ds]

  # ply
#   set ::model [sb3d::model models/ply/shield.ply]
#   $::model set diffuse_texture  models/ply/shield_diffuse.png
#   $::model set specular_texture models/ply/shield_spec.png
#   $::model set normal_texture   models/ply/shield_normal.png

  # gltf
#   set ::model [sb3d::model models/gltf/DamagedHelmet.glb]
#   set ::model [sb3d::model models/gltf/BarramundiFish.glb]
#   set ::model [sb3d::model models/gltf/Earth.gltf]

# cd /work/colinw/packages/glTF-Sample-Assets-main/Models/ABeautifulGame/glTF
# set ::model [sb3d::model ABeautifulGame.gltf]

  # fbx
# set ::model_dir models/fbx
# sb3d::canvas set model_dir $::model_dir
# set ::model [sb3d::model models/fbx/spider.fbx]

  # obj
  set ::model_dir models/obj
  sb3d::canvas set model_dir $::model_dir
# set ::model [sb3d::model models/obj/tommygun.obj]
  set ::model [sb3d::model models/obj/momo.obj]

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

  set ::orient  0
  set ::normals 0
}

proc bboxChanged { } {
  resetProc
}

proc resetProc { args } {
  sb3d::camera exec reset
  sb3d::light  exec reset 1
}

proc cameraChanged { } {
  if {$::orient} {
    showOrient
  }
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
