proc loadImage { file } {
  set image [sb::image {0 0} $file]

  $image set visible 0

  return $image
}

proc randBool { } {
  set r [expr {rand()}]

  return [expr {$r < 0.5}]
}

proc addTextLabel { str pos align } {
  set text [sb::text $pos $str]

  $text set pen.color    black
  $text set brush.color  black
  $text set align        $align
  $text set border.color none

  return $text 
}

proc init { } {
  set ::game_over 0

  set ::board [sb::array 8 8]

  set ::score_text [addTextLabel "" { 5  5} left ]
  set ::move_text  [addTextLabel "" { 95 5} right]
# set ::level_text [addTextLabel "" { 95 5} right]

  setWhitePlayer "Human"
  setBlackPlayer "Computer"

  setComputerLevel 3

  setMoveColor "White"

  set ::white_image [loadImage "othello/white_piece.png"]
  set ::black_image [loadImage "othello/black_piece.png"]

  sb::canvas set range {0 0 100 100}

  set ::bx 0
  set ::by 10
  set ::bw 90
  set ::bh 90

  set ::board_obj [sb::rect [list $::bx $::by [expr {$::bx + $::bw}] [expr {$::by + $::bh}]]]

  set ::dx [expr {$::bw/8.0}]
  set ::dy [expr {$::bh/8.0}]

  for {set i 0} {$i < 8} {incr i} {
    set y1 [expr {$::by + $i*$::dy}]
    set y2 [expr {$y1 + $::dy}]

    for {set j 0} {$j < 8} {incr j} {
      set x1 [expr {$::bx + $j*$::dx}]
      set x2 [expr {$x1 + $::dx}]

      set ::board_square_obj($i,$j) [sb::rect [list $x1 $y1 $x2 $y2]]

      if {[expr {($i + $j) & 1}]} {
        $::board_square_obj($i,$j) set brush.color [list 0.8 0.8 0.8]
      } else {
        $::board_square_obj($i,$j) set brush.color [list 0.7 0.7 0.7]
      }

      set ::piece_obj($i,$j) [sb::image]

      $::piece_obj($i,$j) set rect [list $x1 $y1 $x2 $y2]

      $::board set value -1 [list $i $j]
    }
  }

  $::board set value 0 {3 3}
  $::board set value 1 {4 3}
  $::board set value 1 {3 4}
  $::board set value 0 {4 4}

  # printArray $::board

  updateBoard

  setScoreBoard

  setScore

  set ::depthBoards {}

  sb::canvas set play 1
}

proc setWhitePlayer { player } {
  set ::whitePlayer $player
}

proc setBlackPlayer { player } {
  set ::blackPlayer $player
}

proc setMoveColor { color } {
  set ::moveColor $color

  set ::moveColorText "$color to Move"

  $::move_text set text $::moveColorText
}

proc setComputerLevel { level } {
  set ::computerLevel $level
}

proc setScore { } {
  set numWhite [getNumWhite $::board]
  set numBlack [getNumBlack $::board]

  set ::scoreText "White $numWhite Black $numBlack"

  $::score_text set text "$::scoreText"
}

proc nextMoveColor { } {
  setMoveColor [otherColor $::moveColor]

  if {! [canMoveAnywhere $::board $::moveColor]} {
    setMoveColor [otherColor $::moveColor]
  
    if {! [canMoveAnywhere $::board $::moveColor]} {
      gameOver
    }
  }
}

proc gameOver { } {
  set numWhite [getNumWhite $::board]
  set numBlack [getNumBlack $::board]

  if       {$numWhite > $numBlack } {
    set ::scoreText "White Wins"
  } elseif {$numBlack > $numWhite } {
    set ::scoreText "Black Wins"
  } else {
    set ::scoreText "Draw"
  }

  $::score_text set text "$::scoreText"
  $::move_text  set text ""

  set ::game_over 1

  sb::canvas set play 0
}

proc getNumWhite { board } {
  return [getNumOfValue $board 1]
}

proc getNumBlack { board } {
  return [getNumOfValue $board 0]
}

