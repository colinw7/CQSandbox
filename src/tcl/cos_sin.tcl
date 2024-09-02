proc init { } {
  #puts "> init"

  sb::canvas set range {0 0 100 100}

  sb::canvas set equal_scale 1

  sb::canvas set brush.color black

  # main circle
  set ::c1 [sb::circle {50 50} 25]

  $::c1 set pen.color white
  $::c1 set brush.color "#444"

  # circle point
  set ::c2 [sb::circle {75 50} 1]

  $::c2 set pen.color white
  $::c2 set brush.color "#dddddd"

  # cos circle lines and text
  set ::cx  [sb::circle {75 50} 1]
  set ::tx  [sb::text {50 50} ""]
  set ::lx1 [sb::line {50 0} {50 50}]
  set ::lx2 [sb::line {25 10} {75 10}]

  $::cx set pen.color white
  $::cx set brush.color "#5555ff"

  $::tx set pen.color white

  $::lx1 set pen.dash {6 6}
  $::lx1 set pen.color "#5555ff"
  $::lx2 set pen.color "#5555ff"

  # sin circle lines and text
  set ::cy  [sb::circle {75 50} 1]
  set ::ty  [sb::text {50 50} ""]
  set ::ly1 [sb::line {50 75} {100 75}]
  set ::ly2 [sb::line {85 25} {85 75}]

  $::cy set pen.color white
  $::cy set brush.color "#55ff55"

  $::ty set pen.color white

  $::ly1 set pen.dash {6 6}
  $::ly1 set pen.color "#55ff55"
  $::ly2 set pen.color "#55ff55"

  set ::a 0.0

  updateObjects

  #puts "< init"
}

proc update { } {
  #puts "> update"

  # next angle
  set da [expr {3.141592653 / 90.0}]

  set ::a [expr {$::a + $da}]

  updateObjects

  #puts "< update"
}

proc updateObjects { } {
  # calc cos/sin
  set c [expr {cos($::a)}]
  set s [expr {sin($::a)}]

  # set circle point
  set x [expr {50 + 25*$c}]
  set y [expr {50 + 25*$s}]

  $::c2 set center [list $x $y]

  # set cos text and lines
  $::cx set center [list $x 10]

  $::lx1 set p1 [list $x 10]
  $::lx1 set p2 [list $x $y]

  $::tx set position [list $x 7]
  $::tx set text "Cos: [format "%g" $c]"

  # set sin text and lines
  $::cy set center [list 85 $y]

  $::ly1 set p1 [list $x $y]
  $::ly1 set p2 [list 85 $y]

  $::ty set position [list 87 $y]
  $::ty set text "Sin: [format "%g" $s]"
}
