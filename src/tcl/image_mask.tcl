proc init { args } {
  sb::canvas set brush.color "lightsteelblue"

  set ::imageFile "jigsaw/Catwoman.jpg"
  set ::maskFile  "images/star.png"

  set ::imageObj [sb::image {0 0} $::imageFile]
  set ::maskObj  [sb::image {0 0} $::maskFile]

  $::maskObj set visible 0

  $::imageObj set image_mask $::maskObj
}
