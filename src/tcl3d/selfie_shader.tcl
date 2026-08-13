proc read_file { f } {
  set fp [open $f r]

  set data [read $fp]

  close $fp

  return $data
}

proc init { } {
  set ::shader [sb3d::shader]

  set data ""

  append data [read_file shader3d/selfie_common.fs]
  append data "\n"
  append data [read_file shader3d/selfie_bufferA.fs]
  append data "\n"
  append data [read_file shader3d/selfie_bufferB.fs]
  append data "\n"
  append data [read_file shader3d/selfie_image.fs]

  $::shader set fragment_shader $data
}
