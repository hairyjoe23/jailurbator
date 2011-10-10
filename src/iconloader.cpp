/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#include "iconloader.h"

IconLoader::IconLoader() {
}

QIcon IconLoader::LoadIcon(const QString& name) const {
  QIcon icon;
  icon.addFile(":/icons/normal/" + name + ".png", QSize(), QIcon::Normal, QIcon::Off);
  icon.addFile(":/icons/active/" + name + ".png", QSize(), QIcon::Normal, QIcon::On);
  return icon;
}
