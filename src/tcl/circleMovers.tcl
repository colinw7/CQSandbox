proc randIn { min max } {
  return [expr {rand()*($max - $min) + $min}]
}

proc irandIn { min max } {
  return [expr {int(rand()*($max - $min) + $min)}]
}

proc randomColor { } {
  set r [expr {[irandIn 0 255]/255.0}]
  set g [expr {[irandIn 0 255]/255.0}]
  set b [expr {[irandIn 0 255]/255.0}]

  if {$::transparency} {
    set a [expr {[irandIn 0 50]/255.0}]

    return [list $r $g $b $a]
  } else {
    return [list $r $g $b]
  }
}

proc CircleMoversInit { } {
  set w [expr {$::windowSize/2}]
  set h [expr {$::windowSize/2}]

  for {set i 0} {$i < $::nMovers} {incr i} {
    CircleMoverInit $i $w $h
  }
}

proc CircleMoverInit { ind xI yI } {
  set ::circle_mover_x($ind)  $xI
  set ::circle_mover_y($ind)  $yI
  set ::circle_mover_x0($ind) $::circle_mover_x($ind)
  set ::circle_mover_y0($ind) $::circle_mover_y($ind)

  set dx [expr {$::circle_mover_x($ind) - $::circle_mover_x0($ind)}]
  set dy [expr {$::circle_mover_y($ind) - $::circle_mover_y0($ind)}]

  set ::circle_mover_r($ind) [expr {sqrt($dx*$dx + $dy*$dy)}]

  # its speed of travel in thetaDir
  set ::circle_mover_velocity($ind) [randIn $::vmin $::vmax]
  # direction of velocity
  set ::circle_mover_thetaDir($ind) [randIn 0 $::TWO_PI]

  if {$::varyColors} {
    set ::circle_mover_fillColor($ind) [randomColor]
  } else {
    set ::circle_mover_fillColor($ind) white
  }

  set ::circle_mover_trips($ind) 0

  set ::circle_mover_diam($ind) 0
}

proc CircleMoverMove { ind } {
  if {$::maxTraverses == 0 || $::circle_mover_trips($ind) < $::maxTraverses} {
    set ::circle_mover_x($ind) [expr {$::circle_mover_x($ind) + \
      $::circle_mover_velocity($ind)*$::velocityScaling*cos($::circle_mover_thetaDir($ind))}]
    set ::circle_mover_y($ind) [expr {$::circle_mover_y($ind) + \
      $::circle_mover_velocity($ind)*$::velocityScaling*sin($::circle_mover_thetaDir($ind))}]

    set ::circle_mover_thetaDir($ind) [expr {$::circle_mover_thetaDir($ind) + \
       [randIn [expr {-$::dThetaScaling*$::maxdTheta}] [expr {$::dThetaScaling*$::maxdTheta}]]}]

    if {$::polar} {
      CircleMoverCheckOutOfBoundsPolar $ind
      CircleMoverCalculateSizePolar $ind
    } else {
      CircleMoverCheckOutOfBoundsRect $ind
      CircleMoverCalculateSizeRect $ind
    }
  }
}

proc CircleMoverCheckOutOfBoundsPolar { ind } {
  set dx [expr {$::circle_mover_x($ind) - $::circle_mover_x0($ind)}]
  set dy [expr {$::circle_mover_y($ind) - $::circle_mover_y0($ind)}]

  set ::circle_mover_r($ind) [expr {sqrt($dx*$dx + $dy*$dy)}]

  if {$::circle_mover_r($ind) > $::windowSize/2} {
    set ::circle_mover_x($ind) $::circle_mover_x0($ind)
    set ::circle_mover_y($ind) $::circle_mover_y0($ind)
    set ::circle_mover_r($ind) 0

    if {$::varyColors} {
      set ::circle_mover_fillColor($ind) [randomColor]
    }

    set velocity [randIn $::vmin $::vmax]
    set thetaDir [randIn 0 $::TWO_PI]

    incr ::circle_mover_trips($ind)
  }
}

proc CircleMoverCalculateSizePolar { ind } {
  if {$::decreasingSize} {
    set ::circle_mover_diam($ind) [expr {$::diamInit*(1-$::circle_mover_r($ind)*2/$::windowSize)}]
  } else {
    set ::circle_mover_diam($ind) [expr {$::circle_mover_r($ind)*2.0/$::windowSize*$::diamInit}]
  }
}

proc CircleMoverCheckOutOfBoundsRect { ind } {
  if {$::circle_mover_x($ind) > $::windowSize || $::circle_mover_x($ind) < 0 ||
      $::circle_mover_y($ind) > $::windowSize || $::circle_mover_y($ind) < 0} {
    set $::circle_mover_x($ind) $::circle_mover_x0($ind)
    set $::circle_mover_y($ind) $::circle_mover_y0($ind)

    if {$::varyColors} {
      set ::circle_mover_fillColor($ind) [randomColor]
    }

    set ::circle_mover_velocity($ind) [randIn $::vmin $::vmax]
    set ::circle_mover_thetaDir($ind) [randIn 0 $::TWO_PI]

    incr ::circle_mover_trips($ind)
  }
}

