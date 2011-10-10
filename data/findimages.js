function find_images() {
  // Test for the really simple case first where the document is a single image.
  var children = document.body.childNodes;
  if (children[0] instanceof HTMLImageElement) {
    return children[0].src;
  }
}

// This must be the last function executed.
find_images();
