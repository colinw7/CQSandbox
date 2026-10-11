proc randIn { min max } {
  return [expr {rand()*($max - $min) + $min}]
}

source "tcl3d/addNormalList.tcl"
source "tcl3d/showOrientList.tcl"

proc init { } {
  set cube [sb3d::shape]
  $cube set cube [list 1.0 1.0 1.0]
  $cube set color green

  set faces  [$cube get faces]
  set lines  [$cube get lines]
  set points [$cube get points]

  echo "Faces: $faces"
  echo "Lines: $lines"
  echo "Points: $points"

  foreach face $faces {
    echo "Face $face"
    echo "  Center: [$cube get face.center $face]"
    echo "  Normal: [$cube get face.normal $face]"
    echo "  Orient: [$cube get face.orient $face]"
    echo "  Points: [$cube get face.points $face]"
  }

  foreach line $lines {
    echo "Line $line"
    echo "  [$cube get line.start $line]"
    echo "  [$cube get line.end   $line]"
  }

  foreach point $points {
    echo "Point $point"
    echo "  [$cube get point.position $point]"
  }

if {0} {
  foreach point $points {
    set p [$cube get point.position $point]

    set dx [randIn -0.5 0.5]
    set dy [randIn -0.5 0.5]
    set dz [randIn -0.5 0.5]

    set p1 [list [expr {[lindex $p 0] + $dx}] \
                 [expr {[lindex $p 1] + $dy}] \
                 [expr {[lindex $p 2] + $dz}]]

    $cube set point.position $point $p1
  }
}

if {0} {
  foreach line $lines {
    set p1 [$cube get line.start $line]
    set p2 [$cube get line.end   $line]

    set dx1 [randIn -0.2 0.2]
    set dy1 [randIn -0.2 0.2]
    set dz1 [randIn -0.2 0.2]

    set dx2 [randIn -0.2 0.2]
    set dy2 [randIn -0.2 0.2]
    set dz2 [randIn -0.2 0.2]

    set p3 [list [expr {[lindex $p1 0] + $dx1}] \
                 [expr {[lindex $p1 1] + $dy1}] \
                 [expr {[lindex $p1 2] + $dz1}]]

    set p4 [list [expr {[lindex $p2 0] + $dx2}] \
                 [expr {[lindex $p2 1] + $dy2}] \
                 [expr {[lindex $p2 2] + $dz2}]]

    $cube set line.start $line $p3
    $cube set line.end   $line $p4
  }
}

  foreach face $faces {
    set points [$cube get face.points $face]
    echo "Old: $points"

    set points1 {}

    foreach p $points {
      set dx [randIn -0.1 0.1]
      set dy [randIn -0.1 0.1]
      set dz [randIn -0.1 0.1]

      set p1 [list [expr {[lindex $p 0] + $dx}] \
                   [expr {[lindex $p 1] + $dy}] \
                   [expr {[lindex $p 2] + $dz}]]

      lappend points1 $p1
    }

    echo "New: $points1"
    $cube set face.points $face $points1

    break
  }

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
