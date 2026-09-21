# TODO: draw order (layer)

# layer 0 : background (puzzle grid)
# layer 1 : placed puzzle pieces
# layer 2 : unplaced puzzle pieces

proc genRect { x y w h } {
  return [list $x $y [expr {$x + $w}] [expr $y + $h]]
}

proc irandIn { min max } {
  return [expr {int(rand()*($max - $min) + $min + 0.5)}]
}

proc randUnit { } {
  set r [expr {rand()}]
  if {$r < 0.5} { return -1 }
  return 1
}

proc addRefImage { x y name } {
  set filename "jigsaw/jigsaw_template_${name}.png"
  set image [sb::image [list $x $y] $filename]
  $image set id $name
  $image set visible 0
  return $image
}

proc addSubImage { image rect } {
  set subImage [$image get sub_image $rect]
  $subImage set visible 0
  return $subImage
}

proc init { args } {
  sb::canvas set brush.color "lightsteelblue"

  if {[llength $args] > 0} {
    set ::imageFile [lindex $args 0]
  } else {
    set ::imageFile "jigsaw/Catwoman.jpg"
  }

  set ::solveImage [sb::image {1024 0} $::imageFile]

  $::solveImage set id "solveImage"

  set size [$::solveImage get size]

  set ::solveImageWidth  [lindex $size 0]
  set ::solveImageHeight [lindex $size 1]

  $::solveImage set visible 0

  # ---

  set size   300; # template images are 300x300
  set border 50 ; # image has a 50 pixel border

  set ::w $size
  set ::h $size

  set ::bx $border
  set ::by $border

  set ::nx 5
  set ::ny 5

  set x 0
  set y 0

  set ::si_image [addRefImage $x $y "solid_in"  ]; incr x $size
  set ::so_image [addRefImage $x $y "solid_out" ]; incr x $size
  set ::sn_image [addRefImage $x $y "solid_none"]; incr x $size

  set ::bi_image [addRefImage $x $y "border_in"  ]; incr x $size
  set ::bo_image [addRefImage $x $y "border_out" ]; incr x $size
  set ::bn_image [addRefImage $x $y "border_none"]; incr x $size

  set x1 [expr {                0}]; set y1 [expr {               0}]
  set x2 [expr {            $::bx}]; set y2 [expr {           $::by}]
  set x3 [expr { $::w - 1 - $::bx}]; set y3 [expr {$::h - 1 - $::by}]
  set x4 [expr { $::w - 1        }]; set y4 [expr {$::h - 1        }]

  set bx2 [expr {2*$::bx}]
  set by2 [expr {2*$::by}]

  set x31 [expr {$x3 - $::bx}]
  set y31 [expr {$y3 - $::by}]

  set ::lso_image [addSubImage $::so_image [genRect $x1  $y1  $bx2 $::h]]
  set ::rso_image [addSubImage $::so_image [genRect $x31 $y1  $bx2 $::h]]
  set ::tso_image [addSubImage $::so_image [genRect $x1  $y1  $::w $by2]]
  set ::bso_image [addSubImage $::so_image [genRect $x1  $y31 $::w $by2]]

  set ::lsi_image [addSubImage $::si_image [genRect $x1  $y1  $bx2 $::h]]
  set ::rsi_image [addSubImage $::si_image [genRect $x31 $y1  $bx2 $::h]]
  set ::tsi_image [addSubImage $::si_image [genRect $x1  $y1  $::w $by2]]
  set ::bsi_image [addSubImage $::si_image [genRect $x1  $y31 $::w $by2]]

  set ::lbo_image [addSubImage $::bo_image [genRect $x1  $y1  $bx2 $::h]]
  set ::rbo_image [addSubImage $::bo_image [genRect $x31 $y1  $bx2 $::h]]
  set ::tbo_image [addSubImage $::bo_image [genRect $x1  $y1  $::w $by2]]
  set ::bbo_image [addSubImage $::bo_image [genRect $x1  $y31 $::w $by2]]

  set ::lbi_image [addSubImage $::bi_image [genRect $x1  $y1  $bx2 $::h]]
  set ::rbi_image [addSubImage $::bi_image [genRect $x31 $y1  $bx2 $::h]]
  set ::tbi_image [addSubImage $::bi_image [genRect $x1  $y1  $::w $by2]]
  set ::bbi_image [addSubImage $::bi_image [genRect $x1  $y31 $::w $by2]]

  set border2 [expr {$border*3}]

  set x 0
  set y [expr {$y + $size}]

  $::lso_image set position [list $x $y]; incr x $border2
  $::rso_image set position [list $x $y]; incr x $border2
  $::tso_image set position [list $x $y]; incr x $size
  $::bso_image set position [list $x $y]; incr x $size

  $::lsi_image set position [list $x $y]; incr x $border2
  $::rsi_image set position [list $x $y]; incr x $border2
  $::tsi_image set position [list $x $y]; incr x $size
  $::bsi_image set position [list $x $y]; incr x $size

  set x 0
  set y [expr {$y + $size}]

  $::lbo_image set position [list $x $y]; incr x $border2
  $::rbo_image set position [list $x $y]; incr x $border2
  $::tbo_image set position [list $x $y]; incr x $size
  $::bbo_image set position [list $x $y]; incr x $size

  $::lbi_image set position [list $x $y]; incr x $border2
  $::rbi_image set position [list $x $y]; incr x $border2
  $::tbi_image set position [list $x $y]; incr x $size
  $::bbi_image set position [list $x $y]; incr x $size

  # set y 800 ; showSolidPieces $y
  # set y 1100; showBorderPieces $y

  genPuzzle
}

