/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QMap>

class SettingsPage;
class Ui_SettingsDialog;

class QScrollArea;

class SettingsDialog : public QDialog {
  Q_OBJECT

public:
  SettingsDialog(QWidget* parent = 0);
  ~SettingsDialog();

  enum Page {
    Page_Browsing,
    Page_Login,
    Page_Subreddits
  };

  bool is_loading_settings() const { return loading_settings_; }

  void OpenAtPage(Page page);

  // QDialog
  void accept();

  // QWidget
  void showEvent(QShowEvent* e);

private slots:
  void CurrentTextChanged(const QString& text);

private:
  struct PageData {
    int index_;
    QScrollArea* scroll_area_;
    SettingsPage* page_;
  };

  void AddPage(Page id, SettingsPage* page);

private:
  Ui_SettingsDialog* ui_;
  bool loading_settings_;

  QMap<Page, PageData> pages_;
};

#endif // SETTINGSDIALOG_H
