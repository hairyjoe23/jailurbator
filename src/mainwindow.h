/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "image.h"

#include <QMainWindow>
#include <QPixmapCache>
#include <QScopedPointer>
#include <QUrl>

class Application;
class MineModel;
class RedditModel;
class Ui_MainWindow;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(Application* app, QWidget* parent = 0);
  ~MainWindow();

  // QWidget
  void keyPressEvent(QKeyEvent* e);

private slots:
  void ReloadSettings();
  void LinkSelected();
  void MineSelected();

  void RecreateModel();

  void RedditMode();
  void MineMode();

private:
  void SelectNext(int d);

private:
  Application* app_;
  QScopedPointer<Ui_MainWindow> ui_;

  QScopedPointer<RedditModel> reddit_model_;
  QScopedPointer<MineModel> mine_model_;

  QMenu* mode_menu_;

  QPixmapCache mine_cache_;
};

#endif // MAINWINDOW_H
