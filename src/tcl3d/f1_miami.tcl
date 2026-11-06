proc randIn { min max } {
  return [expr {rand()*($max - $min) + $min}]
} 

proc init { } {
  set ::path [sb3d::path]

  $::path set svg_path "m 388.9,100 c 0,0 62.7,36.7 72.9,42.5 10.1,5.8 16.2,12.4 15.9,16.8 -0.3,4.3 -4.6,10.1 -9.3,13 -4.6,2.9 -12.7,6.9 -15.9,11.3 -3.2,4.3 -5.8,7.8 -6.1,18.8 -0.3,11 -0.3,19.7 -2.6,26.3 -1.9,5.4 -8.7,17.9 -32,26 -22.6,7.8 -29.7,6 -48.4,5.5 -17.3,-0.5 -34.4,-10.1 -34.4,-10.1 0,0 -89.1,-50.6 -97.7,-55.5 -8.7,-4.9 -22.6,-7.5 -33.8,-4.9 -11.6,2.6 -14.6,5 -21.1,9.8 -5.4,4 -13.3,10.1 -20.8,11 -7.5,0.9 -16.9,0.4 -24.3,-2.9 -6.7,-3 -15.6,-12.2 -19.9,-16.8 -4.3,-4.6 -13.5,-10.7 -26.6,-11 -10.7,-0.3 -34.2,2.3 -47.7,18.3 -5.6,6.6 -9.7,11.7 -9.7,24.4 0,9.4 8.6,22 16.9,22.1 16.9,0.3 12.8,-2.2 25.7,-2.6 15.9,-0.5 22.6,1.2 31.5,4.6 8.9,3.4 33.5,9.4 46.6,9.5 17.4,0.2 122.1,-2 129,-2 17.2,-0.1 31.2,3.5 41.6,8.1 10.4,4.6 31.8,14.7 40.5,17.3 8.7,2.6 24.3,8.7 48,8.4 23.7,-0.3 66.2,-2.9 90.2,-11.6 24,-8.7 99.5,-34.4 120.9,-43.7 21.4,-9.3 65.1,-30.8 74,-36.7 5.6,-3.7 9,-5.8 9.3,-11 0.3,-5.2 -8.1,-11 -12.4,-13 -4.3,-2 -7.4,-3.3 -12.7,-7.8 -5.8,-4.9 -8.5,-10.5 -8.5,-16.6 0,-11.3 7.5,-19.6 16.6,-19.9 5.8,-0.2 23.5,0.3 28.4,-0.6 4.9,-0.9 12.1,-3.5 15.9,-9.5 3.8,-6 8.2,-14.9 9.6,-17.1 2.4,-3.7 1.7,-5.8 -2.6,-7.8 -4.3,-2 -6.7,-5.2 -4.6,-11.3 2.1,-6.1 6.2,-22.8 7,-27.2 0.7,-4.1 -0.3,-9.5 -6.4,-9.5 -7.8,0 -196.6,-7.2 -220,-7.8 -23.8,-0.4 -347.9,-13 -386,-15.2 -12,-0.7 -14.8,6.5 -14,10.2 1.5,6.9 18.5,18.2 28.9,24.3 10.4,6.1 14.2,7.5 26.6,8.1 12.4,0.6 19.7,-2.9 26.6,-7.2 6.9,-4.3 21.1,-12.4 37.9,-16.2 11.9,-2.7 33.8,-2.2 42.2,-0.9 11.9,1.9 23.7,6.9 34.4,13 10.7,6.1 80.4,47.1 80.4,47.1 z"

  set ::track [sb3d::shape]

  $::track set path $::path {10 4}

  $::track set diffuse_texture "textures/road_texture.jpg"

  $::path set visible 0

  # ---

# set ::car [sb3d::shape]
# $::car set cube
# $::car set scales [list 2.0 1.0 4.0]

  setModelDir data

  set ::playerRefCarObj [loadRefModel "race_car"]

  set ::playerCarObj [$::playerRefCarObj get ref_object]
  $::playerCarObj set scales [list 0.8 0.8 0.8]

  set ::playerCarDist 0.0
  set ::playerCarY    5
  set ::playerCarPos  [sb3d::vector3 [$::path get tpos 0.0]]
  set ::playerCarDir  [sb3d::vector3 [$::path get tdir 0.0]]
  set ::playerCarYaw  0
  set ::playerCarDYaw 6
  set ::playerCarVel  0.0
  set ::playerCarAcc  0.0
  set ::playerCarDA   0.001

  set ::playerCarDX  0
  set ::playerCarDXV 0.1

  set ::tickSpeed 0.1 ; # 1/10 second

  #---

  set ::computerRefCarObj [loadRefModel "race_car"]

  set carMaterial [sb3d::canvas get material "CarPaint.1"]
echo "$carMaterial"

  set computerCarMaterial [sb3d::material]
  $computerCarMaterial set name "computerCarMaterial"
echo "$computerCarMaterial"

  $computerCarMaterial set diffuse blue

  $::computerRefCarObj exec replace_material $carMaterial $computerCarMaterial

  set ::numComputerCars 5

  for {set i 0} {$i < $::numComputerCars} {incr i} {
    set ::computerCarObj($i) [$::computerRefCarObj get ref_object]
    $::computerCarObj($i) set scales [list 0.8 0.8 0.8]

    set ::computerCarPos($i) 0
    set ::computerCarVel($i) [randIn 0.0001 0.001]
  }

  # ---

  set ::floor [sb3d::shape]

  $::floor set cube

  $::floor set scales   [list 800.0 10.0 500.0]
  $::floor set position [list 400.0 -2.0 50.0]

  $::floor set diffuse_texture "textures/grass_texture.jpg"

  # ---

  set ::camera_x 0
  set ::camera_y 0

  sb3d::camera set disable_roll 1

  # ---

  addTexts

  # ---

  updatePlayerCar

  updateComputerCars
}

