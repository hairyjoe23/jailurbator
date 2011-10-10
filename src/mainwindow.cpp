/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#include "application.h"
#include "iconloader.h"
#include "imagelistdelegate.h"
#include "logging.h"
#include "mainwindow.h"
#include "minemodel.h"
#include "minelistdelegate.h"
#include "redditmodel.h"
#include "stylesheetloader.h"
#include "subredditmodel.h"
#include "ui_mainwindow.h"

#include <QKeyEvent>
#include <QSettings>

MainWindow::MainWindow(Application* app, QWidget* parent)
  : QMainWindow(parent),
    app_(app),
    ui_(new Ui_MainWindow),
    mode_menu_(new QMenu(this))
{
  ui_->setupUi(this);
  ui_->list->setItemDelegate(new ImageListDelegate(this));
  ui_->mine->setItemDelegate(new MineListDelegate(this));

  statusBar()->hide();
  menuBar()->hide();

  ui_->image_state->Init(app_, ui_->web);

  // Load stylesheet
  StyleSheetLoader* loader = new StyleSheetLoader(this);
  loader->SetStyleSheet(this, ":/mainwindow.css");

  // Load icons
  ui_->mode->setIcon(app_->icon_loader()->LoadIcon("screen"));

  // Create actions
  ui_->settings->setDefaultAction(ui_->action_settings);
  ui_->refresh->setDefaultAction(ui_->action_refresh);

  // Mode menu
  QActionGroup* mode_actions = new QActionGroup(this);
  mode_actions->addAction(ui_->action_mode_reddit);
  mode_actions->addAction(ui_->action_mode_mine);

  mode_menu_->addActions(mode_actions->actions());
  ui_->mode->setMenu(mode_menu_);

  RedditMode();
  ui_->action_mode_reddit->setChecked(true);

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
  connect(ui_->action_mode_mine, SIGNAL(triggered()), SLOT(MineMode()));
  connect(ui_->action_mode_reddit, SIGNAL(triggered()), SLOT(RedditMode()));

  ReloadSettings();
}

MainWindow::~MainWindow() {
}

void MainWindow::LinkSelected() {
  const QModelIndex index = ui_->list->currentIndex();
  if (!index.isValid())
    return;

  ui_->web->setPage(index.data(RedditModel::Role_Page).value<QWebPage*>());

  Image image = index.data(RedditModel::Role_Image).value<Image>();
  ui_->image_state->SetCurrentImage(image);
}

void MainWindow::ReloadSettings() {
  RecreateModel();
}

void MainWindow::RecreateModel() {
  // Re-read the list of subscribed subreddits.
  QSettings s;
  s.beginGroup(SubredditModel::kSettingsGroup);

  const QStringList subscribed_subreddits =
      s.value("subscribed_subreddits", SubredditModel::default_subreddits()).toStringList();

  // Recreate the model.
  reddit_model_.reset(new RedditModel(subscribed_subreddits, app_));

  ui_->list->setModel(reddit_model_.data());
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
    ui_->image_state->ToggleLove();
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

void MainWindow::RedditMode() {
  ui_->list_stack->setCurrentWidget(ui_->list_reddit_page);
  ui_->content_stack->setCurrentWidget(ui_->content_reddit_page);

  const QSize max_size = ui_->list_stack->currentWidget()->maximumSize();
  ui_->list_stack->setMaximumSize(max_size);
  ui_->list_frame->setMaximumSize(max_size);
}

void MainWindow::MineMode() {
  mine_model_.reset(new MineModel(app_));
  ui_->mine->setModel(mine_model_.data());

  connect(ui_->mine->selectionModel(),
          SIGNAL(currentChanged(QModelIndex,QModelIndex)),
          SLOT(MineSelected()));

  ui_->list_stack->setCurrentWidget(ui_->list_mine_page);
  ui_->content_stack->setCurrentWidget(ui_->content_mine_page);

  const QSize max_size = ui_->list_stack->currentWidget()->maximumSize();
  ui_->list_stack->setMaximumSize(max_size);
  ui_->list_frame->setMaximumSize(max_size);
}

void MainWindow::MineSelected() {
  const Image image = ui_->mine->currentIndex().data(MineModel::Role_Image).value<Image>();

  QPixmap pixmap;

  const QString cache_key = image.unique_id() +
                            QString::number(ui_->content->width()) +
                            QString::number(ui_->content->height());

  if (!mine_cache_.find(cache_key, &pixmap)) {
    // Load the pixmap
    if (pixmap.load(image.filename())) {
      pixmap = pixmap.scaled(ui_->content->size(), Qt::KeepAspectRatio,
                             Qt::SmoothTransformation);
      mine_cache_.insert(cache_key, pixmap);
    } else {
      return;
    }
  }

  ui_->content->setPixmap(pixmap);
}