proc CircleMoverCalculateSizeRect { ind } {
  set dx [expr {$::circle_mover_x($ind) - $::circle_mover_x0($ind)}]
  set dy [expr {$::circle_mover_y($ind) - $::circle_mover_y0($ind)}]

  if {$::decreasingSize} {
    set ::circle_mover_diam($ind) [expr {$::diamInit*(1-max(abs($dx),abs($dy))*2/$::windowSize}]
  } else {
    set ::circle_mover_diam($ind) [expr {max(abs($dx), abs($dy))*2/$::windowSize*$::diamInit}]
  }
}

proc CircleMoverDisplay { ind } {
  $::renderer exec paint.begin

  if {! $::outline} {
    $::renderer set pen.color transparent
  } else {
    $::renderer set pen.color black
  }

  if {$::maxTraverses == 0 || $::circle_mover_trips($ind) < $::maxTraverses} {
    $::renderer set brush.color $::circle_mover_fillColor($ind)

    set x1 $::circle_mover_x($ind)
    set y1 $::circle_mover_y($ind)
    set x2 [expr {$x1 + $::circle_mover_diam($ind)}]
    set y2 [expr {$y1 + $::circle_mover_diam($ind)}]

    $::renderer exec draw.ellipse [list $x1 $y1 $x2 $y2]
  }

  $::renderer exec paint.end
}

proc init { } {
  echo "init"

  set ::TWO_PI [expr {2*$::PI}]

  # set ::windowSize 500
  set ::windowSize 1024

  set ::renderer [sb::renderer [list 0 0 $::windowSize $::windowSize]]

# set xSize $::windowSize
# set ySize $::windowSize

  set ::maxdTheta [expr {$::TWO_PI/30}]

  set ::nMovers 5
  set ::diamInit 50

  set ::vmin 2
  set ::vmax 4

  set ::maxTraverses 0; # number of trips each mover takes. for infinite, 0

  set ::polar 1

  set ::decreasingSize 1

  set ::varyColors 0
  set ::transparency 0

  set ::outline 1

  set ::velocityScaling 1

  set ::dThetaScaling 1

  # $::renderer set smooth 1

  # $::renderer set size [list $::windowSize $::windowSize]

  $::renderer set brush.color white

  CircleMoversInit

  sb::canvas set play 1
}

proc update { args } {
  # echo "update $args"

  for {set i 0} {$i < $::nMovers} {incr i} {
    CircleMoverDisplay $i

    CircleMoverMove $i
  }
}

proc drawBg { args } {
  # echo "drawBg $args"

  $::renderer exec paint.draw
}

proc keyPress { args } {
  set key [lindex $args 0]

  if       {$key == "c" || $key == "C"} {
    set ::varyColors [expr {! $::varyColors}]

    if {$::varyColors} {
      for {set i 0} {$i < $::nMovers} {incr i} {
        set ::circle_mover_fillColor($i) [randomColor]
      }
    } else {
      for {set i 0} {$i < $::nMovers} {incr i} {
        set ::circle_mover_fillColor($i) white
      }
    }
  } elseif {$key == "p" || $key == "P"} {
    set ::polar 1
  } elseif {$key == "r" || $key == "R"} {
    set ::polar 0
  } elseif {$key == "t" || $key == "T"} {
    set ::transparency [expr {! $::transparency}]
  } elseif {$key == "o" || $key == "O"} {
    set ::outline [expr {! $::outline}]
  } elseif {$key == "s" || $key == "S"} {
    set ::decreasingSize [expr {! $::decreasingSize}]
  } elseif {$key == "i" || $key == "I"} {
    if {$::maxTraverses == 0} {
      set ::maxTraverses 1
    } else {
      set ::maxTraverses 0
    }
  } elseif {$key == "-"} {
    # turn less
    set ::dThetaScaling [expr {$::dThetaScaling/1.2}]
  } elseif {$key == "_"} {
    # turn more
    set ::dThetaScaling [expr {$::dThetaScaling*1.2}]
  } elseif {$key == "+"} {
    # faster
    set ::velocityScaling [expr {$::velocityScaling*1.2}]
  } elseif {$key == "="} {
    # slower
    set ::velocityScaling [expr {$::velocityScaling/1.2}]
  } elseif {$key == "\["} {
    # smaller
    set ::diamInit [expr {$::diamInit/1.2}]
  } elseif {$key == "\{"} {
    # bigger
    set ::diamInit [expr {$::diamInit*1.2}]
  } elseif {$key == "\]"} {
    # less
    incr ::nMovers -1

    if {$::nMovers < 1} {
      set ::nMovers 1
    }

    CircleMoversInit
  } elseif {$key == "\}"} {
    # more
    incr ::nMovers

    CircleMoversInit
  } elseif {$key == "b"} {
    # clear
    $::renderer set brush.color white
    $::renderer set pen.color   transparent

    $::renderer exec fill.rect

    CircleMoversInit
  } elseif {$key == " "} {
    # reset
    $::renderer set brush.color white
    $::renderer set pen.color   transparent

    $::renderer exec fill.rect

    set ::diamInit        50
    set ::maxTraverses    0;  # number of trips each mover takes. for infinite, 0
    set ::polar           1
    set ::decreasingSize  1
    set ::varyColors      0
    set ::transparency    0
    set ::outline         1
    set ::velocityScaling 1
    set ::dThetaScaling   1

    set ::nMovers 10

    CircleMoversInit
  }
}

proc mousePress { args } {
  $::renderer exec paint.begin

  $::renderer set brush.color white
  $::renderer set pen.color   transparent

  $::renderer exec fill.rect

  $::renderer exec paint.end

  CircleMoversInit
}
