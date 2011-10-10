#include "subredditmodel.h"
#include "subredditsettingspage.h"
#include "ui_subredditsettingspage.h"

#include <QSettings>
#include <QSortFilterProxyModel>

SubredditSettingsPage::SubredditSettingsPage(SettingsDialog* dialog)
  : SettingsPage(dialog),
    ui_(new Ui_SubredditSettingsPage),
    model_(new SubredditModel(this)),
    proxy_(new QSortFilterProxyModel(this))
{
  ui_->setupUi(this);

  proxy_->setSourceModel(model_);
  proxy_->setDynamicSortFilter(true);
  proxy_->setSortCaseSensitivity(Qt::CaseInsensitive);

  ui_->list->setModel(proxy_);
  ui_->list->header()->resizeSection(0, 200);
  proxy_->sort(1, Qt::DescendingOrder);
}

SubredditSettingsPage::~SubredditSettingsPage() {
  delete ui_;
}

void SubredditSettingsPage::Load() {
  model_->ReloadSettings();
}

void SubredditSettingsPage::Save() {
  QSettings s;
  s.beginGroup(SubredditModel::kSettingsGroup);

  s.setValue("subscribed_subreddits", model_->subscribed_subreddits());
}
