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

  set ::max_iter 128

  $::mandelbrot set max_iterations $::max_iter

  set ::iter_d1 [expr {($::max_iter - 1.0)/3.0}]
  set ::iter_d2 [expr {2.0*$::iter_d1}]
  set ::iter_d3 [expr {255.0/$::iter_d1}]

  for {set i 0} {$i < $::max_iter} {incr i} {
    set rgb [iterToColor $i]

    set ::colors($i) $rgb
  }

  set ::colors($::max_iter) [list 0 0 0]

  set ::renderer [sb::renderer]

  resize [sb::canvas get pixel_width] [sb::canvas get pixel_height]
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

  set r 0
  set g 0
  set b 0

  if       {$iter == $::max_iter} {
  } elseif {$iter < $::iter_d1} {
    set r [expr { $::iter_d3* $iter              /255.0}]
  } elseif {$iter < $::iter_d2} {
    set g [expr { $::iter_d3*($iter - $::iter_d1)/255.0}]
  } else {
    set b [expr { $::iter_d3*($iter - $::iter_d2)/255.0}]
  }

  return [list $r $g $b]
}

proc drawBg { } {
  # echo "drawBg"

  echo [time drawMandelbrot]
}

proc drawMandelbrot { } {
  for {set y 0} {$y < $::pixelHeight} {incr y} {
    set yy [$::mandelbrot get user_y $y]

    for {set x 0} {$x < $::pixelWidth} {incr x} {
      set xx [$::mandelbrot get user_x $x]

      set iter [$::mandelbrot exec calc $xx $yy]

      set rgb $::colors($iter)

      $::renderer set pen.color $rgb

      $::renderer exec draw.point [list $x $y]
    }
  }
}
