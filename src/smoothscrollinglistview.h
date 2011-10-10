/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#ifndef SMOOTHSCROLLINGLISTVIEW_H
#define SMOOTHSCROLLINGLISTVIEW_H

#include <QListView>

class QPropertyAnimation;

class SmoothScrollingListView : public QListView {
  Q_OBJECT

public:
  SmoothScrollingListView(QWidget* parent = 0);

protected:
  void currentChanged(const QModelIndex& current, const QModelIndex& previous);

private slots:
  void ScrollTo(int index, bool smooth = true);

private:
  int current_index_;
  QPropertyAnimation* scroll_animation_;
};

#endif // SMOOTHSCROLLINGLISTVIEW_H
