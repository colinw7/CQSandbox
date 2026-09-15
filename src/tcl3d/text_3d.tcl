proc init { } {
  set ::text [sb3d::text]

  $::text set color white

  $::text set text "Hello"

  $::text set position [list -9 -2 -10]
  $::text set size     10
}
