proc randIn { min max } {
  return [expr {rand()*($max - $min) + $min}]
}

proc mapReal { r min1 max1 min2 max2 } {
  return [expr {($max2 - $min2)*($r - $min1)/($max1 - $min1) + $min2}]
}

proc distance { p1 p2 } {
  set dx [expr {[lindex $p1 0] - [lindex $p2 0]}]
  set dy [expr {[lindex $p1 1] - [lindex $p2 1]}]
  set dz [expr {[lindex $p1 2] - [lindex $p2 2]}]

  return [expr {sqrt($dx*$dx + $dy*$dy + $dz*$dz)}]
}

proc init { } {
  sb3d::canvas set model_dir hanoi
  set ::hanoi_obj [sb3d::model hanoi/hanoi_block.obj]
  $::hanoi_obj set visible 0
  # echo "$::hanoi_obj"

  set ::column_dy 0.55

# set ::num_blocks 8
  set ::num_blocks 7

  set y 0

  for {set i 0} {$i < $::num_blocks} {incr i} {
    set i1 [expr {$i + 1}]
    set bname "Block$i1"
    set ::block_obj($i) [$::hanoi_obj get child_object $bname]

    $::block_obj($i) set id "block $i"

    $::block_obj($i) exec translate [list 0 $y 0] apply

    set y [expr {$y - $::column_dy}]

    set ::block_anim_pos($i) [sb3d::anim_point]
  }

  set ::base_obj [$::hanoi_obj get child_object Base]
  $::base_obj set id "base"

  for {set i 0} {$i < 3} {incr i} {
    set i1 [expr {$i + 1}]
    set cname "Column$i1"
    set ::column_obj($i) [$::hanoi_obj get child_object $cname]

    $::column_obj($i) set id "column $i"
  }

  # ---

  set ::planeObj [sb3d::plane]

  $::planeObj set position [list 0 2.5 0]
  $::planeObj set scales   [list 10 5 1]
  $::planeObj set color    [list 0 0 1 0.1]
  $::planeObj set visible 1

  # ---

  sb3d::canvas set camera.type first_person

  sb3d::camera set pitch    -90
  sb3d::camera set distance 10
  sb3d::camera set position {0 5 10}

  # ---

  # set ::point_list [sb3d::point_list]
  # $::point_list set point.size 1
  # $::point_list set visible 0

  # ---

  set ::placed 0

  set ::solveList {}

  sb3d::canvas set mode game
}

proc bboxChanged { } {
  if {$::placed == 0} {
    sb3d::light exec reset 1

    resetBlocks

    set ::placed 1
  }
}

proc resetBlocks { } {
  for {set i 0} {$i < $::num_blocks} {incr i} {
    moveBlockToColumn $i 0
  }

  updateColumnBlocks

  positionColumnBlocks 0
}

proc tick { } {
  set na 0

  for {set i 0} {$i < $::num_blocks} {incr i} {
    if {[$::block_anim_pos($i) get can_step]} {
      $::block_anim_pos($i) exec step

      set pos [$::block_anim_pos($i) get value]
      # echo "Move block $i to $pos"

      $::block_obj($i) set position $pos

      incr na
    }
  }

  if {$na > 0} {
    return
  }

  set ns [llength $::solveList]

  if {$ns > 0} {
    # echo "$ns Solves"

    set solveList1 $::solveList
    set ::solveList {}

    set first 1

    foreach solve $solveList1 {
      if {$first} {
        # echo "Anim Move Block $solve"
        moveBlock {*}$solve
        set first 0
      } else {
        lappend ::solveList $solve
      }
    }
  }
}

proc moveBlockToColumn { blockNum columnNum } {
  # echo "moveBlockToColumn block:$blockNum column:$columnNum"

  set ::block_column($blockNum) $columnNum
}

proc updateColumnBlocks { } {
  for {set i 0} {$i < 3} {incr i} {
    for {set j 0} {$j < $::num_blocks} {incr j} {
      set ::column_blocks($i,$j) ""
    }
  }

  for {set i 0} {$i < $::num_blocks} {incr i} {
    set columnNum $::block_column($i)

    lappend ::column_blocks($columnNum,$i) $i
  }
}