proc showSolidPieces { y } {
  set piece1 [getSolidJigsawPiece -1 -1 -1  -1]
  set piece2 [getSolidJigsawPiece  0  0  0   0]
  set piece3 [getSolidJigsawPiece  1  1  1   1]
  
  $piece1 set visible 1
  $piece2 set visible 1
  $piece3 set visible 1
  
  $piece1 set position [list   0 $y]
  $piece2 set position [list 300 $y]
  $piece3 set position [list 600 $y]
}

proc showBorderPieces { y } {
  set piece1 [getBorderJigsawPiece -1 -1 -1  -1]
  set piece2 [getBorderJigsawPiece  0  0  0   0]
  set piece3 [getBorderJigsawPiece  1  1  1   1]

  $piece1 set visible 1
  $piece2 set visible 1
  $piece3 set visible 1

  $piece1 set position [list   0 $y]
  $piece2 set position [list 300 $y]
  $piece3 set position [list 600 $y]
}

proc genPuzzle { } {
  set ::imageGridDx [expr {int($::solveImageWidth/$::nx)}]
  set ::imageGridDy [expr {int($::solveImageHeight/$::ny)}]

  set xscale [expr {(1.0*$::imageGridDx)/($::w - 2*$::bx)}]
  set yscale [expr {(1.0*$::imageGridDy)/($::h - 2*$::by)}]

  set sx [expr {$xscale*$::w}]
  set sy [expr {$yscale*$::h}]

  set ::imageGridBx [expr {$xscale*$::bx}]
  set ::imageGridBy [expr {$yscale*$::by}]

  for {set ix 0} {$ix < $::nx} {incr ix} {
    set last_b($ix) 0
  }

  set y1 8
  set y2 $y1

  for {set iy 0} {$iy < $::ny} {incr iy} {
    set last_r 0

    set y2 [expr {$y1 + $::imageGridDy}]

    set x1 8
    set x2 $x1

    for {set ix 0} {$ix < $::nx} {incr ix} {
      set ind [expr {$iy*$::nx + $ix}]

      set x2 [expr {$x1 + $::imageGridDx}]

      #---

      set xx1 [expr {$x1 - $::imageGridBx}]
      set yy1 [expr {$y1 - $::imageGridBy}]
      set xx2 [expr {$x2 + $::imageGridBx}]
      set yy2 [expr {$y2 + $::imageGridBy}]

      #---

      set l [expr {-$last_r}]
      set r 0
      set t [expr {-$last_b($ix)}]
      set b 0

      if {$ix < $::nx - 1} {
        set r [randUnit]
      }

      if {$iy < $::ny - 1} {
        set b [randUnit]
      }

      #---

      set ::puzzle_grid_x($ind) $xx1
      set ::puzzle_grid_y($ind) $yy1

      #---

      set borderPiece [getBorderJigsawPiece $l $r $t $b]

      set ::puzzle_grid_image($ind) [sb::image]

      $::puzzle_grid_image($ind) set id "puzzle_grid_image($ind)"

      $::puzzle_grid_image($ind) set image $borderPiece

      $::puzzle_grid_image($ind) set position [list $xx1 $yy1]

      $::puzzle_grid_image($ind) exec resize [list $sx $sy]

      $::puzzle_grid_image($ind) set layer   0
      $::puzzle_grid_image($ind) set visible 1

      #---

      set solidPiece [getSolidJigsawPiece $l $r $t $b] ; # mask

      set ::puzzle_piece_mask($ind) [sb::image]
  
      $::puzzle_piece_mask($ind) set id "puzzle_piece_mask($ind)"
  
      $::puzzle_piece_mask($ind) set image $solidPiece

      $::puzzle_piece_mask($ind) exec resize [list $sx $sy]

      $::puzzle_piece_mask($ind) set visible 0

      set ::puzzle_piece_image($ind) [$::solveImage get sub_image [list $xx1 $yy1 $xx2 $yy2]]

      $::puzzle_piece_image($ind) set id "puzzle_piece_image($ind)"

      $::puzzle_piece_image($ind) set position [list $xx1 $yy1]

      $::puzzle_piece_image($ind) exec resize [list $sx $sy]

      $::puzzle_piece_image($ind) set image_mask $::puzzle_piece_mask($ind)

      $::puzzle_piece_image($ind) set layer   2
      $::puzzle_piece_image($ind) set visible 1

      #---

      set x1 $x2

      set last_r      $r
      set last_b($ix) $b
    }

    set y1 $y2
  }

  shufflePieces
}

