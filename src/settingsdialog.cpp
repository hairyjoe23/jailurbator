/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */

#include "browsingsettingspage.h"
#include "logging.h"
#include "mainwindow.h"
#include "redditloginsettingspage.h"
#include "settingsdialog.h"
#include "subredditsettingspage.h"

#include "ui_settingsdialog.h"

#include <QDesktopWidget>
#include <QPushButton>
#include <QScrollArea>


SettingsDialog::SettingsDialog(QWidget* parent)
  : QDialog(parent),
    ui_(new Ui_SettingsDialog),
    loading_settings_(false)
{
  ui_->setupUi(this);

  AddPage(Page_Browsing, new BrowsingSettingsPage(this));
  AddPage(Page_Login, new RedditLoginSettingsPage(this));
  AddPage(Page_Subreddits, new SubredditSettingsPage(this));

  // List box
  connect(ui_->list, SIGNAL(currentTextChanged(QString)), SLOT(CurrentTextChanged(QString)));
  ui_->list->setCurrentRow(Page_Browsing);

  // Make sure the list is big enough to show all the items
  ui_->list->setMinimumWidth(ui_->list->sizeHintForColumn(0));

  ui_->buttonBox->button(QDialogButtonBox::Cancel)->setShortcut(QKeySequence::Close);
}

SettingsDialog::~SettingsDialog() {
  delete ui_;
}

void SettingsDialog::AddPage(Page id, SettingsPage* page) {
  // Create the list item
  QListWidgetItem* item = new QListWidgetItem(page->windowIcon(),
                                              page->windowTitle());
  ui_->list->addItem(item);

  if (!page->IsEnabled()) {
    item->setFlags(Qt::NoItemFlags);
  }

  // Create a scroll area containing the page
  QScrollArea* area = new QScrollArea;
  area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  area->setWidget(page);
  area->setWidgetResizable(true);
  area->setFrameShape(QFrame::NoFrame);
  area->setMinimumWidth(page->layout()->minimumSize().width());

  // Add the page to the stack
  ui_->stacked_widget->addWidget(area);

  // Remember where the page is
  PageData data;
  data.index_ = ui_->list->row(item);
  data.scroll_area_ = area;
  data.page_ = page;
  pages_[id] = data;
}

void SettingsDialog::accept() {
  // Save settings
  foreach (const PageData& data, pages_.values()) {
    data.page_->Save();
  }

  QDialog::accept();
}

void SettingsDialog::showEvent(QShowEvent* e) {
  // Load settings
  loading_settings_ = true;
  foreach (const PageData& data, pages_.values()) {
    data.page_->Load();
  }
  loading_settings_ = false;

  // Resize the dialog if it's too big
  const QSize available = QApplication::desktop()->availableGeometry(this).size();
  if (available.height() < height()) {
    resize(width(), sizeHint().height());
  }

  QDialog::showEvent(e);
}

void SettingsDialog::OpenAtPage(Page page) {
  if (!pages_.contains(page)) {
    return;
  }

  ui_->list->setCurrentRow(pages_[page].index_);
  show();
}

void SettingsDialog::CurrentTextChanged(const QString& text) {
  ui_->title->setText("<b>" + text + "</b>");
}
