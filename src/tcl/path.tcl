proc init { } {
  puts "init"

  sb::canvas set range {0 0 100 100}

  set ::path [sb::path {M 25 25 L 50 25 L 50 50 L 75 50}]
}