proc shufflePieces { } {
  set x1 1024
  set x2 [expr {$x1 + $::solveImageWidth}]
  set y1 0
  set y2 [expr {$y1 + $::solveImageHeight}]

  for {set iy 0} {$iy < $::ny} {incr iy} {
    for {set ix 0} {$ix < $::nx} {incr ix} {
      set ind [expr {$iy*$::nx + $ix}]

      set x [irandIn $x1 $x2]
      set y [irandIn $y1 $y2]

      $::puzzle_piece_image($ind) set position [list $x $y]

      set ::puzzle_piece_solved($ind) 0
    }
  }
}

proc getSolidJigsawPiece { l r t b } {
  set ind [getPieceIndex $l $r $t $b]

  if {! [info exists ::solid_jigsaw_piece($ind)]} {
    set ::solid_jigsaw_piece($ind) [sb::image]

    $::solid_jigsaw_piece($ind) set id "solid_jigsaw_piece($ind)"

    $::solid_jigsaw_piece($ind) set visible 0

    $::solid_jigsaw_piece($ind) set size [list $::w $::h]

    $::solid_jigsaw_piece($ind) exec fill.rect transparent

    set x1 [expr {2*$::bx}]
    set y1 [expr {2*$::by}]
    set x2 [expr {$::w - 2*$::bx}]
    set y2 [expr {$::h - 2*$::by}]

    set x 0
    set y 0

    if       {$l < 0} {
      $::solid_jigsaw_piece($ind) set sub_image [list $x $y] $::lsi_image
    } elseif {$l > 0} {
      $::solid_jigsaw_piece($ind) set sub_image [list $x $y] $::lso_image
    } else {
      set x1 [expr {$::bx}]
    }

    set x [expr {$::w - 2*$::bx}]
    set y 0

    if       {$r < 0} {
      $::solid_jigsaw_piece($ind) set sub_image [list $x $y] $::rsi_image
    } elseif {$r > 0} {
      $::solid_jigsaw_piece($ind) set sub_image [list $x $y] $::rso_image
    } else {
      set x2 [expr {$::w - $::bx - 2}]
    }

    set x 0
    set y 0

    if       {$t < 0} {
      $::solid_jigsaw_piece($ind) set sub_image [list $x $y] $::tsi_image
    } elseif {$t > 0} {
      $::solid_jigsaw_piece($ind) set sub_image [list $x $y] $::tso_image
    } else {
      set y1 [expr {$::bx}]
    }

    set x 0
    set y [expr {$::h - 2*$::by}]

    if       {$b < 0} {
      $::solid_jigsaw_piece($ind) set sub_image [list $x $y] $::bsi_image
    } elseif {$b > 0} {
      $::solid_jigsaw_piece($ind) set sub_image [list $x $y] $::bso_image
    } else {
      set y2 [expr {$::h - $::by}]
    }

    $::solid_jigsaw_piece($ind) exec fill.rect [list $x1 $y1 $x2 $y2] black

    # $::solid_jigsaw_piece($ind) exec stroke.rect blue
  }

  return $::solid_jigsaw_piece($ind)
}

