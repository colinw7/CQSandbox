proc init { } {
  set ::palette [sb::obj_array 255]

  # Generate the palette
  for {set x 0} {$x < [$::palette get dim]} {incr x} {
    # Hue goes from 0 to 85: red to yellow
    # Saturation is always the maximum: 255
    # Lightness is 0..255 for x=0..128, and 255 for x=128..255

    set h [expr {$x/3.0}]
    set s 255
    set b [sb::clamp [expr {$x*3.0}] 0 255]

    $::palette set value $x [sb::color [list ihsb $h $s $b]]
  }

  set ::r [sb::renderer]

  $::r set size {255 255}

  $::r exec paint.begin

  for {set ix 0} {$ix < 255} {incr ix} {
    set c [$::palette get value $ix]

    $::r set pen.color $c

    $::r exec draw.line [list $ix 0] [list $ix 255]
  }

  $::r exec paint.end
}

proc drawBg { args } {
  $::r exec paint.draw
}
