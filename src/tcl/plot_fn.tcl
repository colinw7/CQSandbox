proc init { } {
  sb::canvas set window.size [list 1024 1024]

  set ::pixelWidth  [sb::canvas get pixel_width]
  set ::pixelHeight [sb::canvas get pixel_height]

  set ::renderer [sb::renderer]

  set ::xmin -10
  set ::ymin -10
  set ::xmax  10
  set ::ymax  10

  set ::rangeSet 0

  set ::ticks 0
}

proc calc { x } {
if {0} {
  set amplitude 1.0
  set frequency 1.0
  set y [expr {$amplitude*sin($x*$frequency)}]
}

if {0} {
  set y [expr {cos($x)}]
}

if {0} {
  set ticks [expr {[sb::canvas get ticks]/60.0}]

  set amplitude 1.0
  set frequency 1.0
  set y [expr {sin($x*$frequency)}]
  set t [expr {0.01*(-$ticks*130.0)}]
  set y [expr {$y + sin($x*$frequency*2.1   + $t      )*4.5}]
  set y [expr {$y + sin($x*$frequency*1.72  + $t*1.121)*4.0}]
  set y [expr {$y + sin($x*$frequency*2.221 + $t*0.437)*5.0}]
  set y [expr {$y + sin($x*$frequency*3.1122+ $t*4.269)*2.5}]
  set y [expr {$y * $amplitude*0.06}]
}

  # Properties
  #set octaves 1
  set octaves 4
  set lacunarity 2.0
  set gain 0.5

  # Initial values
  set amplitude 0.5
  set frequency 1.0

  # Loop of octaves
  set y 0
  for {set i 0} {$i < $octaves} {incr i} {
    set y [expr {$y + $amplitude*[sb::perlin_noise [expr {$frequency*$x}]]}]
    set frequency [expr {$frequency * $lacunarity}]
    set amplitude [expr {$amplitude * $gain}]
  }

  return $y
}

proc pixelXToUser { x } {
  return [expr {(($x*1.0)/($::pixelWidth - 1))*($::xmax - $::xmin) + $::xmin}]
}

proc userYToPixel { y } {
  set py [expr {($::pixelHeight - 1)*($y - $::ymin)/($::ymax - $::ymin)}]

  return [expr {$::pixelHeight - 1 - $py}]
}

proc drawBg { args } {
  set ::pixelWidth  [sb::canvas get pixel_width]
  set ::pixelHeight [sb::canvas get pixel_height]

  if {$::rangeSet == 0} {
    set ::rangeSet 1

    for {set px 0} {$px < $::pixelWidth} {incr px} {
      set x [pixelXToUser $px]
      set y [calc $x]

      if {$px == 0} {
        set ::ymin $y
        set ::ymax $y
      }

      if {$y < $::ymin} { set ::ymin $y }
      if {$y > $::ymax} { set ::ymax $y }
    }

    if {$::ymin == $::ymax} {
      set ::ymin [expr {$::ymax - 1}]
      set ::ymax [expr {$::ymax + 1}]
    }

    echo "$::ymin $::ymax"
  }

  set color [list 1 0 0]

  $::renderer set pen.color $color

  set px1 0
  set py1 0

  for {set px 0} {$px < $::pixelWidth} {incr px} {
    set x [pixelXToUser $px]
    set y [calc $x]

    set py [userYToPixel $y]

    if {$px > 0} {
      $::renderer exec draw.line [list $px1 $py1] [list $px $py]
    }

    set px1 $px
    set py1 $py
  }
}
