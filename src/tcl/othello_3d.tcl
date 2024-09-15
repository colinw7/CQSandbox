proc is_odd { i } {
  return [expr {($i % 2) != 0}]
}

proc init { } {
  set ::moveColor "white"

  set ::othello [sb3d::othello]

  set y -0.35

  for {set iy 0} {$iy < 8} {incr iy} {
    set x -0.35

    for {set ix 0} {$ix < 8} {incr ix} {
      set shape [sb3d::shape]

      set ::square($ix,$iy) $shape

      $shape set cube [list 0.15 0.15 0.01]
      $shape set position [list $x $y 0]
      $shape set id "cube:$ix:$iy"

      if {[is_odd $ix] == [is_odd $iy]} {
        $shape set texture "textures/blue_marble.png"
      } else {
        $shape set texture "textures/white_marble.png"
      }

      set shape [sb3d::shape]

      set ::piece($ix,$iy) $shape

      $shape set sphere 0.04
      $shape set position [list $x $y 0.04]
      $shape set id "sphere:$ix:$iy"

      set x [expr {$x + 0.1}]
    }

    set y [expr {$y + 0.1}]
  }

  updateBoard
}

proc updateBoard { } {
  set y 0.0

  for {set iy 0} {$iy < 8} {incr iy} {
    set x 0.0

    for {set ix 0} {$ix < 8} {incr ix} {
      set ind [list $ix $iy]

      set w [$::othello get is_white_piece $ind]
      set b [$::othello get is_black_piece $ind]

      if       {$w} {
        $::piece($ix,$iy) set visible 1
        $::piece($ix,$iy) set color [list 1.0 1.0 1.0]
      } elseif {$b} {
        $::piece($ix,$iy) set visible 1
        $::piece($ix,$iy) set color [list 0.2 0.2 0.4]
      } else {
        $::piece($ix,$iy) set visible 0
      }
    }
  }
}

proc otherColor { c } {
  if {$c == "white"} {
    return "black"
  } elseif {$c == "black"} {
    return "white"
  } else {
    return $c
  }
}

proc click { args } {
  set id $args

  for {set iy 0} {$iy < 8} {incr iy} {
    for {set ix 0} {$ix < 8} {incr ix} {
      if {[$::square($ix,$iy) get id] == $id} {
        puts "click $::square($ix,$iy)"

        set ind [list $ix $iy]

        if {[$::othello get can_move $ind $::moveColor]} {
          puts "Do Move $ind $::moveColor"

          $::othello get do_move $ind $::moveColor

          set ::moveColor [otherColor $::moveColor]

          updateBoard

          set res [$::othello get best_move $::moveColor]
          puts "Best Move: $res"

          set ix1 [lindex $res 0]
          set iy1 [lindex $res 1]

          if {$ix1 >= 0 && $iy1 >= 0} {
            $::othello get do_move $res $::moveColor

            set ::moveColor [otherColor $::moveColor]

            updateBoard
          } else {
            set ::moveColor [otherColor $::moveColor]
          }
        }
      }
    }
  }
}
