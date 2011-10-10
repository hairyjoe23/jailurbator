/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#include "application.h"
#include "closure.h"
#include "imagebackend.h"
#include "network.h"
#include "redditmodel.h"

#include <QNetworkCookieJar>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPainter>
#include <QSettings>
#include <QWebFrame>
#include <QWebPage>

#include <qjson/parser.h>


const char* RedditModel::kCookieName = "reddit_session";
const char* RedditModel::kSettingsGroup = "Reddit";
const char* RedditModel::kUrl = "http://www.reddit.com/r/%1.json";
const char* RedditModel::kUrlForCookies = "http://www.reddit.com/";
const int RedditModel::kThumbnailSize = 70;
const int RedditModel::kDefaultPreloadNext = 15;
const int RedditModel::kDefaultMaxPreloadedPages = 50;


RedditModel::RedditModel(const QStringList& reddits, Application* app,
                         QObject* parent)
  : QAbstractItemModel(parent),
    app_(app),
    reddits_(reddits),
    no_image_(QPixmap::fromImage(ScaleAndPad(QImage(":/noimage.png")))),
    network_(new NetworkAccessManager(this)),
    is_fetching_more_(false),
    no_more_links_(false),
    show_self_posts_(false),
    show_viewed_images_(false),
    preload_next_(kDefaultPreloadNext),
    max_preloaded_pages_(kDefaultMaxPreloadedPages)
{
  // Read the user's cookie
  QSettings s;
  s.beginGroup(kSettingsGroup);
  const QString cookie = s.value("cookie").toString();

  if (cookie.isEmpty()) {
    // Not-logged in users are only allowed to get one subreddit.
    while (reddits_.size() > 1) {
      reddits_.removeLast();
    }
  } else {
    network_->cookieJar()->setCookiesFromUrl(
          QList<QNetworkCookie>() << QNetworkCookie(kCookieName, cookie.toAscii()),
          QUrl(kUrlForCookies));
  }

  connect(app_, SIGNAL(SettingsChanged()), SLOT(ReloadSettings()));
  ReloadSettings();
}

Qt::ItemFlags RedditModel::flags(const QModelIndex& index) const {
  return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant RedditModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid())
    return QVariant();

  const Link& link = links_[index.row()];

  switch (role) {
  case Qt::DecorationRole:
    return link.thumbnail_.isNull() ? no_image_ : link.thumbnail_;

  case Role_Image:
    return QVariant::fromValue(link.image_);

  case Role_Page: {
    RedditModel* this_nc = const_cast<RedditModel*>(this);

    // Load this page
    QWebPage* page = this_nc->GetOrLoadPage(index.row());

    // Preload the next few pages as well.
    for (int i=index.row()+1 ; i<index.row()+preload_next_ && i<rowCount() ; ++i) {
      this_nc->GetOrLoadPage(i);
    }

    return QVariant::fromValue(page);
  }

  default:
    break;
  }

  return QVariant();
}

QVariant RedditModel::headerData(int section, Qt::Orientation orientation, int role) const {
  return QVariant();
}

int RedditModel::rowCount(const QModelIndex& parent) const {
  return links_.count();
}

int RedditModel::columnCount(const QModelIndex& parent) const {
  return 1;
}

bool RedditModel::hasChildren(const QModelIndex& parent) const {
  if (parent.isValid())
    return false;
  return true;
}

bool RedditModel::canFetchMore(const QModelIndex& parent) const {
  if (parent.isValid())
    return false;
  if (reddits_.isEmpty())
    return false;
  return !no_more_links_ && !is_fetching_more_;
}

QModelIndex RedditModel::index(int row, int column, const QModelIndex& parent) const {
  if (row < 0 || row >= links_.count() || column != 0 || parent.isValid())
    return QModelIndex();
  return createIndex(row, column);
}

QModelIndex RedditModel::parent(const QModelIndex& child) const {
  return QModelIndex();
}

void RedditModel::fetchMore(const QModelIndex& parent) {
  if (parent.isValid())
    return;
  if (is_fetching_more_)
    return;

  is_fetching_more_ = true;

  // Construct the URL
  QUrl url(QString(kUrl).arg(reddits_.join("+")));
  if (!last_seen_name_.isEmpty()) {
    url.addQueryItem("after", last_seen_name_);
  }

  qLog(Debug) << "Fetching links" << url;

  // Send the request
  QNetworkRequest request(url);
  request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                       QNetworkRequest::PreferNetwork);
  QNetworkReply* reply = network_->get(request);
  NewClosure(reply, SIGNAL(finished()),
             this,  SLOT(FetchMoreFinished(QNetworkReply*)),
             reply);
}

