/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#include "database.h"
#include "imagebackend.h"
#include "scopedtransaction.h"

#include <QSqlQuery>

ImageBackend::ImageBackend(Database* database, QObject* parent)
  : QObject(parent),
    database_(database)
{
}

void ImageBackend::AddOrUpdateImage(const Image& image) {
  ScopedTransaction t(&database_->db());

  // Find the image
  QSqlQuery query("select ROWID from images"
                  " where reddit_name = :reddit_name"
                  "   and reddit_subreddit = :reddit_subreddit", database_->db());

  query.bindValue(":reddit_name", image.reddit_name());
  query.bindValue(":reddit_subreddit", image.reddit_subreddit());
  query.exec();
  if (!database_->CheckErrors(query))
    return;

  if (query.next()) {
    // The image already exists - update it
    QSqlQuery update("update images set " + Image::kUpdateSpec +
                     " where reddit_name = :w_reddit_name"
                     "   and reddit_subreddit = :w_reddit_subreddit", database_->db());

    image.BindToQuery(&update);
    update.bindValue(":w_reddit_name", image.reddit_name());
    update.bindValue(":w_reddit_subreddit", image.reddit_subreddit());
    update.exec();
    if (!database_->CheckErrors(update))
      return;
  } else {
    // The image doesn't exist - create it
    QSqlQuery insert("insert into images (" + Image::kColumnSpec + ")"
                     " values (" + Image::kBindSpec + ")", database_->db());

    image.BindToQuery(&insert);
    insert.exec();
    if (!database_->CheckErrors(insert))
      return;
  }

  t.Commit();
}

void ImageBackend::DeleteImage(const Image& image) {
  ScopedTransaction t(&database_->db());

  QSqlQuery query("delete from images"
                  " where reddit_name = :reddit_name"
                  "   and reddit_subreddit = :reddit_subreddit", database_->db());
  query.bindValue(":reddit_name", image.reddit_name());
  query.bindValue(":reddit_subreddit", image.reddit_subreddit());
  query.exec();
  if (!database_->CheckErrors(query))
    return;

  t.Commit();
}

bool ImageBackend::FindImage(const Image& image, Image* result) {
  QSqlQuery query("select " + Image::kColumnSpec + " from images"
                  " where reddit_name = :reddit_name"
                  "   and reddit_subreddit = :reddit_subreddit", database_->db());
  query.bindValue(":reddit_name", image.reddit_name());
  query.bindValue(":reddit_subreddit", image.reddit_subreddit());
  query.exec();
  if (!database_->CheckErrors(query))
    return false;

  if (!query.next())
    return false;

  result->InitFromQuery(query);
  return true;
}
