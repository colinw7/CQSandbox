proc init { } {
  set ::ref_image [sb::image {0 0 px} "images/wolverine.jpg"]
  $::ref_image set visible 0

  set ::mask_image [sb::image {0 0 px} "images/star.png"]
  $::mask_image set visible 0

  set ::image [sb::image]
  $::image set image $::ref_image

  sb::ui create "\
<qxml>\n\
<QGroupBox title=\"Functions\">\n\
<QVBoxLayout margin=\"2\" spacing=\"2\">\n\
<QPushButton text=\"Unsharp Mask\"  onClicked=\"unsharpMask\" />\n\
<QPushButton text=\"Sobel\"         onClicked=\"sobel\"       />\n\
<QPushButton text=\"Gaussian Blur\" onClicked=\"gaussianBlur\"/>\n\
<QPushButton text=\"Turbulence\"    onClicked=\"turbulence\"  />\n\
<QPushButton text=\"Erode\"         onClicked=\"erode\"       />\n\
<QPushButton text=\"Dilate\"        onClicked=\"dilate\"      />\n\
<QPushButton text=\"Mask\"          onClicked=\"mask\"        />\n\
<QPushButton text=\"Tint\"          onClicked=\"tint\"        />\n\
<QPushButton text=\"Grayscale\"     onClicked=\"grayscale\"   />\n\
<QPushButton text=\"Sepia\"         onClicked=\"sepia\"       />\n\
<QPushButton text=\"Reset\"         onClicked=\"reset\"       />\n\
</QVBoxLayout>\n\
</QGroupBox>\n\
\
<QGroupBox title=\"Data\">\n\
<QVBoxLayout margin=\"2\" spacing=\"2\">\n\
<QHBoxLayout margin=\"2\" spacing=\"2\">\n\
<QLabel text=\"Mask Strength\"/>\n
<CQTclRealSpin varName=\"mask_strength\"/>\n
</QHBoxLayout>\n\
<CQTclCheckBox text=\"Feldman\" varName=\"feldman\"/>\n
</QVBoxLayout>\n\
</QGroupBox>\n\
\
<QLayoutItem stretch=\"1\"/>\n\
</qxml>"

  set ::mask_strength 2.0
  set ::feldman       0
}

proc unsharpMask { args } {
  echo "unsharp_mask $::mask_strength"

  $::image exec unsharp_mask $::mask_strength

  sb::canvas exec update
}

proc sobel { args } {
  echo "sobel $::feldman"

  $::image exec sobel $::feldman

  sb::canvas exec update
}

proc gaussianBlur { args } {
  $::image exec gaussian_blur

  sb::canvas exec update
}

proc turbulence { args } {
  $::image exec turbulence

  sb::canvas exec update
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
  $::image exec mask $::mask_image

  sb::canvas exec update
}

proc tint { args } {
  $::image exec tint

  sb::canvas exec update
}

proc grayscale { args } {
  $::image exec grayscale

  sb::canvas exec update
}

proc sepia { args } {
  $::image exec sepia

  sb::canvas exec update
}

proc reset { args } {
  $::image set image $::ref_image

  sb::canvas exec update
}