proc addTexts { } {
  set ::vel_text [sb3d::text]

  $::vel_text set position      [list -0.99 0.92 0]
  $::vel_text set color         white
  $::vel_text set text          "Vel: 0"
  $::vel_text set size          0.05
  $::vel_text set align         left
  $::vel_text set ignore_camera 1
  $::vel_text set ignore_world  1
}

proc updateTexts { } {
  set v [expr {100.0*$::playerCarVel/$::tickSpeed}]

  set vstr [format "%3.1f" $v]

  $::vel_text set text "Vel: $vstr"
}

proc setModelDir { dir } {
  set ::model_dir $dir
    
  sb3d::canvas set model_dir $::model_dir
}

proc loadRefModel { name } {
  set obj [loadModel "${::model_dir}/${name}.obj" "${name}_ref"  ]
  return $obj
}

proc loadModel { filename name } {
  set obj [sb3d::model $filename]

  $obj set id $name

  return $obj
}

proc updatePlayerCar { } {
  set py $::playerCarY

  $::playerCarObj set position [list [$::playerCarPos get x] $py [$::playerCarPos get z]]

  set ::playerCarYaw [carDirToYaw]

  $::playerCarObj set angles [list 0 $::playerCarYaw 0]

  # ---

  set dx [$::playerCarDir get x]
  set dz [$::playerCarDir get z]

  set px [$::playerCarPos get x]
  set pz [$::playerCarPos get z]
  set py $::playerCarY

  set dc 5

  set cx [expr {$px - $dc*$dx}]
  set cz [expr {$pz - $dc*$dz}]
  set cy 7

  sb3d::camera set origin [list $cx $cy $cz]

  sb3d::camera set pitch 0
  sb3d::camera set yaw [expr {-$::playerCarYaw}]

  # ---

  set da1 [expr {$::playerCarYaw - 5}]
  set da2 [expr {$::playerCarYaw + 5}]

  set dir1 [carYawToDir1 $da1]
  set dir2 [carYawToDir1 $da2]

  set dl 10

  sb3d::light set current 0
  sb3d::light set enabled 1
  sb3d::light set type point

  set lx [expr {$px - $dl*$dx}]
  set lz [expr {$pz - $dl*$dz}]
  set ly 20

  sb3d::light set position  [list $lx $ly $lz]
  sb3d::light set direction [list $dx 0 $dz]

  sb3d::light set point_radius 100

  sb3d::light set current 1
  sb3d::light set enabled 1
  sb3d::light set type point
  
  set lx [expr {$px + $dl*[$dir1 get x]}]
  set lz [expr {$pz + $dl*[$dir1 get z]}]
  set ly 6
  
  sb3d::light set position  [list $lx $ly $lz]
  sb3d::light set direction [list $dx 0 $dz]
  
  sb3d::light set point_radius 100

  sb3d::light set current 2
  sb3d::light set enabled 1
  sb3d::light set type point

  set lx [expr {$px + $dl*[$dir2 get x]}]
  set lz [expr {$pz + $dl*[$dir2 get z]}]
  set ly 6
  
  sb3d::light set position  [list $lx $ly $lz]
  sb3d::light set direction [list $dx 0 $dz]
  
  sb3d::light set point_radius 100
}

