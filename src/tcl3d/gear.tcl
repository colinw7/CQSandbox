source "tcl3d/addNormals.tcl"
source "tcl3d/showOrient.tcl"

proc addGear { r n ts rot {pos {0 0 0}} {c red} } {
  set shape [sb3d::shape]

  $shape set gear [list $r $n $ts $rot]

  $shape set color $c

  $shape set position $pos

  return $shape
}

proc init { } {
  set ::radius    1
  set ::num_gears 8
  set ::toothSize 0.1
  set ::rot       0

  set ::pos {0 0 0}

  set ::color blue

  set ::gear [addGear $::radius $::num_gears $::toothSize $::rot $::pos $::color]

  sb3d::ui create "\
<qxml>\n\
<QHBoxLayout>\n\
<QLabel text=\"Radius\"/>\n\
<CQTclRealSpin varName=\"radius\" onRealValueChanged=\"updateGear\"/>\n\
</QHBoxLayout>\n\
<QHBoxLayout>\n\
<QLabel text=\"Num Gears\"/>\n\
<CQTclIntegerSpin varName=\"num_gears\" onValueChanged=\"updateGear\"/>\n\
</QHBoxLayout>\n\
<QHBoxLayout>\n\
<QLabel text=\"Tooth Size\"/>\n\
<CQTclRealSpin varName=\"toothSize\" onRealValueChanged=\"updateGear\"/>\n\
</QHBoxLayout>\n\
<QVBoxLayout>\n\
<QPushButton text=\"Normals\" onClicked=\"normalsProc\"/>\n\
<QPushButton text=\"Orientation\" onClicked=\"orientSlot\"/>\n\
</QVBoxLayout>\n\
<QLayoutItem stretch=\"1\"/>\n\
</qxml>"
}

proc tick { } {
}

proc updateGear { args } {
  $::gear set gear [list $::radius $::num_gears $::toothSize $::rot]
}

proc normalsProc { } {
  addNormals
} 

proc orientSlot { } {
  showOrient
} 
