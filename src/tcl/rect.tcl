proc init { } {
  sb::canvas set range {0 0 100 100}

  set rect1 [sb::rect {10 10 40 40}]

  $rect1 set brush.color red
  $rect1 set brush.alpha 0.5

  $rect1 set brush.linear_gradient {0 0 1 1}
}
