proc randIn { min max } {
  return [expr {rand()*($max - $min) + $min}]
}

proc init { } {
  sb::canvas set range {0 0 100 100}

  set ::quad [sb::quad_tree]

  set n 100

  for {set i 0} {$i < $n} {incr i} {
    set x1 [randIn 0 100]
    set y1 [randIn 0 100]
    set w  [randIn 0 10]
    set h  [randIn 0 10]

    set x2 [expr {$x1 + $w}]
    set y2 [expr {$y1 + $h}]

    set rect [sb::rect [list $x1 $y1 $x2 $y2]]

    set r [randIn 0 1]
    set g [randIn 0 1]
    set b [randIn 0 1]

    $rect set brush.color [list $r $g $b]
    $rect set filled 1

    $::quad set object.add $rect
  }
}

proc update { } {
}

proc mousePress { x y } {
  set objs [$::quad get object.at_point [list $x $y]]

  if {[llength $objs] > 0} {
    echo $objs 
  }
}
