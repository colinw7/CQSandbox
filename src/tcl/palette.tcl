proc init { } {
  sb::canvas set window.size [list 512 512]

  set ::pal [sb::color_range]
  # echo [$::pal get interp 0.5]

  $::pal set mode moreland

  set ::renderer [sb::renderer]

  set ::view1 [sb::viewport [list 0.0 0.0 0.5 0.5]]
  set ::view2 [sb::viewport [list 0.5 0.0 1.0 0.5]]
  set ::view3 [sb::viewport [list 0.0 0.5 0.5 1.0]]
  set ::view4 [sb::viewport [list 0.5 0.5 1.0 1.0]]
}

proc drawBg { view } {
  # echo "drawBg $view"

  if       {$view == $::view1} {
    drawPalette rgb_range
  } elseif {$view == $::view2} {
    drawPalette moreland
  } elseif {$view == $::view3} {
    drawPalette plasma
  }
}

proc drawPalette { palette } {
  $::pal set mode $palette

  set pixelRange [sb::canvas get pixel_range]

  set x1 [lindex $pixelRange 0]
  set y1 [lindex $pixelRange 1]
  set x2 [lindex $pixelRange 2]
  set y2 [lindex $pixelRange 3]
  # echo "$x1 $y1 $x2 $y2"

  set w [expr {$x2 - $x1}]
  set h [expr {$y2 - $y1}]

  for {set iy 0} {$iy < $h} {incr iy} {
    set y [expr {$iy/($h - 1.0)}]

    set cy [$::pal get interp $y]

    for {set ix 0} {$ix < $w} {incr ix} {
      set x [expr {$ix/($w - 1.0)}]

      set cx [$::pal get interp $x]

      $::renderer set pen.color $cx

      set ix1 [expr {$ix + $x1}]
      set iy1 [expr {$iy + $y1}]

      $::renderer exec draw.point [list $ix1 $iy1]
    }
  }
}
