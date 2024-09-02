proc init { } {
  #puts "> init"

  set ::frequency 100.0
  set ::phase     0.0
  set ::duration  2.0
  set ::amplitude 1.0

  sb::canvas set brush.color black

  set ::frequency_edit [sb::real_edit { 10 40 px} ::frequency]
  set ::phase_edit     [sb::real_edit {210 40 px} ::phase]
  set ::duration_edit  [sb::real_edit {410 40 px} ::duration]
  set ::amplitude_edit [sb::real_edit {610 40 px} ::amplitude]

  $::frequency_edit set min_value 1.0
  $::frequency_edit set max_value 1000.0

  # points
  set ::np 250

  for {set i 0} {$i < $::np} {incr i} {
    set ::point($i) [sb::circle {0 0} 5px]

    if {$i > 0} {
      set ::line($i) [sb::line {0 0} {0 0}]

      $::line($i) set pen.color red
    }
  }

  updatePoints

  #puts "< init"
}

proc updatePoints { } {
  #puts "> updatePoints"

  sb::set_range 0 -1 $::duration 1

  set pi  3.141592653
  set tau [expr {2*$pi}]

  set lastp {0 0}

  for {set i 0} {$i < $::np} {incr i} {
    set t [expr {$::duration*(1.0*$i)/$::np}]

    set a [expr {$t*$tau*$::frequency + $::phase}]

    set y [expr {cos($a)*$::amplitude}]

    set p [list $t $y]

    $::point($i) set center $p

    if {$i > 0} {
      $::line($i) set p1 $lastp
      $::line($i) set p2 $p
    }

    set lastp $p
  }

  #puts "< updatePoints"
}

proc update { } {
  updatePoints
}
