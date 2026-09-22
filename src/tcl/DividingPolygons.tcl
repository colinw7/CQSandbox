proc randIn { min max } {
  return [expr {rand()*($max - $min) + $min}]
}

proc irandIn { min max } {
  return [expr {int(rand()*($max - $min) + $min)}]
}

proc dist { p1 p2 } {
  echo "dist $p1 $p2"

  set x1 [lindex $p1 0]
  set y1 [lindex $p1 1]
  set x2 [lindex $p2 0]
  set y2 [lindex $p2 1]

  set dx [expr {$x2 - $x1}]
  set dy [expr {$y2 - $y1}]

  return [sb::hypot $dx $dy]
}

proc init { } {
  set ::ANIMATE_STEPS 50
  set ::MIN_SIDE      30

  set ::renderer [sb::renderer]
  $::renderer set size [list 500 500]

  $::renderer set brush.color black

  set ::TWO_PI [expr {2*$::PI}]

  # $::renderer set colorMode "HSB" $::TWO_PI 1 1 1

  # smooth

  $::renderer set pen.color black
  $::renderer set pen.width 2

  initialize

  # frameRate 25
}

proc drawBg { args } {
  draw
}

proc draw { } {
  # Animate
  # filter ERODE
  # filter BLUR

  drawPolygons $::activePolygons
  drawPolygons $::splittablePolygons
  drawPolygons $::donePolygons

  # Check for polygons that ended to animate
  set activePolygons1 {}

  foreach p $::activePolygons {
echo "$p"
    if {[sb::invoke Polygon isAnimationDone $p]} {
      if {[sb::invoke Polygon canSplit $p]} {
        lappend ::splittablePolygons $p
      } else {
        lappend ::donePolygons $p
      }
    } else {
      lappend activePolygons1 $p
    }
  }

  set ::activePolygons $activePolygons1

  # Randomly split polygons
  set splitProbability [expr {1.0/([llength $::splittablePolygons] + 10)}]

  set splittablePolygons1 {}

  foreach p $::splittablePolygons {
    if {[randIn 0 1] < $splitProbability} {
      sb::invoke Polygon splitPolygon $p $::activePolygons
    } else {
      lappend splittablePolygons1 $p
    }
  }

  set ::splittablePolygons $splittablePolygons1

  # Reset, if finished
  if {[llength $::activePolygons] == 0 && [llength $::splittablePolygons] == 0} {
    initialize
  }
}

proc drawPolygons { polygons } {
  foreach p $polygons {
    sb::invoke Polygon draw $p
  }
}

proc mousePressed { args } {
  initialize
}

proc keyPressed { args } {
  initialize
}

proc initialize { } {
echo "initialize"
  set ::activePolygons     {}
  set ::splittablePolygons {}
  set ::donePolygons       {}

  set width  [sb::canvas get pixel_width]
  set height [sb::canvas get pixel_height]

  set x {}
  lappend x 3
  lappend x [expr {$width - 3}]
  lappend x [expr {$width - 3}]
  lappend x 3

  set y {}
  lappend y 3
  lappend y [expr {$height - 3}]
  lappend y [expr {$height - 3}]
  lappend y 3

  set hueValue [randIn 0 $::TWO_PI]
  set hueDelta [expr {$::TWO_PI/8}]

  set poly [sb::instance Polygon $x $y $hueValue $hueDelta]

  lappend ::activePolygons $poly
}

# --------------

# Class representing polygon.
sb::class Polygon

sb::method Polygon init { poly x y hueValue hueDelta } {
echo "init $poly $x $y $hueValue $hueDelta"
  $poly set x $x
  $poly set y $y

  $poly set hueValue $hueValue
  $poly set hueDelta $hueDelta

  $poly set animationStep 0
}

sb::method Polygon draw { poly } {
echo "draw $poly"
  set hueValue [$poly get hueValue]

  # Draws polygon. In order to get rounded corners, polygon is drawn as a bezier curve.
  # Middle points of polygon sides are used as anchor points and polygon
  # vertices as control points.
  # Initially polygon is partially transparent and transparency decreases after each redrawing.

  set animationStep  [$poly get animationStep]
  set animationStep1 [expr {$animationStep + 1}]

  $poly set animationStep $animationStep1

  set a [expr {0.1 + 0.9*pow($::animationStep1/$::ANIMATE_STEPS, 2)}]

  $::renderer set brush.color [hsv $hueValue 1 1 $a]
  $::renderer set pen.color   [hsv 0 1 0 $a]

  $renderer exec path.start

  set x1 [lindex $x 0]
  set y1 [lindex $y 0]
  set x2 [lindex $x 1]
  set y2 [lindex $y 1]

  $renderer exec path.moveTo [list [expr {($x1 + $x2)/2}] [expr {($y1 + $y2)/2}]]

  set n [llength [$poly get x]]

  for {set i 0} {$i < $n} {incr i} {
    set i1 [expr {$i + 1}]
    if {$i1 >= $n} { set i1 0}

    set i2 [expr {$i1 + 1}]
    if {$i2 >= $n} { set i2 0}

    set cx1 [lindex $x $i1]
    set cy1 [lindex $y $i1]

    set cx2 [lindex $x $i2]
    set cy2 [lindex $y $i2]

    set cxm [expr {($cx1 + $cx2)/2}]
    set cym [expr {($cy1 + $cy2)/2}]

    $renderer exec path.curveTo [list $cx1 $cy1] [list $cx1 $cy1] [list $cxm $cym]
  }

  $renderer exec path.end
}

