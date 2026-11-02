proc printVector { v } {
  echo "[$v get x] [$v get y] [$v get z]"
}

set v [sb3d::vector3]
printVector $v

set v [sb3d::vector3 1 2 3]
printVector $v

set v [sb3d::vector3 {3 4 5}]
printVector $v

$v set x 1
$v set y 2
$v set z 3
printVector $v

set v1 [sb3d::vector3 {1 0 0}]
set v2 [sb3d::vector3 {0 1 0}]

set v3 [$v1 exec cross $v2]
printVector $v3

set r [$v1 exec dot $v2]
echo $r

set d [$v1 exec distance $v2]
echo $d