proc getBorderJigsawPiece { l r t b } {
  set ind [getPieceIndex $l $r $t $b]

  if {! [info exists ::border_jigsaw_piece($ind)]} {
    set ::border_jigsaw_piece($ind) [sb::image]

    $::border_jigsaw_piece($ind) set id "border_jigsaw_piece($ind)"

    $::border_jigsaw_piece($ind) set visible 0

    $::border_jigsaw_piece($ind) set size [list $::w $::h]

    $::border_jigsaw_piece($ind) exec fill.rect transparent

    set x1 [expr {2*$::bx}]
    set y1 [expr {2*$::by}]
    set x2 [expr {$::w - 2*$::bx}]
    set y2 [expr {$::h - 2*$::by}]

    set x 0
    set y 0

    if       {$l < 0} {
      $::border_jigsaw_piece($ind) set sub_image [list $x $y] $::lbi_image
    } elseif {$l > 0} {
      $::border_jigsaw_piece($ind) set sub_image [list $x $y] $::lbo_image
    } else {
      set x1 [expr {$::bx}]
    }

    set x [expr {$::w - 2*$::bx}]
    set y 0

    if       {$r < 0} {
      $::border_jigsaw_piece($ind) set sub_image [list $x $y] $::rbi_image
    } elseif {$r > 0} {
      $::border_jigsaw_piece($ind) set sub_image [list $x $y] $::rbo_image
    } else {
      set x2 [expr {$::w - $::bx - 2}]
    }

    set x 0
    set y 0

    if       {$t < 0} {
      $::border_jigsaw_piece($ind) set sub_image [list $x $y] $::tbi_image
    } elseif {$t > 0} {
      $::border_jigsaw_piece($ind) set sub_image [list $x $y] $::tbo_image
    } else {
      set y1 [expr {$::bx}]
    }

    set x 0
    set y [expr {$::h - 2*$::by}]

    if       {$b < 0} {
      $::border_jigsaw_piece($ind) set sub_image [list $x $y] $::bbi_image
    } elseif {$b > 0} {
      $::border_jigsaw_piece($ind) set sub_image [list $x $y] $::bbo_image
    } else {
      set y2 [expr {$::h - $::by}]
    }

    $::border_jigsaw_piece($ind) exec fill.rect [list $x1 $y1 $x2 $y2] white

    $::border_jigsaw_piece($ind) set stroke.width 5

    if {$l == 0} {
      $::border_jigsaw_piece($ind) exec stroke.line [list $x1 $y1] [list $x1 $y2] black
    }
    if {$r == 0} {
      $::border_jigsaw_piece($ind) exec stroke.line [list $x2 $y1] [list $x2 $y2] black
    }
    if {$t == 0} {
      $::border_jigsaw_piece($ind) exec stroke.line [list $x1 $y1] [list $x2 $y1] black
    }
    if {$b == 0} {
      $::border_jigsaw_piece($ind) exec stroke.line [list $x1 $y2] [list $x2 $y2] black
    }

    # $::border_jigsaw_piece($ind) exec stroke.rect blue
  }

  return $::border_jigsaw_piece($ind)
}

