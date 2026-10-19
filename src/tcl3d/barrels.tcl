proc randIn { min max } {
  return [expr {rand()*($max - $min) + $min}]
}

proc loadRefModel { name } {
  set obj [loadModel "${::model_dir}/${name}.obj" "${name}_ref"  ]
  return $obj
}

proc loadModel { filename name { s 1.0 } } {
  # echo "$filename $name"

  set obj [sb3d::model $filename]

  $obj set id      $name
  $obj set visible 0

  if {$s != 1.0} {
    $obj set scale $s
  }

  $obj set id $name

  # echo [$obj get anim.names]

  return $obj
}

proc loadBarrel { } {
  if {! [info exists ::barrelRefObj]} {
    setModelDir "tcl3d/Dungeon_Assets/obj"

    set ::barrelRefObj [loadRefModel "barrel_large"]
  }

  # ---

  set ::max_barrels 1000

  for {set ib 0} {$ib < $::max_barrels} {incr ib} {
    set ::barrel_obj($ib) [$::barrelRefObj get ref_object]

    set x [randIn -20 20]
    set y [randIn -20 20]
    set z [randIn 0 -20]

    $::barrel_obj($ib) set position [list $x $y $z]

    set xa [randIn 0 1]
    set ya [randIn 0 1]
    set za [randIn 0 1]

    set ::barrel_angles($ib) [list $xa $ya $za]
  }
}

proc setModelDir { dir } {
  set ::model_dir $dir

  sb3d::canvas set model_dir $::model_dir
}

proc init { } {
  loadBarrel
}

proc tick { } {
  for {set ib 0} {$ib < $::max_barrels} {incr ib} {
    set da $::barrel_angles($ib)

    set angles [$::barrel_obj($ib) get angles]

    set xa [lindex $angles 0]
    set ya [lindex $angles 1]
    set za [lindex $angles 2]

    set xa [expr {$xa + [lindex $da 0]}]
    set ya [expr {$ya + [lindex $da 1]}]
    set za [expr {$za + [lindex $da 2]}]

    $::barrel_obj($ib) set angles [list $xa $ya $za]
  }
}
