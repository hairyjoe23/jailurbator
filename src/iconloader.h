/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#ifndef ICONLOADER_H
#define ICONLOADER_H

#include <QIcon>
#include <QString>

class IconLoader {
public:
  IconLoader();

  QIcon LoadIcon(const QString& name) const;

private:
  Q_DISABLE_COPY(IconLoader)
};

#endif // ICONLOADER_H
