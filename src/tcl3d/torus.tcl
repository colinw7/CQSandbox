source "tcl3d/addNormals.tcl"
source "tcl3d/showOrient.tcl"

proc addTorus { r0 r1 p1 p2 {pos {0 0 0}} {c red} {patches 20} {xy 20} } {
  set shape [sb3d::shape]

  $shape set num_patches $patches
  $shape set num_xy      $xy

  $shape set torus [list $r0 $r1 $p1 $p2]

  $shape set color $c

  $shape set position $pos

  return $shape
}

proc init { } {
  set ::radius1 2
  set ::radius2 0.5

  set ::power1 1
  set ::power2 1

  # 1 round, 0 square (ring shape)
  set ::min_power1 0.1
  set ::max_power1 2
  set ::dlt_power1 0.01

  # 1 round, 0 square (tube shape)
  set ::min_power2 0.1
  set ::max_power2 2
  set ::dlt_power2 0.01

  set ::min_patches 5
  set ::max_patches 40
  set ::dlt_patches 1

  set ::min_xy 5
  set ::max_xy 100
  set ::dlt_xy 1

  set ::pos {0 0 0}

  set ::color "#ff0000"

  set ::num_patches 15
  set ::num_xy      40

  set ::torus1 [addTorus $::radius1 $::radius2 $::power1 $::power2 $::pos \
                         $::color $::num_patches $::num_xy]

  sb3d::ui create "\
<qxml>\n\
<QVBoxLayout>
<QPushButton text=\"Normals\" onClicked=\"normalsProc\"/>\n\
<QPushButton text=\"Orientation\" onClicked=\"orientSlot\"/>\n\
</QVBoxLayout>
<QLayoutItem stretch=\"1\"/>\n\
</qxml>"
}

proc tick { } {
if {0} {
  set ::power1 [expr {$::power1 + $::dlt_power1}]

  if {$::dlt_power1 > 0} {
    if {$::power1 > $::max_power1} {
      set ::power1 $::max_power1
      set ::dlt_power1 [expr {-$::dlt_power1}]
    }
  } else {
    if {$::power1 < $::min_power1} {
      set ::power1 $::min_power1
      set ::dlt_power1 [expr {-$::dlt_power1}]
    }
  }
}

if {0} {
  set ::power2 [expr {$::power2 + $::dlt_power2}]

  if {$::dlt_power2 > 0} {
    if {$::power2 > $::max_power2} {
      set ::power2 $::max_power2
      set ::dlt_power2 [expr {-$::dlt_power2}]
    }
  } else {
    if {$::power2 < $::min_power2} {
      set ::power2 $::min_power2
      set ::dlt_power2 [expr {-$::dlt_power2}]
    }
  }
}

if {0} {
  set ::num_patches [expr {$::num_patches + $::dlt_patches}]

  if {$::dlt_patches > 0} {
    if {$::num_patches > $::max_patches} {
      set ::num_patches $::max_patches
      set ::dlt_patches [expr {-$::dlt_patches}]
    }
  } else {
    if {$::num_patches < $::min_patches} {
      set ::num_patches $::min_patches
      set ::dlt_patches [expr {-$::dlt_patches}]
    }
  }
}

  set ::num_xy [expr {$::num_xy + $::dlt_xy}]
  
  if {$::dlt_xy > 0} {
    if {$::num_xy > $::max_xy} {
      set ::num_xy $::max_xy
      set ::dlt_xy [expr {-$::dlt_xy}]
    } 
  } else {
    if {$::num_xy < $::min_xy} {
      set ::num_xy $::min_xy
      set ::dlt_xy [expr {-$::dlt_xy}]
    } 
  }

  echo "$::num_patches $::num_xy"
  $::torus1 set num_patches $::num_patches
  $::torus1 set num_xy      $::num_xy

  echo "$::radius1 $::radius2 $::power1 $::power2"
  $::torus1 set torus [list $::radius1 $::radius2 $::power1 $::power2]
}

proc normalsProc { } {
  addNormals
} 

proc orientSlot { } {
  showOrient
} 
