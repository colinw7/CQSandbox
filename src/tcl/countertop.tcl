proc init { } {
  sb::canvas set range {0 0 100 100}

  set ::path [sb::path]

  $::path set  angle  0
  $::path exec moveTo {50 50}
  $::path exec step   32.75
  $::path exec turn   -90
  $::path exec step   22.25
  $::path exec turn   -90
  $::path exec step   23.75
  $::path exec turn   45
  $::path exec step   32
  $::path exec turn   -45
  $::path exec step   22.5
  $::path exec turn   -90
  $::path exec step   9
  $::path exec turn   -45
  $::path exec step   50.5
}
