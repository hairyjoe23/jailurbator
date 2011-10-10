/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#include "image.h"

#include <QSqlQuery>


static QStringList Prepend(const QString& text, const QStringList& list) {
  QStringList ret(list);
  for (int i=0 ; i<ret.count() ; ++i)
    ret[i].prepend(text);
  return ret;
}

static QStringList Updateify(const QStringList& list) {
  QStringList ret(list);
  for (int i=0 ; i<ret.count() ; ++i)
    ret[i].prepend(ret[i] + " = :");
  return ret;
}


const QStringList Image::kColumns = QStringList()
    << "reddit_author" << "reddit_created" << "reddit_domain" << "reddit_link"
    << "reddit_name" << "reddit_score" << "reddit_subreddit" << "reddit_title"
    << "reddit_thumbnail_url" << "reddit_url" << "saved_time" << "filename"
    << "is_loved" << "is_viewed" << "first_viewed_time";

const QString Image::kColumnSpec = Image::kColumns.join(", ");
const QString Image::kBindSpec = Prepend(":", Image::kColumns).join(", ");
const QString Image::kUpdateSpec = Updateify(Image::kColumns).join(", ");


ImageData::ImageData()
  : reddit_created_(0),
    reddit_score_(0),
    saved_time_(0),
    is_loved_(false),
    is_viewed_(false),
    first_viewed_time_(0)
{
}

Image::Image()
  : d(new ImageData)
{
}

void Image::InitFromJson(const QVariantMap& json) {
#define V(type, field) json.value(#field, QVariant()).to##type()

  d->reddit_author_ = V(String, author);
  d->reddit_created_ = V(UInt, created_utc);
  d->reddit_domain_ = V(String, domain);
  d->reddit_link_ = V(String, permalink);
  d->reddit_name_ = V(String, name);
  d->reddit_score_ = V(Int, score);
  d->reddit_subreddit_ = V(String, subreddit);
  d->reddit_title_ = V(String, title);
  d->reddit_thumbnail_url_ = V(ByteArray, thumbnail);
  d->reddit_url_ = V(ByteArray, url);

#undef V
}

void Image::InitFromQuery(const QSqlQuery& query) {
  d->reddit_author_ = query.value(0).toString();
  d->reddit_created_ = query.value(1).toUInt();
  d->reddit_domain_ = query.value(2).toString();
  d->reddit_link_ = query.value(3).toString();
  d->reddit_name_ = query.value(4).toString();
  d->reddit_score_ = query.value(5).toInt();
  d->reddit_subreddit_ = query.value(6).toString();
  d->reddit_title_ = query.value(7).toString();
  d->reddit_thumbnail_url_ = query.value(8).toByteArray();
  d->reddit_url_ = query.value(9).toByteArray();

  d->saved_time_ = query.value(10).toUInt();
  d->filename_ = query.value(11).toString();
  d->is_loved_ = query.value(12).toBool();
  d->is_viewed_ = query.value(13).toBool();
  d->first_viewed_time_ = query.value(14).toUInt();
}

void Image::BindToQuery(QSqlQuery* query) const {
#define BIND(field) query->bindValue(":" #field, d->field##_)

  BIND(reddit_author);
  BIND(reddit_created);
  BIND(reddit_domain);
  BIND(reddit_link);
  BIND(reddit_name);
  BIND(reddit_score);
  BIND(reddit_subreddit);
  BIND(reddit_title);
  BIND(reddit_thumbnail_url);
  BIND(reddit_url);

  BIND(saved_time);
  BIND(filename);
  BIND(is_loved);
  BIND(is_viewed);
  BIND(first_viewed_time);

#undef BIND
}

QString Image::unique_id() const {
  return reddit_name() + reddit_subreddit();
}

bool Image::operator ==(const Image& other) const {
  return unique_id() == other.unique_id();
}

Image::operator bool() const {
  return !reddit_name().isEmpty() && !reddit_subreddit().isEmpty();
}
