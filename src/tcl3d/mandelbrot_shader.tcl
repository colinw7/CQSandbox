proc read_file { f } {
  set fp [open $f r]

  set data [read $fp]

  close $fp

  return $data
}

proc init { } {
  set ::shader [sb3d::shader_toy]

  $::shader set fragment_shader [read_file shader3d/mandelbrot.fs]
}
