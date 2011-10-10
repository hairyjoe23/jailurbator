/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#ifndef MINELISTDELEGATE_H
#define MINELISTDELEGATE_H

#include <QStyledItemDelegate>

class MineListDelegate : public QStyledItemDelegate {
public:
  MineListDelegate(QObject* parent = 0);

  static const int kTextMargin;

  QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

};

#endif // MINELISTDELEGATE_H
