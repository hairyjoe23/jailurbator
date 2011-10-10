/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#include "browsingsettingspage.h"
#include "redditmodel.h"
#include "ui_browsingsettingspage.h"

#include <QFileDialog>
#include <QSettings>

BrowsingSettingsPage::BrowsingSettingsPage(SettingsDialog* dialog)
  : SettingsPage(dialog),
    ui_(new Ui_BrowsingSettingsPage)
{
  ui_->setupUi(this);

  connect(ui_->loved_browse, SIGNAL(clicked()), SLOT(BrowseLoved()));
}

BrowsingSettingsPage::~BrowsingSettingsPage() {
  delete ui_;
}

void BrowsingSettingsPage::Load() {
  QSettings s;
  s.beginGroup(RedditModel::kSettingsGroup);

  ui_->show_self->setChecked(s.value("show_self_posts", false).toBool());
  ui_->preload_next->setValue(s.value("preload_next", RedditModel::kDefaultPreloadNext).toInt());
  ui_->max_preloaded_pages->setValue(s.value("max_preloaded_pages", RedditModel::kDefaultMaxPreloadedPages).toInt());
  ui_->cache_size->setValue(s.value("cache_size", 100).toInt());
  ui_->loved_folder->setText(QDir::toNativeSeparators(
      s.value("loved_folder", QDir::homePath()).toString()));
}

void BrowsingSettingsPage::Save() {
  QSettings s;
  s.beginGroup(RedditModel::kSettingsGroup);

  s.setValue("show_self_posts",ui_->show_self->isChecked());
  s.setValue("preload_next", ui_->preload_next->value());
  s.setValue("max_preloaded_pages", ui_->max_preloaded_pages->value());
  s.setValue("cache_size", ui_->cache_size->value());
  s.setValue("loved_folder", QDir::fromNativeSeparators(ui_->loved_folder->text()));
}

void BrowsingSettingsPage::BrowseLoved() {
  const QString path = QFileDialog::getExistingDirectory(
        this, "Loved images directory",
        QDir::fromNativeSeparators(ui_->loved_folder->text()));

  if (path.isNull())
    return;

  ui_->loved_folder->setText(QDir::toNativeSeparators(path));
}
