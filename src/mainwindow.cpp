/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#include "application.h"
#include "imagebackend.h"
#include "imagelistdelegate.h"
#include "logging.h"
#include "mainwindow.h"
#include "network.h"
#include "redditmodel.h"
#include "stylesheetloader.h"
#include "subredditmodel.h"
#include "utilities.h"
#include "ui_mainwindow.h"

#include <QDir>
#include <QFile>
#include <QKeyEvent>
#include <QSettings>
#include <QWebFrame>

MainWindow::MainWindow(Application* app, QWidget* parent)
  : QMainWindow(parent),
    app_(app),
    ui_(new Ui_MainWindow)
{
  ui_->setupUi(this);
  ui_->list->setItemDelegate(new ImageListDelegate(this));

  statusBar()->hide();
  menuBar()->hide();

  // Load stylesheet
  StyleSheetLoader* loader = new StyleSheetLoader(this);
  loader->SetStyleSheet(this, ":/mainwindow.css");

  // Load icons
  ui_->action_love->setIcon(LoadIcon("heart"));

  // Create actions
  ui_->love->setDefaultAction(ui_->action_love);
  ui_->settings->setDefaultAction(ui_->action_settings);
  ui_->refresh->setDefaultAction(ui_->action_refresh);

  // Set webkit global attributes
  QWebSettings* web_settings = QWebSettings::globalSettings();
  web_settings->setAttribute(QWebSettings::JavaEnabled, false);
  web_settings->setAttribute(QWebSettings::PluginsEnabled, false);
  web_settings->setAttribute(QWebSettings::PrivateBrowsingEnabled, true);
  web_settings->setAttribute(QWebSettings::JavascriptCanAccessClipboard, false);
  web_settings->setAttribute(QWebSettings::JavascriptCanOpenWindows, false);

  connect(ui_->action_settings, SIGNAL(triggered()),
          app_, SLOT(ShowSettingsDialog()));
  connect(app_, SIGNAL(SettingsChanged()), SLOT(ReloadSettings()));
  connect(ui_->action_refresh, SIGNAL(triggered()), SLOT(RecreateModel()));
  connect(ui_->action_love, SIGNAL(triggered(bool)), SLOT(Love(bool)));

  ReloadSettings();
}

MainWindow::~MainWindow() {
}

QIcon MainWindow::LoadIcon(const QString& name) const {
  QIcon icon;
  icon.addFile(":/icons/normal/" + name + ".png", QSize(), QIcon::Normal, QIcon::Off);
  icon.addFile(":/icons/active/" + name + ".png", QSize(), QIcon::Normal, QIcon::On);
  return icon;
}

void MainWindow::LinkSelected() {
  const QModelIndex index = ui_->list->currentIndex();
  if (!index.isValid())
    return;

  ui_->web->setPage(index.data(RedditModel::Role_Page).value<QWebPage*>());

  Image image = index.data(RedditModel::Role_Image).value<Image>();

  // Is this image in the database already?
  Image db_image;
  if (app_->image_backend()->FindImage(image, &db_image)) {
    image = db_image;
  }

  // Sanitise title text
  QString title = image.reddit_title();
  title.replace('\n', ' ');

  // Sanitise subreddit name
  QString subreddit = "/r/" + image.reddit_subreddit();
  subreddit.replace('\n', ' ');

  ui_->score->setText(QString::number(image.reddit_score()));
  ui_->subreddit->setText(subreddit);
  ui_->title->setText(title);
  ui_->action_love->setChecked(image.is_loved());

  current_image_ = image;
}

void MainWindow::ReloadSettings() {
  QSettings s;
  s.beginGroup(RedditModel::kSettingsGroup);

  loved_folder_ = s.value("loved_folder", QDir::homePath()).toString();

  RecreateModel();
}

void MainWindow::RecreateModel() {
  // Re-read the list of subscribed subreddits.
  QSettings s;
  s.beginGroup(SubredditModel::kSettingsGroup);

  const QStringList subscribed_subreddits =
      s.value("subscribed_subreddits", SubredditModel::default_subreddits()).toStringList();

  // Recreate the model.
  model_.reset(new RedditModel(subscribed_subreddits, app_));

  ui_->list->setModel(model_.data());
  connect(ui_->list->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
          SLOT(LinkSelected()));
}

void MainWindow::keyPressEvent(QKeyEvent* e) {
  if ((e->key() == Qt::Key_Escape) ||
      (e->key() == Qt::Key_W && e->modifiers() & Qt::ControlModifier) ||
      (e->key() == Qt::Key_Q && e->modifiers() & Qt::ControlModifier)) {
    close();
    e->accept();
  } else if ((e->key() == Qt::Key_Space) ||
             (e->key() == Qt::Key_PageDown) ||
             (e->key() == Qt::Key_Down) ||
             (e->key() == Qt::Key_Forward)) {
    SelectNext(1);
  } else if ((e->key() == Qt::Key_Up) ||
             (e->key() == Qt::Key_PageUp) ||
             (e->key() == Qt::Key_Back)) {
    SelectNext(-1);
  } else if ((e->key() == Qt::Key_Enter) ||
             (e->key() == Qt::Key_Return)) {
    Love(!current_image_.is_loved());
  }
}

void MainWindow::SelectNext(int d) {
  if (ui_->list->model()->rowCount() == 0)
    return;

  const QModelIndex index = ui_->list->selectionModel()->currentIndex();
  const int row = index.row() + d;

  ui_->list->selectionModel()->setCurrentIndex(
        index.sibling(qBound(0, row, ui_->list->model()->rowCount()), 0),
        QItemSelectionModel::ClearAndSelect);
}

void MainWindow::Love(bool on) {
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
    QWebFrame* frame = ui_->web->page()->mainFrame();
    QString url = frame->evaluateJavaScript(findimages_js_source_).toString();

    // Save it
    if (!url.isEmpty()) {
      current_image_.set_filename(SaveCurrentImage(QUrl(url)));
    }
  }

  app_->image_backend()->AddOrUpdateImage(current_image_);
}

QString MainWindow::SaveCurrentImage(const QUrl& url) const {
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
