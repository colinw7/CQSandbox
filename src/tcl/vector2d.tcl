proc printVector { v } {
  echo "[$v get x] [$v get y]"
}

set v [sb::vector]

printVector $v

$v set x 1
$v set y 2

printVector $v

$v exec inc.x 1
$v exec inc.y 1

printVector $v

$v exec dec.x 1
$v exec dec.y 1

printVector $v
