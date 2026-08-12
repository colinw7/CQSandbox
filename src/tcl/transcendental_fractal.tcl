proc init { } {
  # echo "init"

  set ::canvasSize 256

  set ::windowWidth  700
  set ::windowHeight 400

  sb::canvas set window.size [list $::windowWidth $::windowHeight]

  set ::PI 3.1415926535897931

  sb::canvas set controls.show 1

  sb::canvas set buffered 1

  # set ::pixelWidth  [sb::canvas get pixel_width]
  # set ::pixelHeight [sb::canvas get pixel_height]

  set ::max_iterations 128

  initColors

  set ::renderer [sb::renderer]

  set ::type ""
  setType "SIN"

  sb::ui "\
<qxml>\n\
<QPushButton text=\"COS\"  onClicked=\"setType COS\"/>\n\
<QPushButton text=\"SIN\"  onClicked=\"setType SIN\"/>\n\
<QPushButton text=\"SINH\" onClicked=\"setType SINH\"/>\n\
<QPushButton text=\"COSH\" onClicked=\"setType COSH\"/>\n\
<QPushButton text=\"EXP\" onClicked=\"setType EXP\"/>\n\
<QLayoutItem stretch=\"1\"/>\n\
</qxml>"
}

proc setType { type } {
  if {$type == $::type} {
    return
  }

  set ::type $type

  # echo "setType $::type"

  if       {$::type == "SIN"} {
    set ::xmin [expr { -2*$::PI}]
    set ::ymin [expr {-$::PI}]
    set ::xmax [expr {0}]
    set ::ymax [expr {$::PI}]
  } elseif {$::type == "COS"} {
    set ::xmin [expr {-$::PI}]
    set ::ymin [expr {-$::PI}]
    set ::xmax [expr {$::PI}]
    set ::ymax [expr {$::PI}]
  } elseif {$::type == "SINH"} {
    set ::xmin [expr {-$::PI}]
    set ::ymin [expr {-2*$::PI}]
    set ::xmax [expr {$::PI}]
    set ::ymax [expr {0}]
  } elseif {$::type == "COSH"} {
    set ::xmin [expr {-$::PI}]
    set ::ymin [expr {-$::PI}]
    set ::xmax [expr {$::PI}]
    set ::ymax [expr {$::PI}]
  } elseif {$::type == "EXP"} {
    set ::xmin [expr {-10*$::PI}]
    set ::ymin [expr {-10*$::PI}]
    set ::xmax [expr {10*$::PI}]
    set ::ymax [expr {10*$::PI}]
  }

  set ::xf [expr {(1.0/($::canvasSize - 1))*($::xmax - $::xmin)}]
  set ::yf [expr {(1.0/($::canvasSize - 1))*($::ymin - $::ymax)}]

  sb::canvas exec redraw
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
  # echo "iterToColor $iter"

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
  # echo "calc $x $y"

  if       {$::type == "SIN"} {
    calc_sin $x $y
  } elseif {$::type == "COS"} {
    calc_cos $x $y
  } elseif {$::type == "SINH"} {
    calc_sinh $x $y
  } elseif {$::type == "COSH"} {
    calc_cosh $x $y
  } elseif {$::type == "EXP"} {
    calc_exp $x $y
  }
}

proc calc_cos { x y } {
  set iteration -1;

  set ::zr 0.0
  set ::zi 0.0

  set zr2 0.0
  set zi2 0.0

  while {$zi2 + $zr2 < 1000.0 && $iteration < $::max_iterations} {
    set zr1 [expr { cos($::zr)*cosh($::zi) + $x}]
    set zi1 [expr {-sin($::zr)*sinh($::zi) + $y}]

    set ::zr $zr1
    set ::zi $zi1

    set zr2 [expr {$::zr*$::zr}]
    set zi2 [expr {$::zi*$::zi}]

    incr iteration
  }

  return $iteration
}

proc calc_sin { x y } {
  set iteration -1;

  set ::zr 0.0
  set ::zi 0.0

  set zr2 0.0
  set zi2 0.0

  while {$zi2 + $zr2 < 1000.0 && $iteration < $::max_iterations} {
    set zr1 [expr {sin($::zr)*cosh($::zi) + $x}]
    set zi1 [expr {cos($::zr)*sinh($::zi) + $y}]

    set ::zr $zr1
    set ::zi $zi1

    set zr2 [expr {$::zr*$::zr}]
    set zi2 [expr {$::zi*$::zi}]

    incr iteration
  }

  return $iteration
}

proc calc_exp { x y } {
  set iteration -1;

  set ::zr 0.0
  set ::zi 0.0

  set zr2 0.0
  set zi2 0.0

  while {$zi2 + $zr2 < 1000.0 && $iteration < $::max_iterations} {
    set zr1 [expr {$x*exp($::zr)*cos($::zi) - $y*exp($::zr)*sin($::zi)}]
    set zi1 [expr {$y*exp($::zr)*cos($::zi) + $x*exp($::zr)*sin($::zi)}]

    set ::zr $zr1
    set ::zi $zi1

    set zr2 [expr {$::zr*$::zr}]
    set zi2 [expr {$::zi*$::zi}]

    incr iteration
  }

  return $iteration
}

proc calc_cosh { x y } {
  set iteration -1;

  set ::zr 0.0
  set ::zi 0.0

  set zr2 0.0
  set zi2 0.0

  while {$zi2 + $zr2 < 1000.0 && $iteration < $::max_iterations} {
    set zr1 [expr {cosh($::zr)*cos($::zi) - $x}]
    set zi1 [expr {sinh($::zr)*sin($::zi) + $y}]

    set ::zr $zr1
    set ::zi $zi1

    set zr2 [expr {$::zr*$::zr}]
    set zi2 [expr {$::zi*$::zi}]

    incr iteration
  }

  return $iteration
}

proc calc_sinh { x y } {
  set iteration -1;

  set ::zr 0.0
  set ::zi 0.0

  set zr2 0.0
  set zi2 0.0

  while {$zi2 + $zr2 < 1000.0 && $iteration < $::max_iterations} {
    set zr1 [expr {sinh($::zr)*cos($::zi) - $x}]
    set zi1 [expr {cosh($::zr)*sin($::zi) + $y}]

    set ::zr $zr1
    set ::zi $zi1

    set zr2 [expr {$::zr*$::zr}]
    set zi2 [expr {$::zi*$::zi}]

    incr iteration
  }

  return $iteration
}

proc pixelXToUser { x } {
  #return [expr {(($x*1.0)/($::canvasSize - 1))*($::xmax - $::xmin) + $::xmin}]
  return [expr {$x*$::xf + $::xmin}]
}

proc pixelYToUser { y } {
  #return [expr {(($y*1.0)/($::canvasSize - 1))*($::ymin - $::ymax) + $::ymax}]
  return [expr {$y*$::yf + $::ymax}]
}

proc drawBg { } {
  echo "> drawBg $::type"

  for {set y 0} {$y < $::canvasSize} {incr y} {
    set yy [pixelYToUser $y]

    for {set x 0} {$x < $::canvasSize} {incr x} {
      set xx [pixelXToUser $x]

      set iter [calc $xx $yy]

      if {$iter >= 0} {
        set rgb $::colors($iter)

        $::renderer set pen.color $rgb

        $::renderer exec draw.point [list $x $y]
      }
    }
  }

  echo "< drawBg $::type"
}
