/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#ifndef IMAGELISTDELEGATE_H
#define IMAGELISTDELEGATE_H

#include <QStyledItemDelegate>

class ImageListDelegate : public QStyledItemDelegate {
public:
  ImageListDelegate(QObject* parent);

  static const int kSidePadding;
  static const int kTopPadding;

  QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

#endif // IMAGELISTDELEGATE_H
