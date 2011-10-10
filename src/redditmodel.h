/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#ifndef REDDITMODEL_H
#define REDDITMODEL_H

#include "image.h"

#include <QAbstractItemModel>
#include <QPixmap>
#include <QUrl>

class Application;

class QNetworkAccessManager;
class QNetworkReply;
class QWebPage;


class RedditModel : public QAbstractItemModel {
  Q_OBJECT

public:
  RedditModel(const QStringList& reddits, Application* app,
              QObject* parent = 0);

  static const char* kCookieName;
  static const char* kSettingsGroup;
  static const char* kUrl;
  static const char* kUrlForCookies;
  static const int kThumbnailSize;
  static const int kDefaultPreloadNext;
  static const int kDefaultMaxPreloadedPages;

  enum Role {
    Role_Image = Qt::UserRole + 1,
    Role_Page
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
  void FetchMoreFinished(QNetworkReply* reply);
  void ThumbnailFinished(QNetworkReply* reply, int row);

  void ReloadSettings();

private:
  struct Link {
    Link()
      : page_(NULL) {}

    Link(const Image& image)
      : image_(image),
        page_(NULL) {}

    // Data from reddit.
    Image image_;

    // Data we've populated.
    QPixmap thumbnail_;
    QWebPage* page_;
  };

  static QImage ScaleAndPad(const QImage& image);
  QUrl ResolveUrl(const QUrl& url) const;

  QWebPage* GetOrLoadPage(int index);

private:
  Application* app_;

  // Mostly constant data.
  QStringList reddits_;
  QPixmap no_image_;
  QNetworkAccessManager* network_;

  // State.
  bool is_loading_more_;
  bool no_more_links_;
  QList<Link> links_;
  QList<int> mru_pages_;
  QString last_seen_name_;

  // Options.
  bool show_self_posts_;
  bool show_viewed_images_;
  int preload_next_;
  int max_preloaded_pages_;
};

Q_DECLARE_METATYPE(QWebPage*)

#endif // REDDITMODEL_H