proc getNumOfValue { board value } {
  set n 0

  for {set y 0} {$y < 8} {incr y} {
    for {set x 0} {$x < 8} {incr x} {
      set value1 [getPiece $board $x $y]

      if {$value1 == $value} {
        incr n
      }
    }
  }

  return $n
}

proc setScoreBoard { } {
  set scoreValues {
    { 128   1  64   8   8  64   1 128 }
    {   1   0   4   2   2   4   0   1 }
    {  64   4  32  16  16  32   4  64 }
    {   8   2  16   0   0  16   2   8 }
    {   8   2  16   0   0  16   2   8 }
    {  64   4  32  16  16  32   4  64 }
    {   1   0   4   2   2   4   0   1 }
    { 128   1  64   8   8  64   1 128 }
  }

  for {set y 0} {$y < 8} {incr y} {
    for {set x 0} {$x < 8} {incr x} {
      set value [lindex [lindex $scoreValues $y] $x]

      set ::score_board($y,$x) $value
    }
  }
}

proc updateBoard { } {
  for {set i 0} {$i < 8} {incr i} {
    for {set j 0} {$j < 8} {incr j} {
      set v [$::board get value [list $i $j]]

      if       {$v == 0} {
        $::piece_obj($i,$j) set image $::black_image
      } elseif {$v == 1} {
        $::piece_obj($i,$j) set image $::white_image
      } else {
        $::piece_obj($i,$j) set image ""
      }
    }
  }
}

proc getCurrentPlayer { } {
  if       {$::moveColor == "White"} {
    return $::whitePlayer
  } elseif {$::moveColor == "Black"} {
    return $::blackPlayer
  } else {
    return "None"
  }
}

proc computerMove { } {
  # echo "computerMove"

  set player [getCurrentPlayer]

  while {$player == "Computer"} {
    set moveValue [colorValue $::moveColor]

    set bestMove [getBestMove $moveValue $::computerLevel]

    if {! [llength $bestMove]} {
      #echo "No computer Move"
      break
    }

    #echo "computerBestMove $bestMove"

    set ::best_x [lindex $bestMove 0]
    set ::best_y [lindex $bestMove 1]

    doMove $::board $::best_x $::best_y $::moveColor

    updateBoard

    setScore

    nextMoveColor

    set player [getCurrentPlayer]
  }

  # echo "done"
}

proc getBestMove { value maxDepth } {
  return [getBestMove1 $::board $value $maxDepth 0]
}

proc getBestMove1 { board value maxDepth depth } {
  #echo "getBestMove1 $value $maxDepth $depth"

  if {$maxDepth <= 0} {
    set score 0

    return {}
  }

  set color [valueColor $value]

  set move {}

  set score -9999

  set board1 [getDepthBoard $board $depth]

  set board_score [boardScore $board $value]

  for {set x1 0} {$x1 < 8} {incr x1} {
    for {set y1 0} {$y1 < 8} {incr y1} {
      set rc [canMove $board1 $x1 $y1 $color]
      #echo "canMove $board1 $x1 $y1 $color $rc"

      if {! $rc} {
        continue
      }

      doMove $board1 $x1 $y1 $value

      set score1 [expr {[boardScore $board1 $value] - $board_score}]

      set value1    [otherValue $value]
      set maxDepth1 [expr {$maxDepth - 1}]
      set depth1    [expr {$maxDepth + 1}]

      set move2 [getBestMove1 $board1 $value1 $maxDepth1 $depth1]
      #echo "getBestMove1 $board1 $value1 $maxDepth1 $depth1 $move2"

      if {[llength $move2]} {
        #echo "move2: $move2"

        set score2 [lindex $move2 2]

        set score1 [expr {$score1 - $score2}]
      }

      if {! [llength $move] || $score1 > $score || ($score1 == $score && [randBool])} {
        set move [list $x1 $y1 $score1]

        set score $score1
      }

      set board1 [getDepthBoard $board $depth]
    }
  }

  return $move
}