proc positionColumnBlocks { anim } {
  # echo "positionColumnBlocks $anim"

  for {set i 0} {$i < 3} {incr i} {
    set c [$::column_obj($i) get bbox.center]

    set x [lindex $c 0]
    set y 0
    set z [lindex $c 2]

    for {set j 0} {$j < $::num_blocks} {incr j} {
      if {$::column_blocks($i,$j) != ""} {
        set oldPos [$::block_obj($j) get position]
        set newPos [list $x $y $z]

        if {$oldPos != $newPos} {
          # echo "block $j @ $newPos"

          if {! $anim} {
            $::block_anim_pos($j) exec reset

            $::block_obj($j) set position $newPos
          } else {
            # echo "Anim Move block $j from $oldPos to $newPos"

            $::block_anim_pos($j) set value $oldPos
            $::block_anim_pos($j) set target $newPos
            $::block_anim_pos($j) set steps 5
            $::block_anim_pos($j) set style one_shot

            sb3d::canvas set loop.enabled 1
          }
        }

        set y [expr {$y + $::column_dy}]
      }
    }
  }
}

proc gamePress { x y obj } {
  # echo "gamePress $x $y $obj"

  # $::point_list set visible 1
  # updatePoints $x $y

  #---

  set ::planeObjPos [planeObjPosition $x $y]
  # echo "planeObjPosition: $::planeObjPos"

  if {$obj != ""} {
    set ::pressColumn [objectColumn $obj]
  } else {
    set ::pressColumn ""
  }

  if {$::pressColumn != ""} {
    set ::pressBlock [topBlock $::pressColumn]
  } else {
    set ::pressBlock ""
  }

  if {$::pressBlock != ""} {
    # set c [$::block_obj($::pressBlock) get bbox.center]
    # set pc [sb3d::canvas get point.project $c]
    # set pv [sb3d::canvas get point.view    $c]
    # echo "Block $::pressBlock : $c (Project: $pc) (View: $pv)"

if {0} {
    set pv1 [list $x $y 0.1]
    set pv2 [list $x $y 10]
    set pi1 [sb3d::canvas get point.unproject $pv1]
    set pi2 [sb3d::canvas get point.unproject $pv2]
    echo "Unproject $pv1 -> $pi1"
    echo "Unproject $pv2 -> $pi2"

    set pv1 [list $x $y 0.1]
    set pv2 [list $x $y 10]
    set pi1 [sb3d::canvas get point.unview $pv1]
    set pi2 [sb3d::canvas get point.unview $pv2]
    echo "Unview $pv1 -> $pi1"
    echo "Unview $pv2 -> $pi2"
}

if {0} {
    set pv [list $x $y 0.99]
    set pi1 [sb3d::canvas get point.unproject $pv]
    set pi2 [sb3d::canvas get point.unview $pv]
    echo "Unproject $pv -> $pi1"
    echo "Unview $pv -> $pi2"
}
  }

if {0} {
  for {set i 0} {$i < 3} {incr i} {
    set c [$::column_obj($i) get bbox.center]

    echo "Column $i : [sb3d::canvas get point.project $c]"
  }
}
}

proc gameMove { x y } {
  # echo "gameMove $x $y"

  # updatePoints $x $y

  set ::planeObjPos [planeObjPosition $x $y]
  # echo "planeObjPosition: $::planeObjPos"

  # ---

  if {$::pressBlock != ""} {
    # $::block_obj($::pressBlock) set position $::mouse_pos
    $::block_obj($::pressBlock) set position $::planeObjPos
  }
}

proc gameRelease { x y obj } {
  # echo "gameRelease $x $y $obj"

  # $::point_list set visible 0

  # ---

  if {$::pressBlock != ""} {
    set minDist   0
    set minColumn -1

    for {set i 0} {$i < 3} {incr i} {
      set c [$::column_obj($i) get bbox.center]

      set d [distance $::planeObjPos $c]

      if {$minColumn < 0 || $d < $minDist} {
        set minColumn $i
        set minDist   $d
      }
    }

    set ::releaseColumn $minColumn

    #set ::releaseColumn [objectColumn $obj]
  } else {
    set ::releaseColumn ""
  }

  if {$::pressColumn != "" && $::releaseColumn != "" && $::pressColumn != $::releaseColumn} {
    moveBlock $::pressColumn $::releaseColumn

    positionColumnBlocks 0
  } else {
    positionColumnBlocks 0
  }
}

proc planeObjPosition { x y } {
  set pmin [$::planeObj get bbox.min]
  set pmax [$::planeObj get bbox.max]

  set ppmin [sb3d::canvas get point.project $pmin]
  set ppmax [sb3d::canvas get point.project $pmax]

  # echo "$pmin $ppmin"
  # echo "$pmax $ppmax"

  set x [mapReal $x [lindex $ppmin 0] [lindex $ppmax 0] [lindex $pmin 0] [lindex $pmax 0]]
  set y [mapReal $y [lindex $ppmin 1] [lindex $ppmax 1] [lindex $pmin 1] [lindex $pmax 1]]
  set z [mapReal $x [lindex $ppmin 0] [lindex $ppmax 0] [lindex $pmin 2] [lindex $pmax 2]]

  return [list $x $y $z]
}

