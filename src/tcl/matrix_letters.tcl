proc randIn { min max } {
  return [expr {rand()*($max - $min) + $min}]
}

proc irandIn { min max } {
  return [expr {int(rand()*($max - $min) + $min)}]
}

proc init { } {
  # echo "init"

  sb::canvas set buffered 1

  sb::canvas set blend.enabled 1
  sb::canvas set blend.factor  0.9

  sb::canvas set brush.color black

  set ::renderer [sb::renderer]

  $::renderer set pen.color "#00FF00"

  $::renderer set font.size 64

  resize

  sb::canvas set play 1
}

proc resize { args } {
  # echo "resize"

  set ::fontHeight [$::renderer get font.height]

  set ::columns [expr {[sb::canvas get pixel_width ]/$::fontHeight}]
  set ::rows    [expr {[sb::canvas get pixel_height]/$::fontHeight}]

  for {set i 0} {$i < $::columns} {incr i} {
    set ::drops($i) 1
  }

  set ::chars "田由甲申甴电甶男甸甹町画甼甽甾甿畀畁畂畃畄畅畆畇畈畉畊畋界畍畎畏畐畑"
  # set ::chars "ABCEDFGHIJKLMNOPQRSTUVWXYZ"

  set ::num_chars [string length $::chars]
}

proc drawBg { } {
  # echo "drawBg"

  for {set i 0} {$i < $::columns} {incr i} {
    set x [expr {$i*$::fontHeight}]
    set y [expr {$::drops($i)*$::fontHeight}]

    set pos [irandIn 0 $::num_chars]

    set text [string range $::chars $pos $pos]

    if {$::drops($i) < $::rows} {
      $::renderer exec draw.text [list $x $y] $text
    }

    incr ::drops($i)

    if {$::drops($i) > $::rows && [irandIn 1 1000] > 975} {
      set ::drops($i) 0
    }
  }
}