proc boardScore { board value } {
  set other_value [otherValue $value]

  set score 0

  for {set x 0} {$x < 8} {incr x} {
    for {set y 0} {$y < 8} {incr y} {
      set value1 [$board get value [list $y $x]]
      set score  $::score_board($y,$x)

      if       {$value1 == $value} {
        set score [expr {$score + $value}]
      } elseif {$value1 == $other_value} {
        set score [expr {$score - $value}]
      }
    }
  }

  return $score
}

proc otherValue { value } {
  if {$value == 0} { return 1 }
  if {$value == 1} { return 0 }

  return -1
}

proc getDepthBoard { board depth } {
  #echo "getDepthBoard $depth"

  set numDepthBoards [llength $::depthBoards]

  set depth1 [expr {$depth + 1}]

  while {$depth1 > $numDepthBoards } {
    lappend ::depthBoards [$board get dup]

    incr numDepthBoards
  }

  set board1 [lindex $::depthBoards $depth]

  copyBoard $board $board1

  return $board1
}

proc copyBoard { fromBoard toBoard } {
  for {set y 0} {$y < 8} {incr y} {
    for {set x 0} {$x < 8} {incr x} {
      set value [$fromBoard get value [list $y $x]]

      $toBoard set value $value [list $y $x]
    }
  }
}

proc canMoveAnywhere { board color } {
  for {set x 0} {$x < 8} {incr x} {
    for {set y 0} {$y < 8} {incr y} {
      set rc [canMove $board $x $y $color]
      #echo "canMove $board $x $y $color $rc"

      if {$rc} {
        return 1
      }
    }
  }
  
  return 0 
} 

proc canMove { board x y color } {
  # echo "canMove $board $x $y $color"

  if {! [isNoPiece $board $x $y]} {
    return 0
  }

  set color1 [otherColor $color]

  if {[canMoveDirection $board $x $y  1  0 $color $color1]} { return 1 }
  if {[canMoveDirection $board $x $y  1  1 $color $color1]} { return 1 }
  if {[canMoveDirection $board $x $y  0  1 $color $color1]} { return 1 }
  if {[canMoveDirection $board $x $y -1  1 $color $color1]} { return 1 }
  if {[canMoveDirection $board $x $y -1  0 $color $color1]} { return 1 }
  if {[canMoveDirection $board $x $y -1 -1 $color $color1]} { return 1 }
  if {[canMoveDirection $board $x $y  0 -1 $color $color1]} { return 1 }
  if {[canMoveDirection $board $x $y  1 -1 $color $color1]} { return 1 }

  return 0
}

proc doMove { board x y color } {
  #echo "doMove $board $x $y $color"

  set color1 [otherColor $color]

  if {[canMoveDirection $board $x $y  1  0 $color $color1]} {
    doMoveDirection $board $x $y  1  0 $color $color1
  }
  if {[canMoveDirection $board $x $y  1  1 $color $color1]} {
    doMoveDirection $board $x $y  1  1 $color $color1
  }
  if {[canMoveDirection $board $x $y  0  1 $color $color1]} {
    doMoveDirection $board $x $y  0  1 $color $color1
  }
  if {[canMoveDirection $board $x $y -1  1 $color $color1]} {
    doMoveDirection $board $x $y -1  1 $color $color1
  }
  if {[canMoveDirection $board $x $y -1  0 $color $color1]} {
    doMoveDirection $board $x $y -1  0 $color $color1
  }
  if {[canMoveDirection $board $x $y -1 -1 $color $color1]} {
    doMoveDirection $board $x $y -1 -1 $color $color1
  }
  if {[canMoveDirection $board $x $y  0 -1 $color $color1]} {
    doMoveDirection $board $x $y  0 -1 $color $color1
  }
  if {[canMoveDirection $board $x $y  1 -1 $color $color1]} {
    doMoveDirection $board $x $y  1 -1 $color $color1
  }

  setPieceColor $board $x $y $color
}

