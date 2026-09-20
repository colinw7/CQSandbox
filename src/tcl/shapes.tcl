proc init { } {
  sb::canvas set range {-2 -2 2 2}
  sb::canvas set equal_scale 1

  set ::circle [sb::circle {-0.5 -0.5} 1]
  set ::rect   [sb::rect   { 0.5 0.5 1.5 1.5}]
  set ::line   [sb::line   {-0.1 0.1} {1.0 1.0}]
  set ::text   [sb::text   { 0.7 0.2} "Hello World"]

  $::circle set brush.color blue
  $::rect   set brush.color green
  $::line   set pen.color   orange
  $::line   set pen.width   5
  $::text   set brush.color res
  $::text   set size        32
  $::text   set weight      450
}
