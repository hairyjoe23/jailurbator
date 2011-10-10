/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#include "application.h"
#include "imagelistdelegate.h"
#include "logging.h"
#include "mainwindow.h"
#include "redditmodel.h"
#include "stylesheetloader.h"
#include "subredditmodel.h"
#include "ui_mainwindow.h"

#include <QKeyEvent>
#include <QSettings>

MainWindow::MainWindow(Application* app, QWidget* parent)
  : QMainWindow(parent),
    app_(app),
    ui_(new Ui_MainWindow)
{
  ui_->setupUi(this);
  ui_->list->setItemDelegate(new ImageListDelegate(this));

  statusBar()->hide();
  menuBar()->hide();

  ui_->image_state->Init(app_, ui_->web);

  // Load stylesheet
  StyleSheetLoader* loader = new StyleSheetLoader(this);
  loader->SetStyleSheet(this, ":/mainwindow.css");

  // Create actions
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
