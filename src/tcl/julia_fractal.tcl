proc init { } {
  #echo "init"

  sb::canvas set window.size [list 512 512]

  set ::cr 0.238498
  set ::ci 0.519198

  set ::xmin -1.6
  set ::ymin -1.2
  set ::xmax  1.6
  set ::ymax  1.2

  set ::max_iter 128

  set ::pixelWidth  [sb::canvas get pixel_width]
  set ::pixelHeight [sb::canvas get pixel_height]

  initColors

  set ::renderer [sb::renderer]

  set ::PI 3.1415926535897931

  set ::xf [expr {(1.0/($::pixelWidth  - 1))*($::xmax - $::xmin)}]
  set ::yf [expr {(1.0/($::pixelHeight - 1))*($::ymin - $::ymax)}]
}

proc initColors { } {
  set ::iter_d1 [expr {($::max_iter - 1.0)/3.0}]
  set ::iter_d2 [expr {2.0*$::iter_d1}]
  set ::iter_d3 [expr {255.0/$::iter_d1}]

  for {set i 0} {$i < $::max_iter} {incr i} {
    set rgb [iterToColor $i]

    set ::colors($i) $rgb
  }

  set ::colors($::max_iter) [list 0 0 0]
}

proc iterToColor { iter } {
  #echo "iterToColor $iter"

  set r 0
  set g 0
  set b 0

  if       {$iter == $::max_iter} {
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

  set num_iter -1

  set zr $x
  set zi $y

  set zr2 [expr {$zr*$zr}]
  set zi2 [expr {$zi*$zi}]

  set zri 0

  while {$zi2 + $zr2 < 4.0 && $num_iter < $::max_iter} {
    set zri [expr {$zr*$zi}]

    set zr [expr {$zr2 - $zi2 + $::cr}]
    set zi [expr {$zri + $zri + $::ci}]

    set zr2 [expr {$zr*$zr}]
    set zi2 [expr {$zi*$zi}]

    incr num_iter
  }

  return $num_iter
}

proc setTime { t } {
  set a [expr {2.0*t*::PI}]

  set ::cr [expr {0.5*(1.0 + cos(a))}]
  set ::ci [expr {0.5*(1.0 + sin(a))}]
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
