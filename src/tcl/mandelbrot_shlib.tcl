# TODO: palette change, zoom, iter change, tcl var widget

# tcl:  3066898 microseconds per iteration
# calc: 1214763 microseconds per iteration
# calc: 1106674 microseconds per iteration

proc init { } {
  #echo "init"

  sb::canvas set window.size [list 512 512]

  sb::canvas set module_dir modules/mandelbrot

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

  sb::ui create "\
<qxml>\n\
<QComboBox onCurrentIndexChanged=\"paletteChanged\">\n\
<QComboItem>magma</QComboItem>\n\
<QComboItem>moreland</QComboItem>\n\
<QComboItem>plasma</QComboItem>\n\
<QComboItem>rgb_range</QComboItem>\n\
<QComboItem>viridis</QComboItem>\n\
</QComboBox>\n\
<CQTclIntegerSpin varName=\"max_iterations\" onValueChanged=\"iterationsChanged\"/>\n\
<QLayoutItem stretch=\"1\"/>\n\
</qxml>"
}

proc resize { w h } {
  echo "resize $w $h"

  set ::pixelWidth  $w
  set ::pixelHeight $h

  $::mandelbrot set pixel_xmax $w
  $::mandelbrot set pixel_ymax $h
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

  echo [time drawMandelbrot]
}

proc drawMandelbrot { } {
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
}

proc paletteChanged { args } {
  echo "paletteChanged $args"

  set ind [lindex $args 0]

  if       {$ind == 0} {
    $::pal set mode magma
  } elseif {$ind == 1} {
    $::pal set mode moreland
  } elseif {$ind == 2} {
    $::pal set mode plasma
  } elseif {$ind == 3} {
    $::pal set mode rgb_range
  } elseif {$ind == 4} {
    $::pal set mode viridis
  }

  sb::canvas exec update
}

proc iterationsChanged { args } {
  echo "iterationsChanged $args"

  sb::canvas exec update
}
