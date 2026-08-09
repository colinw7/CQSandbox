proc init { } {
  #echo "init"

  set ::canvasSize 256

  set ::tm 48
  set ::rm 120

  set ::main_view sb::viewport_obj.1

  set ::control_view [sb::viewport [list 0 0 1 0.1]]

  sb::canvas set window.size [list [expr {$::canvasSize + $::rm}] [expr {$::canvasSize + $::tm}]]

  sb::canvas set view.rect {0 0.1 0.9 1.0}

  sb::canvas set buffered 1

  set ::max_iter 128

  # set ::pixelWidth  [sb::canvas get pixel_width]
  # set ::pixelHeight [sb::canvas get pixel_height]

  set ::iter_d1 [expr {($::max_iter - 1.0)/3.0}]
  set ::iter_d2 [expr {2.0*$::iter_d1}]
  set ::iter_d3 [expr {255.0/$::iter_d1}]

  for {set i 0} {$i < $::max_iter} {incr i} {
    set rgb [iterToColor $i]

    set ::colors($i) $rgb
  }

  set ::colors($::max_iter) [list 0 0 0]

  set ::renderer [sb::renderer]

  set ::PI 3.1415926535897931

  sb::canvas set view $::control_view

  set bx 5
  set by 5
  set bw 80
  set ::button1 [sb::button [list $bx $by px] "One"  ]; incr bx $bw
  set ::button2 [sb::button [list $bx $by px] "Two"  ]; incr bx $bw
  set ::button3 [sb::button [list $bx $by px] "Three"]; incr bx $bw
  set ::button4 [sb::button [list $bx $by px] "Four" ]

  $::button1 set proc "setType 0"
  $::button2 set proc "setType 1"
  $::button3 set proc "setType 2"
  $::button4 set proc "setType 3"

  sb::canvas set view $::main_view

  setType 0
}

proc setType { ind } {
  echo "setType $ind"

  set ::type_ind $ind

  if       {$ind == 0} {
    set ::xmin 0.077
    set ::ymin -0.21
    set ::xmax 1.08
    set ::ymax 0.79

    set ::p 0.6
    set ::q 1.1
  } elseif {$ind == 1} {
    set ::xmin 0.077
    set ::ymin -0.21
    set ::xmax 1.08
    set ::ymax 0.79

    set ::p 0.6
    set ::q 1.1
  } elseif {$ind == 2} {
    set ::xmin -2.1
    set ::ymin -1.03
    set ::xmax 1.7
    set ::ymax 1.03

    set ::p 1.0
    set ::q 1.0
  } elseif {$ind == 3} {
    set ::xmin -0.5
    set ::ymin -0.22
    set ::xmax 1.3
    set ::ymax 1.22

    set ::p 0.0
    set ::q 0.0
  }

  set ::xf [expr {(1.0/($::canvasSize  - 1))*($::xmax - $::xmin)}]
  set ::yf [expr {(1.0/($::canvasSize - 1))*($::ymin - $::ymax)}]

  sb::canvas exec step
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

  if       {$::type_ind == 0} {
    calc1 $x $y
  } elseif {$::type_ind == 1} {
    calc2 $x $y
  } elseif {$::type_ind == 2} {
    calc3 $x $y
  } elseif {$::type_ind == 3} {
    calc4 $x $y
  }
}

proc calc1 { x y } {
  set num_iter -1

  set zr $x
  set zi $y

  set zr2 [expr {$zr*$zr}]
  set zi2 [expr {$zi*$zi}]

  while {$zr2 + $zi2 < 4.0 && $num_iter < $::max_iter} {
    set px [expr {$::p*$zr}]
    set py [expr {$::p*$zi}]
    set qx [expr {$::q*$zr}]
    set qy [expr {$::q*$zi}]

    if {$zr >= 0} {
      set zr [expr {$px - $::p - $qy}]
      set zi [expr {$py - $::q + $qx}]
    } else {
      set zr [expr {$px + $::p - $qy}]
      set zi [expr {$py + $::q + $qx}]
    }

    set zr2 [expr {$zr*$zr}]
    set zi2 [expr {$zi*$zi}]

    incr num_iter
  }

  return $num_iter
}

proc calc2 { x y } {
  set num_iter -1

  set zr $x
  set zi $y

  set zr2 [expr {$zr*$zr}]
  set zi2 [expr {$zi*$zi}]

  while {$zr2 + $zi2 < 4.0 && $num_iter < $::max_iter} {
    set px [expr {$::p*$zr}]
    set py [expr {$::p*$zi}]
    set qx [expr {$::q*$zr}]
    set qy [expr {$::q*$zi}]

    if {$qx + $py >= 0} {
      set zr [expr {$px - $::p - $qy}]
      set zi [expr {$py - $::q + $qx}]
    } else {
      set zr [expr {$px + $::p - $qy}]
      set zi [expr {$py + $::q + $qx}]
    }

    set zr2 [expr {$zr*$zr}]
    set zi2 [expr {$zi*$zi}]

    incr num_iter
  }

  return $num_iter
}

proc calc3 { x y } {
  set num_iter -1

  set zr $x
  set zi $y

  set zr2 [expr {$zr*$zr}]
  set zi2 [expr {$zi*$zi}]
  set zri [expr {$zr*$zi}]

  while {$zi2 + $zr2 < 4.0 && $num_iter < $::max_iter} {
    if {$zr >= 0} {
      set zr [expr {$zr2 - $zi2 - 1.0}]
      set zi [expr {2.0*$zri}]
    } else {
      set zr [expr {$zr2 - $zi2 - 1.0 + $::p*$zr}]
      set zi [expr {2.0*$zri}]
    }

    set zr2 [expr {$zr*$zr}]
    set zi2 [expr {$zi*$zi}]
    set zri [expr {$zr*$zi}]

    incr num_iter
  }

  return $num_iter
}

proc calc4 { x y } {
  set num_iter -1

  set zr $x
  set zi $y

  set zr2 [expr {$zr*$zr}]
  set zi2 [expr {$zi*$zi}]

  while {$zr2 + $zi2 < 4.0 && $num_iter < $::max_iter} {
    set zr [expr {2.0*$zr}]
    set zi [expr {2.0*$zi}]

    if       {$zi > 1.0} {
      set zi [expr {$zi - 1}]
    } elseif {$zr > 1.0} {
      set zr [expr {$zr - 1}]
    }

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
  #return [expr {(($x*1.0)/($::canvasSize - 1))*($::xmax - $::xmin) + $::xmin}]
  return [expr {$x*$::xf + $::xmin}]
}

proc pixelYToUser { y } {
  #return [expr {(($y*1.0)/($::canvasSize - 1))*($::ymin - $::ymax) + $::ymax}]
  return [expr {$y*$::yf + $::ymax}]
}

proc resize { args } {
  echo "resize"

  sb::canvas exec step
}

proc drawBg { } {
  if {! [sb::canvas get buffered]} {
    return
  }

  echo "drawBg"

  for {set y 0} {$y < $::canvasSize} {incr y} {
    set yy [pixelYToUser $y]

    for {set x 0} {$x < $::canvasSize} {incr x} {
      set xx [pixelXToUser $x]

      set iter [calc $xx $yy]

      if {$iter >= 0} {
        set rgb $::colors($iter)

        $::renderer set pen.color $rgb

        $::renderer exec draw.point [list $x [expr {$y + $::tm}]]
      }
    }
  }
}
