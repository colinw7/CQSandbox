proc init { } {
  set size 400

  set x1 0
  set x2 [expr {$x1 + $size}]
  set x3 [expr {$x2 + $size}]

  set y1 0
  set y2 [expr {$y1 + $size}]
  set y3 [expr {$y2 + $size}]

  set ::renderer1 [sb::renderer [list $x1 $y1 $x2 $y2]]
  set ::renderer2 [sb::renderer [list $x2 $y1 $x3 $y2]]
  set ::renderer3 [sb::renderer [list $x1 $y2 $x2 $y3]]
  set ::renderer4 [sb::renderer [list $x2 $y2 $x3 $y3]]
}

proc drawBg { args } {
  $::renderer1 set brush.color red
  $::renderer1 exec fill.rect

  $::renderer2 set brush.color green
  $::renderer2 exec fill.rect

  $::renderer3 set brush.color blue
  $::renderer3 exec fill.rect

  $::renderer4 set brush.color orange
  $::renderer4 exec fill.rect
}
