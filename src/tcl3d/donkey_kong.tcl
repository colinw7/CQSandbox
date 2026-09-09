proc loadModel { filename name { s 1.0 } } {
  # echo "$filename $name"

  set obj [sb3d::model $filename]

  $obj set id      $name
  $obj set visible 0

  if {$s != 1.0} {
    $obj set scale $s
  }

  return $obj
}

proc loadRefModel { name } {
  set obj [loadModel "${::model_dir}/${name}.obj" "${name}_ref"  ]
  return $obj
}

proc loadFloor { } {
  if {! [info exists ::floorRefObj]} {
    setModelDir "tcl3d/Dungeon_Assets/obj"

    set ::floorRefObj [loadRefModel "floor_wood_large"]
  }
}

proc loadBarrel { } {
  if {! [info exists ::barrelRefObj]} {
    setModelDir "tcl3d/Dungeon_Assets/obj"

    set ::barrelRefObj [loadRefModel "barrel_large"]
  }
}

proc setModelDir { dir } {
  set ::model_dir $dir
  
  sb3d::canvas set model_dir $::model_dir
} 

proc init { } {
  loadFloor

  loadBarrel

  set ::barrelPath [sb3d::path]

  $::barrelPath set visible 1
  $::barrelPath set color   green

  set dir 1

  set dx  4.0
  set dx1 [expr {$dx + 0.2}]
  set dy  4.1
  set dy1 0.4

  set z -8

  set nx 5
  set ny 5

  set w  [expr {$nx*$dx1}]
  set w2 [expr {$w/2.0}]

  set y 0

  for {set iy 0} {$iy < $ny} {incr iy} {
    set x [expr {-$dir*$w2}]

    for {set ix 0} {$ix < $nx} {incr ix} {
      set ::floor($ix,$iy) [$::floorRefObj get ref_object]

      set x1 [expr {$x + $dir*$dx/2}]

      $::floor($ix,$iy) set position [list $x1 $y $z]

      $::floor($ix,$iy) set visible 1

      set y1 [expr {$y + 1.0}]

      if {$ix == 0 && $iy == 0} {
        $::barrelPath exec moveTo [list $x1 $y1 $z]
      } else {
        $::barrelPath exec lineTo [list $x1 $y1 $z]
      }

      set x [expr {$x + $dir*$dx1}]
      set y [expr {$y + $dy1}]
    }

    set y [expr {$y + $dy}]

    set dir [expr {-1*$dir}]
  }

  set nb 1

  for {set ib 0} {$ib < $nb} {incr ib} {
    set ::barrel($ib) [$::barrelRefObj get ref_object]

    $::barrel($ib) set position [list 8 27 -9]
    $::barrel($ib) set angles   [list 90 0 0]

    $::barrel($ib) set visible 1
  }

  set ::barrel_t  0.0
  set ::barrel_dt 0.0001

  set ::barrel_a   0.0
  set ::barrel_da  0.5
  set ::barrel_pos {0 0 0}

  sb3d::canvas set camera.type first_person

  sb3d::camera set pitch    -90
  sb3d::camera set origin   {0 0 0}
  sb3d::camera set distance 18

  sb3d::canvas set loop.enabled 1
}

proc bboxChanged { } {
  sb3d::light exec reset 1
}

proc tick { } {
  set ib 0

  if {$::barrel_t > 1.0} { set ::barrel_t 0.0 }
  if {$::barrel_a > 360.0} { set ::barrel_a 0.0 }

  set t1 [expr {1.0 - $::barrel_t}]

  set old_pos $::barrel_pos

  set ::barrel_pos [$::barrelPath get tpos $t1]

  $::barrel($ib) set position $::barrel_pos
  $::barrel($ib) set angles   [list 90 $::barrel_a 0]

  set ::barrel_t [expr {$::barrel_t + $::barrel_dt}]

  if {[lindex $old_pos 0] > [lindex $::barrel_pos 0]} {
    set ::barrel_a [expr {$::barrel_a + $::barrel_da}]
  } else {
    set ::barrel_a [expr {$::barrel_a - $::barrel_da}]
  }
}
