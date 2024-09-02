proc read_file { f } {
  set fp [open $f r]

  set data [read $fp]

  close $fp

  return $data
}

proc init { } {
  set ::shader [sb3d::shader]

# $::shader set fragment_shader [read_file shaders/metaballs.fs]
  $::shader set fragment_shader [read_file shaders/shader_1.fs]
}
