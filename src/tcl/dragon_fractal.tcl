proc init { } {
  #echo "init"

  sb::canvas set window.size [list 512 512]

  set ::xmin -2.2
  set ::ymin -1.5
  set ::xmax  4.2
  set ::ymax  1.5

  set ::max_iterations 128

  set ::pixelWidth  [sb::canvas get pixel_width]
  set ::pixelHeight [sb::canvas get pixel_height]

  initColors

  set ::renderer [sb::renderer]

  set ::xf [expr {(1.0/($::pixelWidth  - 1))*($::xmax - $::xmin)}]
  set ::yf [expr {(1.0/($::pixelHeight - 1))*($::ymin - $::ymax)}]
}

proc initColors { } {
  set ::iter_d1 [expr {($::max_iterations - 1.0)/3.0}]
  set ::iter_d2 [expr {2.0*$::iter_d1}]
  set ::iter_d3 [expr {255.0/$::iter_d1}]

  for {set i 0} {$i < $::max_iterations} {incr i} {
    set rgb [iterToColor $i]

    set ::colors($i) $rgb
  }

  set ::colors($::max_iterations) [list 0 0 0]
}

proc iterToColor { iter } {
  #echo "iterToColor $iter"

  set r 0
  set g 0
  set b 0

  if       {$iter == $::max_iterations} {
  } elseif {$iter < $::iter_d1} {
    set r [expr { $::iter_d3* $iter/255.0}]
  } elseif {$iter < $::iter_d2} {
    set g [expr { $::iter_d3*($iter - $::iter_d1)/255.0}]
  } else {
    set b [expr { $::iter_d3*($iter - $::iter_d2)/255.0}]
  }

  return [list $r $g $b]
}

proc calc { x y } {
  #echo "calc $x $y"

  set zr 0.5
  set zi 0.0

  set iteration -1

  while {$zr*$zr + $zi*$zi < 4.0 && $iteration < $::max_iterations} {
    set zrt [expr {($zi - $zr)*($zi + $zr) + $zr}]
    set zit [expr {$zr*$zi}]

    set zit1 [expr {$zit + $zit - $zi}]

    set zr [expr {$x*$zrt + $y*$zit1}]
    set zi [expr {$y*$zrt - $x*$zit1}]

    incr iteration
  }

  return $iteration
}

proc pixelXToUser { x } {
  #return [expr {(($x*1.0)/($::pixelWidth - 1))*($::xmax - $::xmin) + $::xmin}]
  return [expr {$x*$::xf + $::xmin}]
}

proc pixelYToUser { y } {
  #return [expr {(($y*1.0)/($::pixelHeight - 1))*($::ymin - $::ymax) + $::ymax}]
  return [expr {$y*$::yf + $::ymax}]
}

proc drawBg { } {
  # echo "drawBg"

  for {set y 0} {$y < $::pixelHeight} {incr y} {
    set yy [pixelYToUser $y]

    for {set x 0} {$x < $::pixelWidth} {incr x} {
      set xx [pixelXToUser $x]

      set iter [calc $xx $yy]

      if {$iter >= 0} {
        set rgb $::colors($iter)

        $::renderer set pen.color $rgb

        $::renderer exec draw.point [list $x $y]
      }
    }
  }
}