proc carDirToYaw { } {
  set dx [$::playerCarDir get x]
  set dz [$::playerCarDir get z]

  set da [expr {atan2($dz, $dx)}]
  set da1 [expr {180*$da/$::PI}]

  set da2 [expr {$da1 + 90}]
  set da3 [expr {-$da2}]

  return $da3
}

proc carYawToDir { } {
  return [carYawToDir1 $::playerCarYaw]
}

proc carYawToDir1 { car_yaw } {
  set a1 [expr {-$car_yaw - 90}]

  return [yawToDir $a1]
}

proc yawToDir { yaw } {
  set a [expr {$::PI*$yaw/180.0}]

  set dx [expr {cos($a)}]
  set dy [expr {sin($a)}]

  return [sb3d::vector3 [list $dx 0 $dy]]
}

proc updateComputerCars { } {
  for {set i 0} {$i < $::numComputerCars} {incr i} {
    set ::computerCarPos($i) [expr {$::computerCarPos($i) + $::computerCarVel($i)}]

    if {$::computerCarPos($i) >= 1.0} {
      set ::computerCarPos($i) [expr {$::computerCarPos($i) - 1.0}]
    }

    set pos [$::path get tpos $::computerCarPos($i)]
    set dir [$::path get tdir $::computerCarPos($i)]

    set dx [lindex $dir 0]
    set dz [lindex $dir 2]

    set da  [expr {atan2($dz, $dx)}]
    set da1 [expr {180*$da/$::PI}]
    set da2 [expr {$da1 + 90}]
    set da3 [expr {-$da2}]

    set px [lindex $pos 0]
    set pz [lindex $pos 2]
    set py $::playerCarY

    $::computerCarObj($i) set position [list $px $py $pz]
    $::computerCarObj($i) set angles   [list 0 $da3 0]
  }
}

proc tick { args } {
  if       {[sb3d::canvas get key "left"]} {
if {0} {
    set ::playerCarDX [expr {$::playerCarDX - $::playerCarDXV}]

    if {$::playerCarDX < -1.0} {
      set ::playerCarDX -1.0
    }
}
    set ::playerCarYaw [expr {$::playerCarYaw + $::playerCarDYaw}]
    set ::playerCarDir [carYawToDir]
    # echo "$::playerCarYaw [$::playerCarDir get string] [carDirToYaw]"
  } elseif {[sb3d::canvas get key "right"]} {
if {0} {
    set ::playerCarDX [expr {$::playerCarDX + $::playerCarDXV}]

    if {$::playerCarDX > 1.0} {
      set ::playerCarDX 1.0
    }
}
    set ::playerCarYaw [expr {$::playerCarYaw - $::playerCarDYaw}]
    set ::playerCarDir [carYawToDir]
    # echo "$::playerCarYaw [$::playerCarDir get string] [carDirToYaw]"
  }

  if       {[sb3d::canvas get key "up"]} {
    if {$::playerCarAcc < 0} {
      set ::playerCarAcc 0
    }
    set ::playerCarAcc [expr {$::playerCarAcc + $::playerCarDA}]
    #echo "$::playerCarAcc"
  } elseif {[sb3d::canvas get key "down"]} {
    if {$::playerCarAcc > 0} {
      set ::playerCarAcc 0
    }
    set ::playerCarAcc [expr {$::playerCarAcc - 5*$::playerCarDA}]
    #echo "$::playerCarAcc"
  } else {
    set ::playerCarAcc [expr {$::playerCarAcc - $::playerCarDA}]
    #echo "$::playerCarAcc"
  }

  set ::playerCarVel [expr {$::playerCarVel + $::playerCarAcc}]

  if       {$::playerCarVel >= 1.0} {
    set ::playerCarVel 1.0
  } elseif {$::playerCarVel < 0.0} {
    set ::playerCarVel 0.0
  }

  set ::playerCarDist [expr {$::playerCarDist + $::playerCarVel}]

  if       {$::playerCarDist >= 1.0} {
    set ::playerCarDist [expr {$::playerCarDist - 1.0}]
  } elseif {$::playerCarDist < 0.0} {
    set ::playerCarDist [expr {1.0 + $::playerCarDist}]
  }

  set cv [sb3d::vector3 $::playerCarVel 1 $::playerCarVel]
  set ::playerCarPos [$::playerCarPos exec + [$::playerCarDir exec * $cv]]
  # echo [$::playerCarPos get string]

  updatePlayerCar

  updateComputerCars

  updateTexts
}
