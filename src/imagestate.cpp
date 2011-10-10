/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#include "application.h"
#include "iconloader.h"
#include "imagebackend.h"
#include "imagestate.h"
#include "logging.h"
#include "network.h"
#include "redditmodel.h"
#include "utilities.h"
#include "ui_imagestate.h"

#include <QDir>
#include <QFile>
#include <QSettings>
#include <QWebFrame>
#include <QWebView>

ImageState::ImageState(QWidget* parent)
  : QFrame(parent),
    ui_(new Ui_ImageState),
    app_(NULL),
    web_view_(NULL)
{
  ui_->setupUi(this);

  ui_->love->setDefaultAction(ui_->action_love);

  connect(ui_->action_love, SIGNAL(triggered(bool)), SLOT(Love(bool)));

  ReloadSettings();
}

void ImageState::Init(Application* app, QWebView* web_view) {
  Q_ASSERT(!app_);
  app_ = app;
  web_view_ = web_view;

  // Load icons
  ui_->action_love->setIcon(app_->icon_loader()->LoadIcon("heart"));

  // Connect signals
  connect(app_, SIGNAL(SettingsChanged()), SLOT(ReloadSettings()));
}

ImageState::~ImageState() {
}

void ImageState::SetCurrentImage(const Image& image) {
  current_image_ = image;

  // Sanitise title text
  QString title = current_image_.reddit_title();
  title.replace('\n', ' ');

  // Sanitise subreddit name
  QString subreddit = "/r/" + current_image_.reddit_subreddit();
  subreddit.replace('\n', ' ');

  // Update the UI
  ui_->score->setText(QString::number(current_image_.reddit_score()));
  ui_->subreddit->setText(subreddit);
  ui_->title->setText(title);
  ui_->action_love->setChecked(current_image_.is_loved());

  // Mark the image as viewed
  if (!current_image_.is_viewed()) {
    current_image_.set_first_viewed_time(QDateTime::currentDateTime());
    current_image_.set_viewed(true);
    app_->image_backend()->AddOrUpdateImage(current_image_);
  }
}

void ImageState::ToggleLove() {
  Love(!current_image_.is_loved());
}

void ImageState::ReloadSettings() {
  QSettings s;
  s.beginGroup(RedditModel::kSettingsGroup);

  loved_folder_ = s.value("loved_folder", QDir::homePath()).toString();
}

void ImageState::Love(bool on) {
  if (!current_image_)
    return;

  current_image_.set_loved(on);

  // If we're unloving the file then delete the existing one.
  if (!on && !current_image_.filename().isEmpty()) {
    QFile::remove(current_image_.filename());
    current_image_.set_filename(QString());
  }

  // If we're loving the file then save the image.
  if (on) {
    if (findimages_js_source_.isNull()) {
      QFile source(":/findimages.js");
      source.open(QIODevice::ReadOnly);
      findimages_js_source_ = QString::fromUtf8(source.readAll());
    }

    // Find the URL of the image in the page.
    QWebFrame* frame = web_view_->page()->mainFrame();
    QString url = frame->evaluateJavaScript(findimages_js_source_).toString();

    // Save it
    if (!url.isEmpty()) {
      current_image_.set_saved_time(QDateTime::currentDateTime());
      current_image_.set_filename(SaveCurrentImage(QUrl(url)));
    }
  }

  app_->image_backend()->AddOrUpdateImage(current_image_);
}

QString ImageState::SaveCurrentImage(const QUrl& url) const {
  // Get the image out of the network cache.
  QScopedPointer<QIODevice> source(ThreadSafeNetworkDiskCache::Data(url));
  if (!source) {
    qLog(Warning) << url << "not found in network cache";
    return QString();
  }

  // Make a filename for the image.
  const QString extension = url.path().contains('.') ?
        url.path().mid(url.path().lastIndexOf('.') + 1) : "jpg";

  const QString name = current_image_.reddit_title().replace(
        QRegExp("[^a-zA-Z0-9_-]"), "_");

  const QString filename = QString("%1_%2_%3.%4").arg(
        current_image_.reddit_subreddit(),
        current_image_.reddit_name(),
        name, extension);

  QString path = QDir(loved_folder_).absoluteFilePath(filename);

  QFile destination(path);

  if (!Utilities::Copy(source.data(), &destination))
    return QString();

  return path;
}