proc getPieceIndex { l r t b } {
  set l1 [expr {$l + 1}]
  set r1 [expr {$r + 1}]
  set t1 [expr {$t + 1}]
  set b1 [expr {$b + 1}]

  return [expr {3*(3*(3*$l1 + $r1) + $t1) + $b1}]
}

proc mousePress { x y } {
  set minInd -1
  set minD   0

  for {set iy 0} {$iy < $::ny} {incr iy} {
    for {set ix 0} {$ix < $::nx} {incr ix} {
      set ind [expr {$iy*$::nx + $ix}]

      if {$::puzzle_piece_solved($ind)} {
        continue
      }

      set pos [$::puzzle_piece_image($ind) get position]

      set xc [expr {[lindex $pos 0] + $::imageGridDx/2 + $::imageGridBx}]
      set yc [expr {[lindex $pos 1] + $::imageGridDy/2 + $::imageGridBy}]

      set dx [expr {$x - $xc}]
      set dy [expr {$y - $yc}]

      set d [hypot $dx $dy]

      if {$minInd < 0 || $d < $minD} {
        set minInd $ind
        set minD   $d
      }
    }
  }

  if {$minD < $::imageGridDx || $minD < $::imageGridDy} {
    set ::mouseMoveInd $minInd

    set pos [$::puzzle_piece_image($::mouseMoveInd) get position]

    set ::mouseMoveDx [expr {$x - [lindex $pos 0]}]
    set ::mouseMoveDy [expr {$y - [lindex $pos 1]}]
  } else {
    set ::mouseMoveInd -1

    set ::mouseMoveDx 0
    set ::mouseMoveDy 0
  }
}

proc mouseMove { x y } {
  if {$::mouseMoveInd >= 0} {
    set x1 [expr {$x - $::mouseMoveDx}]
    set y1 [expr {$y - $::mouseMoveDy}]

    $::puzzle_piece_image($::mouseMoveInd) set position [list $x1 $y1]
  }
}

proc mouseRelease { x y } {
  if {$::mouseMoveInd >= 0} {
    set pos [$::puzzle_piece_image($::mouseMoveInd) get position]

    set dx [expr {$::puzzle_grid_x($::mouseMoveInd) - [lindex $pos 0]}]
    set dy [expr {$::puzzle_grid_y($::mouseMoveInd) - [lindex $pos 1]}]

    set d [hypot $dx $dy]

    if {$d < $::imageGridDx/4 || $d < $::imageGridDy/4} {
      $::puzzle_piece_image($::mouseMoveInd) set position \
        [list $::puzzle_grid_x($::mouseMoveInd) $::puzzle_grid_y($::mouseMoveInd)]

      $::puzzle_piece_image($::mouseMoveInd) set layer 1

      set ::puzzle_piece_solved($::mouseMoveInd) 1

      if {[checkSolved]} {
        echo "Solved"
      }
    }

    set ::mouseMoveInd -1
  }
}

proc checkSolved { } {
  for {set iy 0} {$iy < $::ny} {incr iy} {
    for {set ix 0} {$ix < $::nx} {incr ix} {
      set ind [expr {$iy*$::nx + $ix}]

      if {! $::puzzle_piece_solved($ind)} {
        return 0
      }
    }
  }

  return 1
}

proc hypot { dx dy } {
  return [expr {sqrt($dx*$dx + $dy*$dy)}]
}
