proc init { } {
  sb3d::canvas set module_dir modules/mandelbrot

  set mandelbrot [sb3d::shlib mandelbrot]

  echo [$mandelbrot get xmin]
  echo [$mandelbrot get ymin]
  echo [$mandelbrot get xmax]
  echo [$mandelbrot get ymax]

  $mandelbrot set a 1
  $mandelbrot exec a

  exit
}
