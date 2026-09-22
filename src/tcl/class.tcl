sb::class Vector

sb::method Vector init { v x y } {
  # echo "init $v $x $y"

  $v set x $x
  $v set y $y
}

sb::method Vector add { v1 v2 } {
  # echo "add $v1 $v2"

  $v1 set x [expr {[$v1 get x] + [$v2 get x]}]
  $v1 set y [expr {[$v1 get y] + [$v2 get y]}]
}

set v1 [sb::instance Vector 1 2]
set v2 [sb::instance Vector 3 4]

sb::invoke Vector add $v1 $v2

echo "[$v1 get x] [$v1 get y]"
