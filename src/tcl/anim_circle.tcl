proc init { } {
  sb::canvas set range {-1 -1 1 1}

  set ::circle [sb::circle {0 0} 64px]

  $::circle set brush.color green

  $::circle set user.state         1
  $::circle set animate.animating  1
  $::circle set center.target      {1 0}
  $::circle set center.steps       20
  $::circle set radius.target      128px
  $::circle set radius.steps       20
  $::circle set brush.color.target blue
  $::circle set brush.steps        20
}

proc update { } {
  if {! [$::circle get animate.animating]} {
    set state [$::circle get user.state]

    if       {$state == 1} {
      set state  2
      set dest   {0 0}
      set color  green
      set radius 64px
    } elseif {$state == 2} {
      set state  3
      set dest   {-1 0}
      set color  red
      set radius 32px
    } elseif {$state == 3} {
      set state  4
      set dest   {0 0}
      set color  green
      set radius 64px
    } else {
      set state  1
      set dest   {1 0}
      set color  blue
      set radius 128px
    }

    $::circle set user.state         $state
    $::circle set animate.animating  1
    $::circle set center.target      $dest
    $::circle set radius.target      $radius
    $::circle set brush.color.target $color
  }
}
