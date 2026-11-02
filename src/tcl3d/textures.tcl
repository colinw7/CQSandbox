proc printTexture { texture } {
  echo "Texture: $texture"
}

proc init { } {
  # fbx
  set ::model_dir models/fbx
  sb3d::canvas set model_dir $::model_dir
  set ::model [sb3d::model models/fbx/spider.fbx]

  set textures [sb3d::canvas get textures]

  foreach texture $textures {
    printTexture $texture
  }
}
