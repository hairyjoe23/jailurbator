/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#ifndef APPLICATION_H
#define APPLICATION_H

#include "settingsdialog.h"

#include <QObject>

class Database;
class IconLoader;
class ImageBackend;

class Application : public QObject {
  Q_OBJECT

public:
  Application();
  ~Application();

  // Things that are created on startup.
  Database* database() const { return database_; }
  IconLoader* icon_loader() const { return icon_loader_; }
  ImageBackend* image_backend() const { return image_backend_; }

  // Things that are created lazily.
  SettingsDialog* settings_dialog();

public slots:
  void ShowSettingsDialog();
  void ShowSettingsDialogAtPage(SettingsDialog::Page page);

signals:
  void SettingsChanged();

private:
  Database* database_;
  IconLoader* icon_loader_;
  ImageBackend* image_backend_;
  SettingsDialog* settings_dialog_;
};

#endif // APPLICATION_H
