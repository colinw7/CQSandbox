proc genRect { x y w h } {
  return [list $x $y [expr {$x + $w}] [expr $y + $h]]
}

proc randUnit { } {
  set r [expr {rand()}]
  if {$r < 0.5} { return -1 }
  return 1
}

proc addRefImage { x y filename } {
  set image [sb::image [list $x $y] $filename]
  $image set visible 0
  return $image
}

proc addSubImage { image rect } {
  set subImage [$image get sub_image $rect]
  $subImage set visible 0
  return $subImage
}

proc init { } {
  set size   300; # template images are 300x300
  set border 50 ; # image has a 50 pixel border

  set x 0
  set y 0

  set ::si_image [addRefImage $x $y "jigsaw/jigsaw_template_solid_in.png"  ]; incr x $size
  set ::so_image [addRefImage $x $y "jigsaw/jigsaw_template_solid_out.png" ]; incr x $size
  set ::sn_image [addRefImage $x $y "jigsaw/jigsaw_template_solid_none.png"]; incr x $size

  set ::bi_image [addRefImage $x $y "jigsaw/jigsaw_template_border_in.png"  ]; incr x $size
  set ::bo_image [addRefImage $x $y "jigsaw/jigsaw_template_border_out.png" ]; incr x $size
  set ::bn_image [addRefImage $x $y "jigsaw/jigsaw_template_border_none.png"]; incr x $size

  set ::w $size
  set ::h $size

  set ::bx $border
  set ::by $border

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

  set piece1 [getSolidJigsawPiece -1 1 -1 1]
  set piece2 [getSolidJigsawPiece 1 -1 1 -1]

  set y 1024

  $piece1 set position [list   0 $y]
  $piece2 set position [list 300 $y]

  genPuzzle 5 5 1024 1024
}

proc genPuzzle { nx ny w h } {
  set dx [expr {int($w/$nx)}]
  set dy [expr {int($h/$ny)}]

  for {set ix 0} {$ix < $nx} {incr ix} {
    set last_b($ix) 0
  }

  set y1 0
  set y2 0

  for {set iy 0} {$iy < $ny} {incr iy} {
    set last_r 0

    set y2 [expr {$y1 + $dy}]

    set x1 0
    set x2 0

    for {set ix 0} {$ix < $nx} {incr ix} {
      set ind [expr {$iy*$nx + $ix}]

      set x2 [expr {$x1 + $dx}]

      #---

      set l [expr {-$last_r}]
      set r 0
      set t [expr {-$last_b($ix)}]
      set b 0

      if {$ix < $nx - 1} {
        set r [randUnit]
      }

      if {$iy < $ny - 1} {
        set b [randUnit]
      }

      set piece [getBorderJigsawPiece $l $r $t $b]

      set ::puzzle_grid_image($ind) [sb::image]

      $::puzzle_grid_image($ind) set image $piece

      $::puzzle_grid_image($ind) set position [list $x1 $y1]

      $::puzzle_grid_image($ind) exec resize [list $dx $dy]

      set x1 $x2

      set last_r      $r
      set last_b($ix) $b
    }

    set y1 $y2
  }
}

proc getSolidJigsawPiece { l r t b } {
  set ind [expr {3*(3*(3*$l + $r) + $t) + $b}]

  if {! [info exists ::jigsaw_piece($ind)]} {
    set ::jigsaw_piece($ind) [sb::image]

    $::jigsaw_piece($ind) set size [list $::w $::h]

    set x1 [expr {$::bx}]
    set y1 [expr {$::by}]
    set x2 [expr {$::w - $::bx}]
    set y2 [expr {$::h - $::by}]

    $::jigsaw_piece($ind) exec fill.rect [list $x1 $y1 $x2 $y2] black

    set x 0
    set y 0

    if       {$l < 0} {
      $::jigsaw_piece($ind) set sub_image [list $x $y] $::lsi_image
    } elseif {$l > 0} {
      $::jigsaw_piece($ind) set sub_image [list $x $y] $::lso_image
    }

    set x [expr {$::w - 2*$::bx}]
    set y 0

    if       {$r < 0} {
      $::jigsaw_piece($ind) set sub_image [list $x $y] $::rsi_image
    } elseif {$r > 0} {
      $::jigsaw_piece($ind) set sub_image [list $x $y] $::rso_image
    }

    set x 0
    set y 0

    if       {$t < 0} {
      $::jigsaw_piece($ind) set sub_image [list $x $y] $::tsi_image
    } elseif {$t > 0} {
      $::jigsaw_piece($ind) set sub_image [list $x $y] $::tso_image
    }

    set x 0
    set y [expr {$::h - 2*$::by}]

    if       {$b < 0} {
      $::jigsaw_piece($ind) set sub_image [list $x $y] $::bsi_image
    } elseif {$b > 0} {
      $::jigsaw_piece($ind) set sub_image [list $x $y] $::bso_image
    }

    $::jigsaw_piece($ind) exec stroke.rect blue
  }

  return $::jigsaw_piece($ind)
}

proc getBorderJigsawPiece { l r t b } {
  set ind [expr {3*(3*(3*$l + $r) + $t) + $b}]

  if {! [info exists ::jigsaw_piece($ind)]} {
    set ::jigsaw_piece($ind) [sb::image]

    $::jigsaw_piece($ind) set size [list $::w $::h]

    set x1 [expr {$::bx}]
    set y1 [expr {$::by}]
    set x2 [expr {$::w - $::bx}]
    set y2 [expr {$::h - $::by}]

    $::jigsaw_piece($ind) exec stroke.rect [list $x1 $y1 $x2 $y2] black

    set x 0
    set y 0

    if       {$l < 0} {
      $::jigsaw_piece($ind) set sub_image [list $x $y] $::lbi_image
    } elseif {$l > 0} {
      $::jigsaw_piece($ind) set sub_image [list $x $y] $::lbo_image
    }

    set x [expr {$::w - 2*$::bx}]
    set y 0

    if       {$r < 0} {
      $::jigsaw_piece($ind) set sub_image [list $x $y] $::rbi_image
    } elseif {$r > 0} {
      $::jigsaw_piece($ind) set sub_image [list $x $y] $::rbo_image
    }

    set x 0
    set y 0

    if       {$t < 0} {
      $::jigsaw_piece($ind) set sub_image [list $x $y] $::tbi_image
    } elseif {$t > 0} {
      $::jigsaw_piece($ind) set sub_image [list $x $y] $::tbo_image
    }

    set x 0
    set y [expr {$::h - 2*$::by}]

    if       {$b < 0} {
      $::jigsaw_piece($ind) set sub_image [list $x $y] $::bbi_image
    } elseif {$b > 0} {
      $::jigsaw_piece($ind) set sub_image [list $x $y] $::bbo_image
    }

    $::jigsaw_piece($ind) exec stroke.rect blue
  }

  return $::jigsaw_piece($ind)
}
