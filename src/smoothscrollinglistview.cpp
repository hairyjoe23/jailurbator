/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#include "smoothscrollinglistview.h"

#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QScrollBar>

SmoothScrollingListView::SmoothScrollingListView(QWidget* parent)
  : QListView(parent),
    current_index_(-1),
    scroll_animation_(new QPropertyAnimation(verticalScrollBar(), "value", this))
{
  scroll_animation_->setDuration(200);
  scroll_animation_->setEasingCurve(QEasingCurve::Linear);
}

void SmoothScrollingListView::currentChanged(const QModelIndex& current,
                                             const QModelIndex& previous) {
  QListView::currentChanged(current, previous);

  if (current.isValid() && current.row() != current_index_) {
    // Scroll to the item
    ScrollTo(current.row());
  }
}

void SmoothScrollingListView::ScrollTo(int index, bool smooth) {
  current_index_ = qBound(0, index, model()->rowCount());

  const int current_y = verticalOffset();
  const int target_y = current_y +
      visualRect(model()->index(index, 0)).center().y() -
      height() / 2;

  if (current_y == target_y)
    return;

  if (smooth) {
    if (scroll_animation_->state() == QAbstractAnimation::Running) {
      scroll_animation_->stop();
    }
    scroll_animation_->setStartValue(current_y);
    scroll_animation_->setEndValue(target_y);
    scroll_animation_->start();
  } else {
    scroll_animation_->stop();
    verticalScrollBar()->setValue(target_y);
  }
}
