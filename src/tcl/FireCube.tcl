 # Fire Cube demo effect
 # by luis2048.
 #
 # A rotating wireframe cube with flames rising up the screen.
 # The fire effect has been used quite often for oldskool demos.
 # First you create a palette of 256 colors ranging from red to
 # yellow (including black). For every frame, calculate each row
 # of pixels based on the two rows below it: The value of each pixel,
 # becomes the sum of the 3 pixels below it (one directly below, one
 # to the left, and one to the right), and one pixel directly two
 # rows below it. Then divide the sum so that the fire dies out as
 # it rises.

# Flame colors

proc randIn { min max } {
  return [expr {rand()*($max - $min) + $min}]
}

proc init { } {
  set ::renderer [sb::renderer]

  set ::width  640
  set ::height 360

  set ::angle 0

  $::renderer set size [list $::width $::height]

  # Create buffered image for 3d cube
  set ::pg [sb::renderer] ; # 3D ?
  $::pg set size [list $::width $::height]

  set ::calc1 [sb::int_array $::width]
  set ::calc3 [sb::int_array $::width]
  set ::calc4 [sb::int_array $::width]
  set ::calc2 [sb::int_array $::height]
  set ::calc5 [sb::int_array $::height]

  # colorMode(HSB)

  # This will contain the pixels used to calculate the fire effect
  set ::fire [sb::int_matrix $::width $::height]

  set ::palette [sb::obj_array 255]

  # Generate the palette
  for {set x 0} {$x < [$::palette get dim]} {incr x} {
    # Hue goes from 0 to 85: red to yellow
    # Saturation is always the maximum: 255
    # Lightness is 0..255 for x=0..128, and 255 for x=128..255

    set h [expr {$x/3.0}]
    set s 255
    set b [sb::clamp [expr {$x*3.0}] 0 255]

    $::palette set value $x [sb::color [list ihsb $h $s $b]]
  }

  # Precalculate which pixel values to add during animation loop
  # this speeds up the effect by 10fps
  for {set x 0} {$x < $::width} {incr x} {
    $::calc1 set value $x [expr { $x                 % $::width}]
    $::calc3 set value $x [expr {($x - 1 + $::width) % $::width}]
    $::calc4 set value $x [expr {($x + 1)            % $::width}]
  }

  for {set y 0} {$y < $::height} {incr y} {
    $::calc2 set value $y [expr {($y + 1) % $::height}]
    $::calc5 set value $y [expr {($y + 2) % $::height}]
  }
}

proc drawBg { args } {
  $::renderer exec paint.draw
}

proc update { } {
  draw
}

proc draw { } {
  echo "draw"

  set ::angle [expr {$::angle + 0.05}]

if {0} {
  # Rotating wireframe cube
  $::pg exec paint.begin
  $::pg exec translate [expr {$::width >> 1}] [expr {$::height >> 1}]
  $::pg exec rotateX [expr {sin($::angle/2)}]
  $::pg exec rotateY [expr {cos($::angle/2)}]
  $::pg set  brush.color black
  $::pg exec fill.rect
  $::pg set  pen.color [list 128 128 128]
  $::pg exec scale 25
  $::pg exec brush.color transparent
  $::pg exec box 4
  $::pg exec paint.end
}

  # Randomize the bottom row of the fire buffer
  for {set x 0} {$x < $::width} {incr x} {
    $::fire set value $x [expr {$::height-1}] [expr {int([randIn 0 190])}]
  }

  # loadPixels
  $::renderer exec paint.begin

  set counter 0

  # Do the fire calculations for every pixel, from top to bottom
  for {set y 0} {$y < $::height} {incr y} {
    for {set x 0} {$x < $::width} {incr x} {
      # Add pixel values around current pixel

      $::fire set value $x $y [expr {\
        (([$::fire get value [$::calc3 get value $x] [$::calc2 get value $y]] +
          [$::fire get value [$::calc1 get value $x] [$::calc2 get value $y]] +
          [$::fire get value [$::calc4 get value $x] [$::calc2 get value $y]] +
          [$::fire get value [$::calc1 get value $x] [$::calc5 get value $y]]) << 5) / 129}]

      # Output everything to screen using our palette colors
      $::renderer set pen.color [$::palette get value [$::fire get value $x $y]]

      $::renderer exec image.pixel [list $x $y]

if {0} {
      # Extract the red value using right shift and bit mask
      # equivalent of red(pg.pixels[x+y*w])
      if {([$pg get pixel $counter] >> 16 & 0xFF) == 128} {
        # Only map 3D cube 'lit' pixels onto fire array needed for next frame
        $::fire set value $x $y 128
      }
}

      incr counter
    }
  }

  # updatePixels
  $::renderer exec paint.end
}
