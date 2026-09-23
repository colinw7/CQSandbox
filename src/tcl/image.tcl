proc init { } {
  set ::ref_image [sb::image {0 0 px} "img/wolverine.jpg"]
  $::ref_image set visible 0

  set ::image [sb::image]
  $::image set image $::ref_image

  sb::ui create "\
<qxml>\n\
<QPushButton text=\"Unsharp Mask\"  onClicked=\"unsharpMask\" />\n\
<QPushButton text=\"Sobel\"         onClicked=\"sobel\"       />\n\
<QPushButton text=\"Gaussian Blur\" onClicked=\"gaussianBlur\"/>\n\
<QPushButton text=\"Turbulence\"    onClicked=\"turbulence\"  />\n\
<QPushButton text=\"Erode\"         onClicked=\"erode\"       />\n\
<QPushButton text=\"Dilate\"        onClicked=\"dilate\"      />\n\
<QPushButton text=\"Mask\"          onClicked=\"mask\"        />\n\
<QPushButton text=\"Tint\"          onClicked=\"tint\"        />\n\
<QPushButton text=\"Reset\"         onClicked=\"reset\"       />\n\
<QLayoutItem stretch=\"1\"                                    />\n\
</qxml>"
}

proc unsharpMask { args } {
  $::image exec unsharp_mask

  sb::canvas exec update
}

proc sobel { args } {
  $::image exec sobel

  sb::canvas exec update
}

proc gaussianBlur { args } {
  $::image exec gaussian_blue

  sb::canvas exec update
}

proc turbulence { args } {
}

proc erode { args } {
  $::image exec erode

  sb::canvas exec update
}

proc dilate { args } {
  $::image exec dilate

  sb::canvas exec update
}

proc mask { args } {
}

proc tint { args } {
}

proc reset { args } {
  $::image set image $::ref_image

  sb::canvas exec update
}
