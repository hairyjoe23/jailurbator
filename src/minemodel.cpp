/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#include "application.h"
#include "closure.h"
#include "database.h"
#include "minemodel.h"
#include "network.h"
#include "redditmodel.h"

#include <QNetworkReply>
#include <QSqlQuery>

const int MineModel::kLimit = 255;

MineModel::MineModel(Application* app, QObject* parent)
  : QAbstractItemModel(parent),
    app_(app),
    network_(new NetworkAccessManager(this)),
    no_image_(QPixmap::fromImage(RedditModel::ScaleAndPad(QImage(":/noimage.png")))),
    at_end_(false)
{
}

Qt::ItemFlags MineModel::flags(const QModelIndex& index) const {
  return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant MineModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.column() != 0)
    return false;

  const ImageData& image = images_[index.row()];

  switch (role) {
  case Qt::DisplayRole:
    return image.image_.reddit_title();

  case Qt::DecorationRole:
    return const_cast<MineModel*>(this)->ThumbnailFor(index.row());

  case Role_Image:
    return QVariant::fromValue(image.image_);

  default:
    break;
  }

  return QVariant();
}

QVariant MineModel::headerData(int section, Qt::Orientation orientation, int role) const {
  return QVariant();
}

int MineModel::rowCount(const QModelIndex& parent) const {
  return images_.count();
}

int MineModel::columnCount(const QModelIndex& parent) const {
  return 1;
}

bool MineModel::hasChildren(const QModelIndex& parent) const {
  return !parent.isValid();
}

bool MineModel::canFetchMore(const QModelIndex& parent) const {
  return !parent.isValid() && !at_end_;
}

void MineModel::fetchMore(const QModelIndex& parent) {
  if (parent.isValid() || at_end_)
    return;

  QSqlQuery query(QString(
      "select " + Image::kColumnSpec + " from images"
      " where is_loved = 'true'"
      " order by saved_time desc"
      " limit %1 offset %2").arg(kLimit).arg(images_.count()), app_->database()->db());

  query.exec();
  if (!app_->database()->CheckErrors(query)) {
    at_end_ = true;
    return;
  }

  QList<ImageData> new_images;
  while (query.next()) {
    Image image;
    image.InitFromQuery(query);

    new_images << ImageData(image);
  }

  if (new_images.isEmpty()) {
    at_end_ = true;
    return;
  }

  beginInsertRows(QModelIndex(), images_.count(),
                  images_.count() + new_images.count() - 1);
  images_.append(new_images);
  endInsertRows();
}

QModelIndex MineModel::index(int row, int column, const QModelIndex& parent) const {
  if (column != 0 || parent.isValid() || row < 0 || row >= images_.count())
    return QModelIndex();
  return createIndex(row, column);
}

QModelIndex MineModel::parent(const QModelIndex& child) const {
  return QModelIndex();
}

QPixmap MineModel::ThumbnailFor(int index) {
  const ImageData& data = images_[index];
  const Image& image = data.image_;

  if (!data.thumbnail_.isNull())
    return data.thumbnail_;

  if (image.reddit_thumbnail_url().scheme().isEmpty())
    return no_image_;

  QNetworkReply* reply = network_->get(QNetworkRequest(image.reddit_thumbnail_url()));

  NewClosure(reply, SIGNAL(finished()),
             this, SLOT(ThumbnailFinished(QNetworkReply*,int)),
             reply, index);

  return no_image_;
}

void MineModel::ThumbnailFinished(QNetworkReply* reply, int row) {
  if (reply->error() != QNetworkReply::NoError) {
    qLog(Warning) << "Error fetching thumbnail" << reply->url();
    return;
  }

  QImage image;
  if (!image.load(reply, NULL)) {
    qLog(Warning) << "Error loading thumbnail" << reply->url();
    return;
  }

  images_[row].thumbnail_ = QPixmap::fromImage(RedditModel::ScaleAndPad(image));
  emit dataChanged(index(row, 0), index(row, 0));
}
