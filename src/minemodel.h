/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#ifndef MINEMODEL_H
#define MINEMODEL_H

#include "image.h"

#include <QAbstractItemModel>
#include <QPixmap>

class Application;

class QNetworkAccessManager;
class QNetworkReply;

class MineModel : public QAbstractItemModel {
  Q_OBJECT

public:
  MineModel(Application* app, QObject* parent = 0);

  static const int kLimit;

  enum Role {
    Role_Image = Qt::UserRole + 1
  };

  // QAbstractItemModel
  Qt::ItemFlags flags(const QModelIndex& index) const;
  QVariant data(const QModelIndex& index, int role) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  int rowCount(const QModelIndex& parent = QModelIndex()) const;
  int columnCount(const QModelIndex& parent = QModelIndex()) const;
  bool hasChildren(const QModelIndex& parent = QModelIndex()) const;
  bool canFetchMore(const QModelIndex& parent = QModelIndex()) const;
  void fetchMore(const QModelIndex& parent = QModelIndex());
  QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
  QModelIndex parent(const QModelIndex& child) const;

private slots:
  void ThumbnailFinished(QNetworkReply* reply, int row);

private:
  struct ImageData {
    ImageData() {}
    ImageData(const Image& image) : image_(image) {}

    Image image_;
    QPixmap thumbnail_;
  };

  QPixmap ThumbnailFor(int index);

private:
  Application* app_;
  QNetworkAccessManager* network_;
  QPixmap no_image_;

  QList<ImageData> images_;

  bool at_end_;
};

#endif // MINEMODEL_H
