set ::ParticleSystem [sb::class ParticleSystem]

set ::Particle [sb::class Particle]

proc randIn { min max } {
  return [expr {rand()*($max - $min) + $min}]
}

proc init { } {
  sb::canvas set motionEvent 1

  set ::NUM_PARTICLES 1000

  # smooth

  set ::r [sb::renderer]

  set ::width  500
  set ::height 500

  $::r set size [list $::width $::height]

  $::r set brush.color black

  $::r exec fill.rect

  set ::p [$::ParticleSystem create]

  set ::mouseX 0
  set ::mouseY 0
}

proc drawBg { args } {
  $::r exec paint.draw
} 

proc update { } { 
  draw
}

proc draw { } {
  $::r exec paint.begin

  $::r set pen.color transparent
  $::r set brush.color {0 0 0 0.02}

  $::r exec fill.rect

  $::p exec update

  $::p exec render

  $::r exec paint.end
}

proc mouseMotion { x y } {
  set ::mouseX $x
  set ::mouseY $y
}

# ---

$::ParticleSystem proc init { p } {
  set particles [sb::obj_array $::NUM_PARTICLES]

  for {set i 0} {$i < $::NUM_PARTICLES} {incr i} {
    $particles set value $i [$::Particle create]
  }

  $p set particles $particles
}

$::ParticleSystem proc update { p } {
  set particles [$p get particles]

  for {set i 0} {$i < $::NUM_PARTICLES} {incr i} {
    [$particles get value $i] exec update
  }
}

$::ParticleSystem proc render { p } {
  $::r set pen.color white

  set particles [$p get particles]

  for {set i 0} {$i < $::NUM_PARTICLES} {incr i} {
    [$particles get value $i] exec render
  }
}

# ---

$::Particle proc init { p } {
  $p set position [sb::vector [randIn 0 $::width] [randIn 0 $::height]]
  $p set velocity [sb::vector 0 0]
}

$::Particle proc update { p } {
  set position [$p get position]
  set velocity [$p get velocity]

  $velocity set x [expr {20*[sb::noise [expr {$::mouseX/10 + [$position get y]/100}]] -0.5}]
  $velocity set y [expr {20*[sb::noise [expr {$::mouseY/10 + [$position get x]/100}]] -0.5}]

  $position exec add $velocity

  if {[$position get x] <         0} { $position exec inc.x $::width  }
  if {[$position get x] >  $::width} { $position exec dec.x $::width  }
  if {[$position get y] <         0} { $position exec inc.y $::height }
  if {[$position get y] > $::height} { $position exec dec.y $::height }

  $p set position $position
  $p set velocity $velocity
}

$::Particle proc render { p } {
  set position [$p get position]
  set velocity [$p get velocity]

  set x [$position get x]
  set y [$position get y]

  set vx [$velocity get x]
  set vy [$velocity get y]

  set x1 [expr {$x - $vx}]
  set y1 [expr {$y - $vy}]

  $::r exec draw.line [list $x $y] [list $x1 $y1]
}
