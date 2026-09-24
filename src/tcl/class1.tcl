set Vector [sb::class Vector]

$Vector proc init { v x y } {
  # echo "init $v $x $y"

  $v set x $x
  $v set y $y
}

$Vector proc add { v1 v2 } {
  # echo "add $v1 $v2"

  $v1 set x [expr {[$v1 get x] + [$v2 get x]}]
  $v1 set y [expr {[$v1 get y] + [$v2 get y]}]
}

$Vector proc test { v x } {
  set x 1
}

$Vector proc print { v } {
  echo "[$v get x] [$v get y]"
}

# ---

set v1 [$Vector create 1 2]
set v2 [$Vector create 3 4]

$v1 exec add $v2
$v1 exec print

$v1 exec add $v2
$v1 exec print

set x 5
$v1 exec test $x
echo "$x"

set v3 [$Vector create 5 6]
$v3 exec print
