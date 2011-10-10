/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#ifndef IMAGE_H
#define IMAGE_H

#include <QDateTime>
#include <QSharedData>
#include <QStringList>
#include <QUrl>
#include <QVariant>

class QSqlQuery;


class ImageData : public QSharedData {
public:
  ImageData();

  QString reddit_author_;
  uint reddit_created_;
  QString reddit_domain_;
  QString reddit_link_;
  QString reddit_name_;
  int reddit_score_;
  QString reddit_subreddit_;
  QString reddit_title_;
  QByteArray reddit_thumbnail_url_;
  QByteArray reddit_url_;

  uint saved_time_;
  QString filename_;
  bool is_loved_;
};


class Image {
public:
  Image();

  static const QStringList kColumns;
  static const QString kColumnSpec;
  static const QString kBindSpec;
  static const QString kUpdateSpec;

  void InitFromQuery(const QSqlQuery& query);
  void InitFromJson(const QVariantMap& json);

  void BindToQuery(QSqlQuery* query) const;

  // Simple getters
  QString reddit_author() const { return d->reddit_author_; }
  QDateTime reddit_created() const { return QDateTime::fromTime_t(d->reddit_created_); }
  QString reddit_domain() const { return d->reddit_domain_; }
  QString reddit_name() const { return d->reddit_name_; }
  QString reddit_link() const { return d->reddit_link_; }
  int reddit_score() const { return d->reddit_score_; }
  QString reddit_subreddit() const { return d->reddit_subreddit_; }
  QString reddit_title() const { return d->reddit_title_; }
  QUrl reddit_thumbnail_url() const { return QUrl::fromEncoded(d->reddit_thumbnail_url_); }
  QUrl reddit_url() const { return QUrl::fromEncoded(d->reddit_url_); }
  QDateTime saved_time() const { return QDateTime::fromTime_t(d->saved_time_); }
  QString filename() const { return d->filename_; }
  bool is_loved() const { return d->is_loved_; }

  // Simple setters
  void set_reddit_author(const QString& v) { d->reddit_author_ = v; }
  void set_reddit_created(const QDateTime& v) { d->reddit_created_ = v.toTime_t(); }
  void set_reddit_domain(const QString& v) { d->reddit_domain_ = v; }
  void set_reddit_name(const QString& v) { d->reddit_name_ = v; }
  void set_reddit_link(const QString& v) { d->reddit_link_ = v; }
  void set_reddit_score(int v) { d->reddit_score_ = v; }
  void set_reddit_subreddit(const QString& v) { d->reddit_subreddit_ = v; }
  void set_reddit_title(const QString& v) { d->reddit_title_ = v; }
  void set_reddit_thumbnail_url(const QUrl& v) { d->reddit_thumbnail_url_ = v.toEncoded(); }
  void set_reddit_url(const QUrl& v) { d->reddit_url_ = v.toEncoded(); }
  void set_saved_time(const QDateTime& v) { d->saved_time_ = v.toTime_t(); }
  void set_filename(const QString& v) { d->filename_ = v; }
  void set_loved(bool v) { d->is_loved_ = v; }

  // Two images are equal if they are in the same subreddit and have the same
  // full name.
  bool operator ==(const Image& other) const;

  // An image is "true" if it is valid - it has a subreddit and full name set.
  operator bool() const;

private:
  QSharedDataPointer<ImageData> d;
};

Q_DECLARE_METATYPE(Image)

#endif // IMAGE_H