proc updatePoints { x y } {
  #echo "updatePoints $x $y"

  set n 100

  # $::point_list set list.size $n

# set xmin  10000
# set zmin  10000
# set xmax -10000
# set zmax -10000

  for {set i 0} {$i < $n} {incr i} {
    set z [randIn 0.999 0.99999]
  # set z 0.999999

    set p [list $x $y $z]

#   set p1 [sb3d::canvas get point.unproject $p]
    set p2 [sb3d::canvas get point.unview $p]

    # $::point_list set point.position $p2 $i
    # $::point_list set point.color green $i

#   set x1 [lindex $p2 0]
#   set z1 [lindex $p2 2]

#   if {$x1 < $xmin} { set xmin $x1; set zmin $z }
#   if {$x1 > $xmax} { set xmax $x1; set zmax $z }
  }

# echo "XMin: $xmin, ZMin: $zmin"
# echo "XMax: $xmax, ZMax: $zmax"

  sb3d::canvas exec update

  set z 0.99999
  set p [list $x $y $z]
  set ::mouse_pos [sb3d::canvas get point.unview $p]
}

proc keyPress { k } {
  if       {$k == "1"} {
    moveBlock 0 1
  } elseif {$k == "2"} {
    moveBlock 1 2
  } elseif {$k == "3"} {
    moveBlock 2 0
  } elseif {$k == "r"} {
    resetBlocks
  } elseif {$k == "s"} {
    solve
  }
}

proc objectColumn { obj } {
  set type [lindex [$obj get id] 0]

  if       {$type == "block"} {
    set n [lindex [$obj get id] 1]
    return $::block_column($n)
  } elseif {$type == "column"} {
    set n [lindex [$obj get id] 1]
    return $n
  }
  return ""
}

proc moveBlock { fromColumn toColumn { update 1 } } {
  # echo "moveBlock $fromColumn $toColumn"

  set block1 [topBlock $fromColumn]

  if {$block1 == ""} {
    echo "No Top Block"
    return
  }

  set block2 [topBlock $toColumn]
  if {$block2 != ""} {
    #echo "$block1 $block2"

    if {$block1 < $block2} {
      echo "Destination Block Larger than Source"
      return
    }
  }

  moveBlockToColumn $block1 $toColumn

  updateColumnBlocks

  if {$update} {
    positionColumnBlocks 1
  }
}

proc numColumnBlocks { column } {
  set n 0

  for {set i 0} {$i < $::num_blocks} {incr i} {
    if {$::column_blocks($column,$i) != ""} {
      incr n
    }
  }

  return $n
}

proc topBlock { column } {
  set block ""

  for {set i 0} {$i < $::num_blocks} {incr i} {
    if {$::column_blocks($column,$i) != ""} {
      set block $i
    }
  }

  return $block
}

proc bottomBlock { column } {
  for {set i 0} {$i < $::num_blocks} {incr i} {
    if {$::column_blocks($column,$i) != ""} {
      return $i
    }
  }

  return ""
}

proc solve { } {
  # echo "solve"

  set column1 0
  set column2 2

  set ::solveDepth 0

  set block [bottomBlock $column1]
  if {$block == ""} { return }

  set ::solveList {}

  for {set i 0} {$i < $::num_blocks} {incr i} {
    set save_block_column($i) $::block_column($i)
  }

  # move bottom block on column 0 to column 1
  solve1 $block $column1 $column2 $::solveDepth

  for {set i 0} {$i < $::num_blocks} {incr i} {
    set ::block_column($i) $save_block_column($i)
  }

  updateColumnBlocks

  if {0} {
  echo "solved"
  foreach solve $::solveList {
    echo "  moveBlock $solve"
  }
  }
}

proc solve1 { block column1 column2 depth } {
  # echo "solve1 $block $column1 $column2 $depth"

  if {[topBlock $column1] == $block} {
    moveBlock $column1 $column2 0
    lappend ::solveList [list $column1 $column2]
    return
  }

  if {$depth > 8} {
    echo "TOO DEEP"
    return
  }

  set depth1 [expr {$depth + 1}]
  set block1 [expr {$block + 1}]

  set column3 [otherColumn $column1 $column2]

  solve1 $block1 $column1 $column3 $depth1

  moveBlock $column1 $column2 0
  lappend ::solveList [list $column1 $column2]

  solve1 $block1 $column3 $column2 $depth1
}

proc otherColumn { column1 column2 } {
  if {$column1 > $column2} {
    return [otherColumn $column2 $column1]
  }

  if       {$column1 == 0} {
    if {$column2 == 1} {
      return 2
    } else {
      return 1
    }
  } elseif {$column1 == 1} {
    return 0
  } else {
    return -1
  }
}