# Returns true, if polygon animation is complete (if it is no more semi-transparent).
sb::method Polygon isAnimationDone { poly } {
echo "isAnimationDone $poly"
  set animationStep [$poly get animationStep]
echo "$animationStep $::ANIMATE_STEPS"

  if {$animationStep >= $::ANIMATE_STEPS} {
    return 1
  } else {
    return 0
  }
}

# Returns true, if polygon can be split into two smaller polygons.
# Polygon can be, split if at least two sides are splitable.
sb::method Polygon canSplit { poly } {
echo "canSplit $poly"
  set splitableSides 0

  set n [llength [$poly get x]]

  for {set i 0} {$i < $n} {incr i} {
    if {[sb::invoke Polygon isSplitableSide $poly $i]} {
      incr splitableSides
    }
  }

  return [expr {$splitableSides >= 2}]
}

# Returns true, if specified side is splitable.
# Side can be split, if it is longer than two miminum side lengths.
sb::method Polygon isSplitableSide { poly i } {
echo "isSplitableSide $poly $i"
  set x [$poly get x]
  set y [$poly get y]

echo $x
  set n [llength $x]
echo $i

  set x1 [lindex $x $i]
  set y1 [lindex $y $i]
echo "$x1 $y1"

  set i1 [expr {$i + 1}]
  if {$i1 >= $n} { set i1 0 }

  set x2 [lindex $x $i1]
  set y2 [lindex $y $i1]
echo "$x2 $y2"

  if {[dist [list $x1 $y1] [list $x2 $y2]] >= 2*$::MIN_SIDE} {
    return 1
  } else {
    return 0
  }
}

# Splits polygon into two. Append resulting polygons to specified list.
sb::method Polygon splitPolygon { poly targetList } {
  set n [llength [$poly get x]]

  # Randomly get first side to be split
  set side1 0
  do {
    set side1 [irandIn 0 $n]
  } while {! [sb::invoke Polygon isSplitableSide $poly $side1]}

  # Randomly get second side to be split
  set side2 0
  do {
    set side2 [irandIn 0 $n]
  } while {$side1 == $side2 || ! [sb::invoke Polygon isSplitableSide $poly $side2]}

  # Get randon split points
  set c1 [sb::invoke Polygon getRandomSplitPoint $poly $side1]
  set c2 [sb::invoke Polygon getRandomSplitPoint $poly $side2]

  # Do split
  lappend targetList [createNew $side1 $side2 $c1 $c2  1]
  lappend targetList [createNew $side2 $side1 $c2 $c1 -1]
}

# Create new polygon that is splitted half of this one
sb::method Polygon createNew { poly side1 side2 c1 c2 hueMultiplier } {
  set x [$poly get x]
  set y [$poly get y]

  set n [llength $x]

  set side11 [expr {$side1 + 1}]
  if {$side11 >= $n} { set side11 0 }

  set side21 [expr {$side2 + 1}]
  if {$side21 >= $n} { set side21 0 }

  set ic1 [expr {1 - $c1}]
  set ic2 [expr {1 - $c2}]

  set xSplit1 [expr {[lindex $x $side1]*$c1 + [lindex $x $side11]*$ic1}]
  set ySplit1 [expr {[lindex $y $side1]*$c1 + [lindex $y $side11]*$ic1}]

  set xSplit2 [expr {[lindex $x $side2]*$c2 + [lindex $x $side21]*$ic2}]
  set ySplit2 [expr {[lindex $y $side2]*$c2 + [lindex $y $side21]*$ic2}]

  set n1 [expr {($side2 - $side1 + $n) + 2}]
  if {$n1 >= $n} { set n1 [expr {$n1 - $n}] }

  set x1 {}
  set y1 {}

  lappend x1 $xSplit2
  lappend y1 $ySplit2
  lappend x1 $xSplit1
  lappend y1 $ySplit1

  for {set i 2} {$i < $n1} {incr i} {
    set i1 [expr {$side1 + $i - 1}]
    if {$i1 >= $n} { set i1 0 }

    lappend x1 [lindex $x $i1]
    lappend y1 [lindex $y $i1]
  }

  set hueValue [$poly get hueValue]
  set hueDelta [$poly get hueDelta]

  set hueValue1 [expr {($hueValue + $hueDelta) % $::TWO_PI*$::hueMultiplier}]
  set hueDelta1 [expr {$hueDelta*0.7}]

  set poly [sb::instance Polygon $x1 $y1 $hueValue1 $hueDelta1]

  return $poly
}

# Returns random split point for specifed side. Split point value is in interval (0,1).
sb::method Polygon getRandomSplitPoint { poly side } {
  set x [$poly get x]
  set y [$poly get y]

  set n [llength $x]

  set x1 [lindex $x $side]
  set y1 [lindex $y $side]

  incr side

  if {$side >= $n} {
    set n 0
  }

  set x2 [lindex $x $side]
  set y2 [lindex $y $side]

  set sideLength [dist [list $x1 $y1] [list $x2 $y2]]

  set c [expr {$::MIN_SIDE/$sideLength}]

  return [randIn $c [expr {1 - $c}]]
}
