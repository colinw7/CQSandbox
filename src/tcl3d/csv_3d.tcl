proc init { } {
  set ::csv [sb3d::csv "csv/3d.csv"]

  $::csv set first_line_header 1

  if {! [$::csv exec load]} {
    puts "Model load failed"
    exit 1
  }

  set ::nr [$::csv get num_rows]
# set ::nc [$::csv get num_cols]

  createObjs
}

proc createObjs { } {
  set ::particles [sb3d::particle_list]

  $::particles set size $::nr

  for {set r 0} {$r < $::nr} {incr r} {
    set x [$::csv get data $r 1]
    set y [$::csv get data $r 2]
    set z [$::csv get data $r 3]
    set g [$::csv get data $r 6]

    $::particles set position [list $x $z $y] $r

    set c [list 1.0 1.0 1.0]

    if       {$g == "A"} {
      set c [list 1.0 0.0 0.0]
    } elseif {$g == "B"} {
      set c [list 0.0 1.0 0.0]
    } elseif {$g == "C"} {
      set c [list 0.0 0.0 1.0]
    } elseif {$g == "D"} {
      set c [list 1.0 0.0 1.0]
    } elseif {$g == "E"} {
      set c [list 0.0 1.0 1.0]
    } elseif {$g == "F"} {
      set c [list 1.0 1.0 0.0]
    } else {
      puts "Unhandled $g"
    }

    $::particles set color $c $r
  }

  set range [$::particles get range]

  puts $range
}