void RedditModel::FetchMoreFinished(QNetworkReply* reply) {
  is_fetching_more_ = false;

  if (reply->error() != QNetworkReply::NoError) {
    qLog(Warning) << "Error fetching links" << reply->url();
    no_more_links_ = true;
    return;
  }

  // Parse the JSON response
  QJson::Parser parser;
  QVariantMap data = parser.parse(reply).toMap();
  if (!data.contains("data") || !data["data"].toMap().contains("children")) {
    qLog(Warning) << "Error parsing link response" << reply->url();
    no_more_links_ = true;
    return;
  }

  QVariantList links = data["data"].toMap()["children"].toList();

  if (links.isEmpty()) {
    no_more_links_ = true;
    return;
  }

  QList<Link> new_links;
  int row = links_.count();
  foreach (const QVariant& link_variant, links) {
    QVariantMap link_data = link_variant.toMap();
    if (!link_data.contains("data"))
      continue;

    link_data = link_data["data"].toMap();

    Image image;
    image.InitFromJson(link_data);

    // Update the last seen name - this must be done before skipping anything
    // for the case that we skip all the images in this set.
    last_seen_name_ = image.reddit_name();

    // Skip self posts
    if (!show_self_posts_ && link_data["is_self"].toBool())
      continue;

    // Check if this image has been viewed before
    if (app_->image_backend()->FindImage(image, &image)) {
      if (image.is_viewed() && !show_viewed_images_)
        continue;
    }

    // Adjust the URL to get just the image if we can.
    image.set_reddit_url(ResolveUrl(image.reddit_url()));

    // Create a Link object
    new_links << Link(image);

    // Start fetching the thumbnail
    if (!image.reddit_thumbnail_url().scheme().isEmpty()) {
      qLog(Debug) << "Fetching thumbnail" << image.reddit_thumbnail_url();

      QNetworkReply* reply = network_->get(QNetworkRequest(image.reddit_thumbnail_url()));
      NewClosure(reply, SIGNAL(finished()),
                 this, SLOT(ThumbnailFinished(QNetworkReply*,int)),
                 reply, row);
    }

    row ++;
  }

  // Insert the rows into the model
  const int begin = links_.count();
  const int end   = links_.count() + new_links.count() - 1;

  beginInsertRows(QModelIndex(), begin, end);
  links_ << new_links;
  endInsertRows();
}

QUrl RedditModel::ResolveUrl(const QUrl& url) const {
  if (url.host() == "i.imgur.com") {
    return url;
  }

  if (url.host() == "imgur.com" && !url.path().startsWith("/a/")) {
    QUrl ret(url);
    ret.setHost("i.imgur.com");
    ret.setPath(ret.path() + ".jpg");
    return ret;
  }

  if (url.host().endsWith("yrimg.com") && url.path().endsWith(".php")) {
    QUrl ret(url);
    ret.setPath("/up" + ret.path().left(ret.path().length() - 4));
    return ret;
  }

  return url;
}

void RedditModel::ThumbnailFinished(QNetworkReply* reply, int row) {
  if (reply->error() != QNetworkReply::NoError) {
    qLog(Warning) << "Error fetching thumbnail" << reply->url();
    return;
  }

  QImage image;
  if (!image.load(reply, NULL)) {
    qLog(Warning) << "Error loading thumbnail" << reply->url();
    return;
  }

  links_[row].thumbnail_ = QPixmap::fromImage(ScaleAndPad(image));
  emit dataChanged(index(row, 0), index(row, 0));
}

QImage RedditModel::ScaleAndPad(const QImage& image) {
  if (image.isNull())
    return QImage();

  const QSize target_size = QSize(kThumbnailSize, kThumbnailSize);

  if (image.size() == target_size)
    return image;

  // Scale the image down
  QImage copy;
  copy = image.scaled(target_size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

  // Pad the image to kHeight x kHeight
  if (copy.size() == target_size)
    return copy;

  QImage padded_image(target_size, QImage::Format_ARGB32);
  padded_image.fill(0);

  QPainter p(&padded_image);
  p.drawImage((kThumbnailSize - copy.width()) / 2, (kThumbnailSize - copy.height()) / 2,
              copy);
  p.end();

  return padded_image;
}

QWebPage* RedditModel::GetOrLoadPage(int index) {
  Link& link = links_[index];

  const int mru_index = mru_pages_.indexOf(index);
  if (mru_index != -1) {
    // The page was already loaded, move it to the front of the MRU list.
    mru_pages_.insert(0, mru_pages_.takeAt(mru_index));
  } else {
    // Page has not been loaded yet.
    link.page_ = new QWebPage(this);
    link.page_->setNetworkAccessManager(network_);
    link.page_->mainFrame()->load(link.image_.reddit_url());
    mru_pages_.insert(0, index);

    if (mru_pages_.count() >= max_preloaded_pages_) {
      // Destroy the least recently used page.
      const int lru_index = mru_pages_.takeLast();
      delete links_[lru_index].page_;
      links_[lru_index].page_ = NULL;
    }
  }

  return link.page_;
}

void RedditModel::ReloadSettings() {
  QSettings s;
  s.beginGroup(kSettingsGroup);

  show_self_posts_ = s.value("show_self_posts", false).toBool();
  show_viewed_images_ = s.value("show_viewed_images", false).toBool();
  preload_next_ = s.value("preload_next", kDefaultPreloadNext).toInt();
  max_preloaded_pages_ = s.value("max_preloaded_pages", kDefaultMaxPreloadedPages).toInt();
  qint64 cache_size = s.value("cache_size", 100).toInt();

  ThreadSafeNetworkDiskCache::SetMaximumCacheSize(cache_size * 1000000ll);
}
