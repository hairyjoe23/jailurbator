/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#ifndef REDDITLOGINSETTINGSPAGE_H
#define REDDITLOGINSETTINGSPAGE_H

#include "settingspage.h"

#include <QScopedPointer>

class Ui_RedditLoginSettingsPage;

class QNetworkAccessManager;
class QNetworkReply;

class RedditLoginSettingsPage : public SettingsPage {
  Q_OBJECT

public:
  RedditLoginSettingsPage(SettingsDialog* dialog);
  ~RedditLoginSettingsPage();

  static const char* kLoginUrl;

  void Load();
  void Save();

private slots:
  void Login();
  void Logout();

  void LoginFinished(QNetworkReply* reply);

private:
  void UpdateLogoutText(const QString& username);

private:
  QScopedPointer<Ui_RedditLoginSettingsPage> ui_;
  QNetworkAccessManager* network_;
};

#endif // REDDITLOGINSETTINGSPAGE_H
