proc init { } {
  sb::canvas set range {0 0 100 100}

  set group1 [sb::group {10 10 40 40}]
  set group2 [sb::group {60 60 90 90}]

  $group1 set brush.color red
  $group1 set brush.alpha 0.5
  $group2 set brush.color green
  $group2 set brush.alpha 0.5

  $group1 set range {0 0 100 100}
  $group2 set range {0 0 100 100}

  set circle1 [sb::circle {50 50} 25px]
  set circle2 [sb::circle {50 50} 25px]

  $circle1 set group $group1
  $circle2 set group $group2
}
