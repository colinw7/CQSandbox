proc init { } {
  set ::n 8

  sb::canvas set range {0 0 1 1}
  sb::canvas set equal_scale 1

  set ::g [sb::circles_group {0 0 1 1}]

  $::g set brush.alpha 0.1
  $::g set n $::n

  set ::n_edit [sb::integer_edit {10 40 px} ::n]
  $::n_edit set min_value 3
  $::n_edit set max_value 99
  $::n_edit set proc update_n

  sb::canvas set play 1
}

proc update_n { args } {
  #puts "update_n $::n"
  $::g set n $::n
}
