proc swap { a b } {
  upvar $a a1
  upvar $b b1

  set t  $a1
  set a1 $b1
  set b1 $t
}

proc init { } {
  #echo "init"

  sb::canvas set window.size [list 512 512]

  sb::canvas set module_dir modules/mandelbrot

  sb::canvas set rubberBandEvent 1

  # sb::canvas set buffered 1

  set ::mandelbrot [sb::shlib mandelbrot]

  $::mandelbrot set xmin -2.0
  $::mandelbrot set ymin -1.2
  $::mandelbrot set xmax  1.2
  $::mandelbrot set ymax  1.2

  set ::max_iterations 512

  $::mandelbrot set max_iterations $::max_iterations

  set ::pal [sb::color_range]

  set ::renderer [sb::renderer]

  resize [sb::canvas get pixel_width] [sb::canvas get pixel_height]

  set ::needsDraw 1

  # ---

  set uistr "<qxml>\n"

  set pnames [$::pal get palette_names]

  append uistr "<QComboBox onCurrentIndexChanged=\"paletteChanged\">\n"
  foreach pname $pnames {
    append uistr "<QComboItem>${pname}</QComboItem>\n"
  }
  append uistr "</QComboBox>\n"

  append uistr "\
<CQTclIntegerSpin varName=\"max_iterations\" onValueChanged=\"iterationsChanged\"/>\n"

  append uistr "<QLayoutItem stretch=\"1\"/>\n"
  append uistr "</qxml>"

  sb::ui create $uistr
}

proc resize { w h } {
  echo "resize $w $h"

  set ::pixelWidth  $w
  set ::pixelHeight $h

  $::mandelbrot set pixel_xmin 0
  $::mandelbrot set pixel_ymin 0
  $::mandelbrot set pixel_xmax $w
  $::mandelbrot set pixel_ymax $h

  set ::needsDraw 1
}

proc iterToColor { iter } {
  #echo "iterToColor $iter"

  if {$iter == $::max_iterations} {
    return "#000000"
  }

  set r [expr {$iter/($::max_iterations - 1.0)}]

  return [$::pal get interp $r]
}

proc drawBg { args } {
  # echo "drawBg"

  if {$::needsDraw} {
    set ::needsDraw 0

    drawMandelbrot
  }

  $::renderer exec paint.draw
}

proc drawMandelbrot { } {
  echo "drawMandelbrot"

  $::renderer exec paint.begin

  for {set iy 0} {$iy < $::pixelHeight} {incr iy} {
    set y [$::mandelbrot get user_y $iy]

    for {set ix 0} {$ix < $::pixelWidth} {incr ix} {
      set x [$::mandelbrot get user_x $ix]

      set iter [$::mandelbrot exec calc $x $y]

      set rgb [iterToColor $iter]

      $::renderer set pen.color $rgb

      $::renderer exec draw.point [list $ix $iy]
    }
  }

  $::renderer exec paint.end
}

proc rubberBandRelease { px1 py1 px2 py2 } {
  set x1 [$::mandelbrot get user_x $px1]
  set y1 [$::mandelbrot get user_y $py1]
  set x2 [$::mandelbrot get user_x $px2]
  set y2 [$::mandelbrot get user_y $py2]

  if {$x1 > $x2} { swap x1 x2 }
  if {$y1 > $y2} { swap y1 y2 }

  echo "zoom $x1 $y1 $x2 $y2"

  $::mandelbrot set xmin $x1
  $::mandelbrot set ymin $y1
  $::mandelbrot set xmax $x2
  $::mandelbrot set ymax $y2

  set ::needsDraw 1

  sb::canvas exec redraw
}

proc keyPress { args } {
  set key [lindex $args 0]
  
  if {$key == "r" || $key == "R"} {
    $::mandelbrot set xmin -2.0
    $::mandelbrot set ymin -1.2
    $::mandelbrot set xmax  1.2
    $::mandelbrot set ymax  1.2

    set ::needsDraw 1

    sb::canvas exec redraw 
  }
}

proc paletteChanged { args } {
  echo "paletteChanged $args"

  set ind [lindex $args 0]

  set pnames [$::pal get palette_names]

  $::pal set mode [lindex $pnames $ind]

  set ::needsDraw 1

  sb::canvas exec redraw
}

proc iterationsChanged { args } {
  echo "iterationsChanged $args"

  $::mandelbrot set max_iterations $::max_iterations

  set ::needsDraw 1

  sb::canvas exec redraw
}
