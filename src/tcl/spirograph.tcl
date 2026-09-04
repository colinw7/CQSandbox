proc addToothedWheel { x y r rot t } {
  set rt1 [expr {$r + $::toothSize}]
  set rt2 [expr {$r - $::toothSize}]

  $::path exec moveTo [list [expr {$x + sin($rot)*$rt1}] [expr {$y + cos($rot)*$rt1}]]

  set i $t

  while {$i <= 2*$::PI + 1E-6} {
    set t6 [expr {$t/6}]

    set t65 [expr {$t6*5}]
    set t64 [expr {$t6*4}]
    set t63 [expr {$t6*3}]
    set t62 [expr {$t6*2}]

    set roti [expr {$rot + $i}]

    $::path exec curveTo \
      [list [expr {$x + sin($roti - $t65)*$rt1}] [expr {$y + cos($roti - $t65)*$rt1}]] \
      [list [expr {$x + sin($roti - $t64)*$rt2}] [expr {$y + cos($roti - $t64)*$rt2}]] \
      [list [expr {$x + sin($roti - $t63)*$rt2}] [expr {$y + cos($roti - $t63)*$rt2}]]
    $::path exec curveTo \
      [list [expr {$x + sin($roti - $t62)*$rt2}] [expr {$y + cos($roti - $t62)*$rt2}]] \
      [list [expr {$x + sin($roti - $t6 )*$rt1}] [expr {$y + cos($roti - $t6 )*$rt1}]] \
      [list [expr {$x + sin($roti       )*$rt1}] [expr {$y + cos($roti       )*$rt1}]]

    set i [expr {$i + $t}]
  }
}

proc init { } {
  set ::PI 3.1415926535897931

  sb::canvas set range {-100 -100 100 100}

  set ::path [sb::path]

  $::path set brush.color red
  $::path set pen.color black
  $::path set pen.width 4

  set ::toothSize 5

  set x   0
  set y   0
  set n   60
  set r   60
  set rot 0
  set a   [expr {2*$::PI/$n}]

  addToothedWheel $x $y $r $rot $a
}
