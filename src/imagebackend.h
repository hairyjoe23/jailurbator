/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#ifndef IMAGEBACKEND_H
#define IMAGEBACKEND_H

#include "image.h"

#include <QObject>

class Database;

class ImageBackend : public QObject {
public:
  ImageBackend(Database* database, QObject* parent = 0);

  void AddOrUpdateImage(const Image& image);
  void DeleteImage(const Image& image);
  bool FindImage(const Image& image, Image* result);

private:
  Database* database_;
};

#endif // IMAGEBACKEND_H
