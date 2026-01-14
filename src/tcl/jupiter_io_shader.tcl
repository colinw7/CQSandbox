proc read_file { f } {
  set fp [open $f r]

  set data [read $fp]

  close $fp

  return $data
}

proc init { } {
  set ::shader [sb3d::shader]

# $::shader set fragment_shader [read_file shader3d/jupiter_io_A.fs]
  $::shader set fragment_shader [read_file shader3d/jupiter_io_B.fs]

# set data ""
# append data [read_file shader3d/jupiter_io_common.fs]
# append data "\n"
# append data [read_file shader3d/jupiter_io_bufferA.fs]
# append data "\n"
# append data [read_file shader3d/jupiter_io_bufferB.fs]
# append data "\n"
# append data [read_file shader3d/jupiter_io_image.fs]
# append data "\n"
# $::shader set fragment_shader $data
}
