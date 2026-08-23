proc printVector { v } {
  echo "[$v get x] [$v get y]"
}

set v [sb3d::vector]

printVector $v

$v set x 1
$v set y 2

printVector $v