proc doMoveDirection { board x y dx dy color otherColor } {
  set x1 [expr {$x + $dx}]
  set y1 [expr {$y + $dy}]

  set value [getPieceColor $board $x1 $y1]

  if {$value != $otherColor} {
    return
  }

  setPieceColor $board $x1 $y1 $color

  set x1 [expr {$x1 + $dx}]
  set y1 [expr {$y1 + $dy}]

  set value [getPieceColor $board $x1 $y1]

  while {$value == $otherColor} {
    setPieceColor $board $x1 $y1 $color

    set x1 [expr {$x1 + $dx}]
    set y1 [expr {$y1 + $dy}]

    set value [getPieceColor $board $x1 $y1]
  }
}

proc canMoveDirection { board x y dx dy color otherColor } {
  # echo "canMoveDirection $board $x $y $dx $dy $color $otherColor"

  set x1 [expr {$x + $dx}]
  set y1 [expr {$y + $dy}]

  set value [getPieceColor $board $x1 $y1]

  if {$value != $otherColor} {
    return 0
  }

  set x1 [expr {$x1 + $dx}]
  set y1 [expr {$y1 + $dy}]

  set value [getPieceColor $board $x1 $y1]

  while {$value == $otherColor} {
    set x1 [expr {$x1 + $dx}]
    set y1 [expr {$y1 + $dy}]

    set value [getPieceColor $board $x1 $y1]
  }

  set value [getPieceColor $board $x1 $y1]

  if {$value != $color} {
    return 0
  }

  return 1
}

proc isNoPiece { board x y } {
  set value [getPiece $board $x $y]

  if {$value == -1} {
    return 1
  } else {
    return 0
  }
}

proc getPieceColor { board x y } {
  set value [getPiece $board $x $y]

  return [valueColor $value]
}

proc setPieceColor { board x y color } {
  set value [colorValue $color]

  setPiece $board $x $y $value
}

proc getPiece { board x y } {
  if {$x < 0 || $x >= 8 || $y < 0 || $y >= 8} {
    return -1
  }

  return [$board get value [list $y $x]]
}

proc setPiece { board x y value } {
  if {$x < 0 || $x >= 8 || $y < 0 || $y >= 8} {
    return -1
  }

  $board set value $value [list $y $x]
}

proc colorValue { color } {
  if {$color == "Black"} { return 0 }
  if {$color == "White"} { return 1 }

  return -1
}

proc valueColor { value } {
  if {$value == 0} { return "Black" }
  if {$value == 1} { return "White" }

  return "None"
}

proc otherColor { color } {
  if {$color == "White"} {
    return "Black"
  } else {
    return "White"
  }
}

proc printArray { a } {
  set dim0 [$a get dim0]
  set dim1 [$a get dim1]

  echo -nonewline "{"
  for {set i 0} {$i < $dim0} {incr i} {
    if {$i > 0} { echo -nonewline " " }
    echo -nonewline "{"
    for {set j 0} {$j < $dim1} {incr j} {
      if {$j > 0} { echo -nonewline " " }
      echo -nonewline "[$a get value [list $i $j]]"
    }
    echo -nonewline "}"
  }
  echo "}"
}

proc update { } {
  set player [getCurrentPlayer]

  if {$player == "Computer"} {
    computerMove
  }
}

proc mousePress { x y } {
  if {$::game_over} {
    return
  }

  set player [getCurrentPlayer]

  if {$player != "Human"} {
    return
  }

  set pos [pointToPos $x $y]

  echo "$x $y $pos"

  set c [lindex $pos 0]
  set r [lindex $pos 1]

  if {$c < 0 || $c >= 8 || $r < 0 || $r >= 8} {
    return
  }

  if {[canMove $::board $c $r $::moveColor]} {
    doMove $::board $c $r $::moveColor

    updateBoard
    
    setScore

    nextMoveColor
  }
}

proc pointToPos { x y } {
  set x1 [expr {$x - $::bx}]
  set y1 [expr {$y - $::by}]

  if {$x1 < 0 || $y1 < 0} {
    return [list -1 -1]
  }

  set c [expr {int($x1/$::dx)}]
  set r [expr {int($y1/$::dy)}]

  return [list $c $r]
}
