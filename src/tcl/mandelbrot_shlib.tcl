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

  set ::max_iter 512

  $::mandelbrot set max_iterations $::max_iter

  initColors

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

proc initColors { } {
if {0} {
  set ::iter_d1 [expr {($::max_iter - 1.0)/3.0}]
  set ::iter_d2 [expr {2.0*$::iter_d1}]
  set ::iter_d3 [expr {255.0/$::iter_d1}]

  for {set i 0} {$i < $::max_iter} {incr i} {
    set rgb [iterToColor $i]

    set ::colors($i) $rgb
  }

  set ::colors($::max_iter) [list 0 0 0]
} else {
  set ::pal [sb::color_range]
}
}

proc iterToColor { iter } {
  #echo "iterToColor $iter"

if {0} {
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
} else {
  if {$iter == $::max_iter} {
    return "#000000"
  }

  set r [expr {$iter/($::max_iter - 1.0)}]

  return [$::pal get interp $r]
}

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

if {0} {
      set rgb $::colors($iter)
} else {
      set rgb [iterToColor $iter]
}

      $::renderer set pen.color $rgb

      $::renderer exec draw.point [list $ix $iy]
    }
  }
}
