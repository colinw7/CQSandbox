proc randIn { min max } {
  return [expr {rand()*($max - $min) + $min}]
}

proc irandIn { min max } {
  return [expr {int(rand()*($max - $min) + $min + 0.5)}]
}

proc init { } {
  #echo "init"

  set ::ind 1

  sb::canvas set window.size [list 512 512]

  set ::renderer [sb::renderer]
}

proc drawBg { } {
  # echo "drawBg"

  $::renderer set pen.color black

  set ::pixelWidth  [sb::canvas get pixel_width]
  set ::pixelHeight [sb::canvas get pixel_height]

  set x [irandIn 0 [expr {$::pixelWidth  - 1}]]
  set y [irandIn 0 [expr {$::pixelHeight - 1}]]

  set ::num_iterations 250000

  if       {$::ind == 1} {
    for {set i 0} {$i < $::num_iterations} {incr i} {
      set type [irandIn 0 2]

      if       {$type == 0} {
        set x [expr {$x/2}]
        set y [expr {$y/2}]
      } elseif {$type == 1} {
        set x [expr {($x + $::pixelWidth - 1)/2}]
        set y [expr {$y/2}]
      } elseif {$type == 2} {
        set x [expr {($x + $::pixelWidth/2  )/2}]
        set y [expr {($y + $::pixelHeight - 1)/2}]
      }

      $::renderer exec draw.point [list $x [expr {$::pixelHeight - 1 - $y}]]
    }
  } elseif {$::ind == 2} {
    for {set i 0} {$i < $::num_iterations} {incr i} {
      set type [irandIn 0 3]

      if       {$type == 0} {
        set x [expr {$x/3}]
        set y [expr {$y/3}]
      } elseif {$type == 1} {
        set x [expr {($x + $::pixelWidth - 1)*2/3}]
        set y [expr {$y/3}]
      } elseif {$type == 2} {
        set x [expr {($x + $::pixelWidth  - 1)/3}]
        set y [expr {($y + $::pixelHeight - 1)*2/3}]
      } elseif {$type == 3} {
        set x [expr {$x/3}]
        set y [expr {($y + $::pixelHeight - 1)*2/3}]
      }

      $::renderer exec draw.point [list $x $y]
    }
  } elseif {$::ind == 3} {
    set s2 [expr {sqrt(0.5)}]

    for {set i 0} {$i < $::num_iterations} {incr i} {
      set type [irandIn 0 2]

      if       {$type == 0} {
        set x [expr {$x*$s2}]
        set y [expr {$y*$s2}]
      } elseif {$type == 1} {
        set x [expr {sqrt((($::pixelWidth - 1)*($::pixelWidth - 1) + $x*$x)/2)}]
        set y [expr {$y*$s2}]
      } elseif {$type == 2} {
        set x [expr {sqrt(($::pixelWidth*$::pixelWidth/4 + $x*$x)/2)}]
        set y [expr {sqrt((($::pixelHeight - 1)*($::pixelHeight - 1) + $y*$y)/2)}]
      }

      $::renderer exec draw.point [list $x $y]
    }
  } elseif {$::ind == 4} {
    set length [expr {min($::pixelWidth, $::pixelHeight)}]

    set x1 [expr {-$length/2}]
    set y1 [expr {-$length/2}]
    set x2 [expr { $length/2}]
    set y2 [expr { $length/2}]

    fillBox $x1 $y1 $x2 $y2 1

    set level 4

    generateBox $x1 $y1 $x2 $y2 $level $length
  }
}

proc generateBox { x1 y1 x2 y2 level length } {
  set length1 [expr {$length/3}]

  set x3 [expr {$x1 + $length1}]
  set y3 [expr {$y1 + $length1}]
  set x4 [expr {$x2 - $length1}]
  set y4 [expr {$y2 - $length1}]

  nodeBox $x1 $y1 $x2 $y2 $x3 $y3 $x4 $y4 $level $length1
}

proc nodeBox { x1 y1 x2 y2 x3 y3 x4 y4 level length } {
  fillBox $x3 $y3 $x4 $y4 0

  if {$level == 0} {
    return
  }

  set level1 [expr {$level - 1}]

  generateBox $x1 $y1 $x3 $y3 $level1 $length
  generateBox $x3 $y1 $x4 $y3 $level1 $length
  generateBox $x4 $y1 $x2 $y3 $level1 $length
  generateBox $x1 $y3 $x3 $y4 $level1 $length
  generateBox $x4 $y3 $x2 $y4 $level1 $length
  generateBox $x1 $y4 $x3 $y2 $level1 $length
  generateBox $x3 $y4 $x4 $y2 $level1 $length
  generateBox $x4 $y4 $x2 $y2 $level1 $length
}

proc fillBox { x1 y1 x2 y2 solid } {
  set x1 [expr {$::pixelWidth /2 + $x1}]
  set y1 [expr {$::pixelHeight/2 + $y1}]
  set x2 [expr {$::pixelWidth /2 + $x2}]
  set y2 [expr {$::pixelHeight/2 + $y2}]

  if {$solid} {
    $::renderer set brush.color black
  } else {
    $::renderer set brush.color white
  }

  $::renderer exec draw.rect [list $x1 $y1 $x2 $y2]
}

proc keyPress { args } {
  set key [lindex $args 0]

  if       {$key == "1"} {
    set ::ind 1
  } elseif {$key == "2"} {
    set ::ind 2
  } elseif {$key == "3"} {
    set ::ind 3
  } elseif {$key == "4"} {
    set ::ind 4
  }
}
