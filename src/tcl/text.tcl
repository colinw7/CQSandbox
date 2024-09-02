proc init { } {
  sb::canvas set range {0 0 100 100}

  sb::canvas set equal_scale 1

  # text
  set ::nt [sb::text {50 50} "Hello World"]
  set ::ht [sb::text {50 50} "<h1>Header</h1><p>Text</p>"]

  $::nt set align       right
  $::nt set brush.color blue

  $::ht set html        1
  $::ht set brush.color red
  $::ht set align       left
}
