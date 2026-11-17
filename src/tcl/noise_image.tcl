proc init { } {
  #echo "init"

  sb::canvas set window.size [list 600 200]

  set ::pixelWidth  [sb::canvas get pixel_width]
  set ::pixelHeight [sb::canvas get pixel_height]

  set ::renderer [sb::renderer]

  set ::xmin -10
  set ::ymin -10
  set ::xmax  10
  set ::ymax  10
}

proc calc { i x y } {
  if       {$i == 0} {
    return [sb::perlin_noise $x $y]
  } elseif {$i == 1} {
    return [sb::simplex_noise $x $y]
  } else {
    return [sb::solid_noise $x $y]
  }
}

proc pixelXToUser { x } {
  return [expr {(($x*1.0)/($::pixelWidth - 1))*($::xmax - $::xmin) + $::xmin}]
}

proc pixelYToUser { y } {
  return [expr {(($y*1.0)/($::pixelHeight - 1))*($::ymin - $::ymax) + $::ymax}]
}

proc drawBg { args } {
  for {set y 0} {$y < $::pixelHeight} {incr y} {
    set yy [pixelYToUser $y]

    for {set x 0} {$x < $::pixelWidth} {incr x} {
      set i 2
      if {$x < 200} {
        set i 0
      } elseif {$x < 400} {
        set i 1
      }

      set xx [pixelXToUser $x]

      set color [calc $i $xx $yy]

      set rgb [list $color $color $color]

      $::renderer set pen.color $rgb

      $::renderer exec draw.point [list $x $y]
    }
  }
}
