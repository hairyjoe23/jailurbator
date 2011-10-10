/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#include "imagelistdelegate.h"
#include "minelistdelegate.h"
#include "minemodel.h"
#include "redditmodel.h"
#include "utilities.h"

#include <QPainter>

const int MineListDelegate::kTextMargin = 6;

MineListDelegate::MineListDelegate(QObject* parent)
  : QStyledItemDelegate(parent)
{
}

QSize MineListDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const {
  return QSize(ImageListDelegate::kSidePadding * 2 + RedditModel::kThumbnailSize,
               ImageListDelegate::kTopPadding  * 2 + RedditModel::kThumbnailSize);
}

void MineListDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                             const QModelIndex& index) const {
  const QPixmap thumbnail(index.data(Qt::DecorationRole).value<QPixmap>());
  const Image image(index.data(MineModel::Role_Image).value<Image>());

  if (option.state & QStyle::State_Selected) {
    painter->fillRect(option.rect, option.palette.brush(QPalette::Highlight));
  } else {
    painter->fillRect(option.rect, option.palette.brush(QPalette::Base));
  }

  // Fonts
  QFont title_font(option.font);
  title_font.setBold(true);

  QPen title_pen(option.palette.color(QPalette::Active,
      option.state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Text));
  QPen caption_pen(option.palette.color(QPalette::Disabled,
      option.state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Text));

  // Draw the thumbnail
  QRect thumbnail_rect(option.rect.left() + ImageListDelegate::kSidePadding,
                       option.rect.top() + ImageListDelegate::kTopPadding,
                       RedditModel::kThumbnailSize, RedditModel::kThumbnailSize);

  painter->drawPixmap(thumbnail_rect, thumbnail);

  const int line_height = option.fontMetrics.height();

  // First line
  QRect line1(thumbnail_rect.right() + kTextMargin, thumbnail_rect.top(),
              0, line_height);
  line1.setWidth(option.rect.right() - line1.left());

  painter->setFont(title_font);
  painter->setPen(title_pen);
  painter->drawText(line1, Qt::AlignVCenter, image.reddit_title());

  // Second line
  QRect line2(line1.left(), line1.bottom(), line1.width(), line1.height());
  painter->setFont(option.font);
  painter->setPen(caption_pen);
  painter->drawText(line2, Qt::AlignVCenter, QString("/r/%1 - %2").arg(
                      image.reddit_subreddit(), image.reddit_author()));

  // Third line
  QRect line3(line2.left(), line2.bottom(), line2.width(), line2.height());
  painter->setFont(option.font);
  painter->setPen(caption_pen);
  painter->drawText(line3, Qt::AlignVCenter, Utilities::Ago(
                      image.saved_time().toTime_t(), QLocale::system()));
}


