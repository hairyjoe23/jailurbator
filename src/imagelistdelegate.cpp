/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#include "imagelistdelegate.h"
#include "redditmodel.h"

#include <QPainter>

const int ImageListDelegate::kSidePadding = 6;
const int ImageListDelegate::kTopPadding = 1;

ImageListDelegate::ImageListDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
{
}

QSize ImageListDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const {
  return QSize(kSidePadding + RedditModel::kThumbnailSize + kSidePadding,
               kTopPadding  + RedditModel::kThumbnailSize + kTopPadding);
}

void ImageListDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                              const QModelIndex& index) const {
  QPixmap thumbnail(index.data(Qt::DecorationRole).value<QPixmap>());

  if (option.state & QStyle::State_Selected) {
    painter->fillRect(option.rect, option.palette.brush(QPalette::Highlight));
  } else {
    painter->fillRect(option.rect, option.palette.brush(QPalette::Base));
  }

  QRect thumbnail_rect(0, 0, RedditModel::kThumbnailSize, RedditModel::kThumbnailSize);
  thumbnail_rect.moveCenter(option.rect.center());

  painter->drawPixmap(thumbnail_rect, thumbnail);
}
