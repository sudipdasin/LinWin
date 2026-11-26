// SPDX-License-Identifier: GPL-3.0-or-later
#include <gio/gio.h>

#include "app_launcher_widget.h"
#include "theme_visuals.h"
#include "shell_icons.h"
#include "recent_documents.h"
#include <QDesktopServices>
#include <QFileIconProvider>
#include <QCheckBox>

#include <QAbstractItemView>
#include <QCollator>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QSize>
#include <QSettings>
#include <QVBoxLayout>

#include <algorithm>

namespace {
const QString kPinnedAppsSetting = QStringLiteral("launcher/pinnedApplications");

QString appName(GAppInfo *appInfo)
{
    const gchar *name = g_app_info_get_display_name(appInfo);
    if (name == nullptr || *name == '\0') {
        name = g_app_info_get_name(appInfo);
    }
    return name != nullptr ? QString::fromUtf8(name) : QObject::tr("Unnamed application");
}

QString appDescription(GAppInfo *appInfo)
{
    const gchar *description = g_app_info_get_description(appInfo);
    return description != nullptr ? QString::fromUtf8(description) : QString();
}

QIcon appIcon(GAppInfo *appInfo)
{
    GIcon *icon = g_app_info_get_icon(appInfo);
    if (icon != nullptr && G_IS_THEMED_ICON(icon)) {
        const gchar *const *names = g_themed_icon_get_names(G_THEMED_ICON(icon));
        for (int index = 0; names != nullptr && names[index] != nullptr; ++index) {
            const QIcon candidate = QIcon::fromTheme(QString::fromUtf8(names[index]));
            if (!candidate.isNull()) {
                return candidate;
            }
        }
    }
    if (icon != nullptr && G_IS_FILE_ICON(icon)) {
        GFile *file = g_file_icon_get_file(G_FILE_ICON(icon));
        gchar *path = file != nullptr ? g_file_get_path(file) : nullptr;
        if (path != nullptr) {
            const QIcon fileIcon(QString::fromUtf8(path));
            g_free(path);
            if (!fileIcon.isNull()) {
                return fileIcon;
            }
        }
    }
    return QIcon::fromTheme(QStringLiteral("application-x-executable"));
}
}

AppLauncherWidget::AppLauncherWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(14, 12, 14, 12);
    layout->setSpacing(8);

    auto *brand = new QHBoxLayout;
    auto *logo = new QLabel(this);
    logo->setPixmap(LinWinVisual::logoIcon().pixmap(36,36));
    brand->addWidget(logo);
    auto *title = new QLabel(tr("LinWin"), this);
    title->setObjectName(QStringLiteral("pageTitle"));
    auto *description = new QLabel(
        tr("Find and open applications installed on this computer. Pin the ones you use most."), this);
    description->setObjectName(QStringLiteral("intro"));
    description->setWordWrap(true);

    searchField_ = new QLineEdit(this);
    searchField_->setObjectName(QStringLiteral("applicationSearch"));
    searchField_->setPlaceholderText(tr("Search apps by name or description"));
    searchField_->setClearButtonEnabled(true);
    searchField_->addAction(LinWinVisual::shellIcon(LinWinVisual::Glyph::Search), QLineEdit::LeadingPosition);

    applicationList_ = new QListWidget(this);
    applicationList_->setObjectName(QStringLiteral("applicationList"));
    applicationList_->setSelectionMode(QAbstractItemView::SingleSelection);
    applicationList_->setUniformItemSizes(true);
    applicationList_->setIconSize(QSize(32, 32));

    auto *actions = new QHBoxLayout;
    auto *openButton = new QPushButton(tr("Open"), this);
    pinButton_ = new QPushButton(tr("Pin"), this);
    openButton->setObjectName(QStringLiteral("primaryAction"));
    actions->addWidget(openButton);
    actions->addWidget(pinButton_);
    actions->addStretch();

    statusLabel_ = new QLabel(this);
    statusLabel_->setObjectName(QStringLiteral("launcherStatus"));
    statusLabel_->setWordWrap(true);

    brand->addWidget(title, 1);
    layout->addLayout(brand);
    layout->addWidget(description);
    layout->addWidget(searchField_);
    auto *columns = new QHBoxLayout;
    auto *all = new QVBoxLayout;
    all->addWidget(new QLabel(tr("All apps"), this));
    all->addWidget(applicationList_, 1);
    columns->addLayout(all, 1);
    auto *pins = new QVBoxLayout;
    pins->addWidget(new QLabel(tr("Pinned"), this));
    pinnedList_ = new QListWidget(this);
    pinnedList_->setObjectName(QStringLiteral("pinnedApps"));
    pinnedList_->setViewMode(QListView::IconMode);
    pinnedList_->setResizeMode(QListView::Adjust);
    pinnedList_->setMovement(QListView::Static);
    pinnedList_->setIconSize(QSize(32,32));
    pinnedList_->setGridSize(QSize(90,72));
    pinnedList_->setWordWrap(true);
    pins->addWidget(pinnedList_, 1);
    columns->addLayout(pins, 1);
    layout->addLayout(columns, 1);
    auto *recentHeader=new QHBoxLayout;
    recentHeader->addWidget(new QLabel(tr("Recommended documents"),this));
    auto *recentToggle=new QCheckBox(tr("Show recent"),this);
    recentToggle->setObjectName(QStringLiteral("linwinRecentToggle"));
    recentToggle->setChecked(QSettings().value(QStringLiteral("launcher/showRecent"),true).toBool());
    recentHeader->addStretch(); recentHeader->addWidget(recentToggle);
    layout->addLayout(recentHeader);
    recentList_=new QListWidget(this);
    recentList_->setObjectName(QStringLiteral("recentDocuments"));
    recentList_->setMaximumHeight(100);
    recentList_->setMinimumHeight(44);
    layout->addWidget(recentList_);
    connect(recentToggle,&QCheckBox::toggled,this,[this](bool enabled){
        QSettings().setValue(QStringLiteral("launcher/showRecent"),enabled);
        refreshRecentDocuments();
    });
    connect(recentList_,&QListWidget::itemActivated,this,[this](QListWidgetItem *item){
        if (!item || item->isHidden()) return;
        const QString path=item->data(Qt::UserRole).toString();
        if (!LinWinRecent::allowedDocument(path) || !QDesktopServices::openUrl(QUrl::fromLocalFile(path)))
            statusLabel_->setText(tr("This document could not be opened."));
    });
    connect(pinnedList_, &QListWidget::itemActivated, this, [this](QListWidgetItem *pin) {
        if (!pin || pin->isHidden()) return;
        const auto id = pin->data(Qt::UserRole).toString();
        for (int row=0;row<applicationList_->count();++row) {
            auto *item=applicationList_->item(row);
            if (!item->isHidden() && item->data(Qt::UserRole).toString()==id) {
                applicationList_->setCurrentRow(row); launchSelectedApplication(); break;
            }
        }
    });
    layout->addLayout(actions);
    auto *quick=new QHBoxLayout;
    for (const auto &tool : QList<QPair<int,QString>>{{1,tr("Open Files")},{2,tr("Open Tasks")},{3,tr("Open Clipboard")},{4,tr("Open Settings")}}) {
        auto *button=new QPushButton(tool.second,this);
        button->setObjectName(QStringLiteral("linwinStartTool_%1").arg(tool.first));
        const LinWinVisual::Glyph glyphs[]{LinWinVisual::Glyph::Files,LinWinVisual::Glyph::Tasks,LinWinVisual::Glyph::Clipboard,LinWinVisual::Glyph::Settings};
        button->setIcon(LinWinVisual::shellIcon(glyphs[tool.first-1]));
        button->setIconSize(QSize(18,18));
        quick->addWidget(button);
        connect(button,&QPushButton::clicked,this,[this,tool]{ emit toolRequested(tool.first); });
    }
    layout->addLayout(quick);
    layout->addWidget(statusLabel_);
    refreshRecentDocuments();

    QSettings settings;
    pinnedApplicationIds_ = settings.value(kPinnedAppsSetting).toStringList();
    loadApplications();
    // Seed only a fresh profile, and only from installed applications.
    if (!settings.contains(kPinnedAppsSetting)) {
        const QStringList defaults{QStringLiteral("firefox.desktop"), QStringLiteral("org.mozilla.firefox.desktop"),
            QStringLiteral("thunar.desktop"), QStringLiteral("org.xfce.Thunar.desktop"),
            QStringLiteral("libreoffice-writer.desktop"), QStringLiteral("org.libreoffice.LibreOffice.writer.desktop"),
            QStringLiteral("xfce4-terminal.desktop"), QStringLiteral("org.xfce.mousepad.desktop"),
            QStringLiteral("org.gnome.Calculator.desktop"), QStringLiteral("vlc.desktop")};
        for (const auto &id : defaults) {
            if (applications_.contains(id) && !pinnedApplicationIds_.contains(id)) pinnedApplicationIds_.append(id);
        }
        savePinnedApplications();
    }
    refreshList();

    connect(searchField_, &QLineEdit::textChanged,
            this, &AppLauncherWidget::filterApplications);
    connect(openButton, &QPushButton::clicked,
            this, &AppLauncherWidget::launchSelectedApplication);
    connect(pinButton_, &QPushButton::clicked,
            this, &AppLauncherWidget::togglePinnedApplication);
    connect(applicationList_, &QListWidget::itemActivated,
            this, [this](QListWidgetItem *) { launchSelectedApplication(); });
    connect(applicationList_, &QListWidget::currentRowChanged,
            this, [this](int) { updatePinButton(); });
    connect(searchField_, &QLineEdit::returnPressed,
            this, [this]() {
                if (applicationList_->currentItem() == nullptr) {
                    for (int row = 0; row < applicationList_->count(); ++row) {
                        if (!applicationList_->item(row)->isHidden()) {
                            applicationList_->setCurrentRow(row);
                            break;
                        }
                    }
                }
                launchSelectedApplication();
            });

    updatePinButton();
}

AppLauncherWidget::~AppLauncherWidget()
{
    for (GAppInfo *appInfo : applications_) {
        g_object_unref(appInfo);
    }
}

void AppLauncherWidget::focusSearch()
{
    // Re-enumerate the host desktop registry when Start opens, including new packages.
    loadApplications();
    searchField_->clear();
    refreshList();
    refreshRecentDocuments();
    searchField_->setFocus(Qt::OtherFocusReason);
}

void AppLauncherWidget::refreshRecentDocuments()
{
    recentList_->clear();
    const bool enabled=QSettings().value(QStringLiteral("launcher/showRecent"),true).toBool();
    recentList_->setVisible(enabled);
    if (!enabled) return;
    QFileIconProvider icons;
    for (const auto &entry: LinWinRecent::read(LinWinRecent::registryPath())) {
        auto *item=new QListWidgetItem(icons.icon(QFileInfo(entry.path)),QFileInfo(entry.path).fileName(),recentList_);
        item->setData(Qt::UserRole,entry.path);
        item->setToolTip(QDir::toNativeSeparators(entry.path));
    }
    if (recentList_->count()==0) {
        auto *item=new QListWidgetItem(tr("No recent documents available"),recentList_);
        item->setFlags(Qt::NoItemFlags);
    }
}

void AppLauncherWidget::loadApplications()
{
    // Release previous references so removed applications are no longer offered.
    for (GAppInfo *appInfo : applications_) g_object_unref(appInfo);
    applications_.clear();
    GList *registeredApps = g_app_info_get_all();
    for (GList *node = registeredApps; node != nullptr; node = node->next) {
        auto *appInfo = G_APP_INFO(node->data);
        const gchar *id = g_app_info_get_id(appInfo);
        if (!g_app_info_should_show(appInfo) || id == nullptr || *id == '\0') {
            continue;
        }

        const QString appId = QString::fromUtf8(id);
        if (!applications_.contains(appId)) {
            applications_.insert(appId, G_APP_INFO(g_object_ref(appInfo)));
        }
    }
    g_list_free_full(registeredApps, g_object_unref);
}

void AppLauncherWidget::refreshList()
{
    applicationList_->clear();
    pinnedList_->clear();
    QStringList ids = applications_.keys();
    QCollator collator;
    std::sort(ids.begin(), ids.end(), [this, &collator](const QString &left, const QString &right) {
        const bool leftPinned = pinnedApplicationIds_.contains(left);
        const bool rightPinned = pinnedApplicationIds_.contains(right);
        if (leftPinned != rightPinned) {
            return leftPinned;
        }
        return collator.compare(appName(applications_.value(left)), appName(applications_.value(right))) < 0;
    });

    for (const QString &id : ids) {
        GAppInfo *appInfo = applications_.value(id);
        auto *item = new QListWidgetItem(appIcon(appInfo), appName(appInfo), applicationList_);
        item->setData(Qt::UserRole, id);
        const QString description = appDescription(appInfo);
        item->setToolTip(description.isEmpty() ? appName(appInfo) : description);
        if (pinnedApplicationIds_.contains(id)) {
            item->setText(appName(appInfo) + tr("  •  Pinned"));
            auto *pin = new QListWidgetItem(appIcon(appInfo), appName(appInfo), pinnedList_);
            pin->setData(Qt::UserRole, id);
            pin->setToolTip(description);
        }
    }

    filterApplications(searchField_->text());
    updatePinButton();
    if (applications_.isEmpty()) {
        statusLabel_->setText(tr("No installed applications were found in the desktop application registry."));
    } else {
        statusLabel_->setText(tr("%1 applications found. Double-click an app or select it and choose Open.")
                                  .arg(static_cast<qlonglong>(applications_.size())));
    }
}

void AppLauncherWidget::filterApplications(const QString &query)
{
    const QString needle = query.trimmed();
    int visibleCount = 0;
    for (int row = 0; row < applicationList_->count(); ++row) {
        auto *item = applicationList_->item(row);
        const QString searchable = item->text() + QLatin1Char(' ') + item->toolTip();
        const bool matches = needle.isEmpty() || searchable.contains(needle, Qt::CaseInsensitive);
        item->setHidden(!matches);
        if (matches) {
            ++visibleCount;
        }
    }

    for (int row=0;row<pinnedList_->count();++row) {
        auto *pin=pinnedList_->item(row);
        pin->setHidden(!needle.isEmpty() && !(pin->text()+QLatin1Char(' ')+pin->toolTip()).contains(needle, Qt::CaseInsensitive));
    }
    if (applicationList_->currentItem() != nullptr && applicationList_->currentItem()->isHidden()) {
        applicationList_->setCurrentRow(-1);
    }

    if (!needle.isEmpty() && visibleCount == 0) {
        statusLabel_->setText(tr("No apps match “%1”. Check the spelling or clear the search.").arg(needle));
    } else if (!applications_.isEmpty()) {
        statusLabel_->setText(tr("%1 apps shown. Use the arrow keys and Enter to open one.")
                                  .arg(static_cast<qlonglong>(visibleCount)));
    }
}

void AppLauncherWidget::launchSelectedApplication()
{
    QListWidgetItem *item = applicationList_->currentItem();
    if (item == nullptr || item->isHidden()) {
        statusLabel_->setText(tr("Select a visible app to open."));
        return;
    }

    const QString appId = item->data(Qt::UserRole).toString();
    GAppInfo *appInfo = applications_.value(appId, nullptr);
    if (appInfo == nullptr) {
        statusLabel_->setText(tr("This app is no longer available. Refresh the application list after installing apps."));
        return;
    }

    GAppLaunchContext *launchContext = g_app_launch_context_new();
    GError *error = nullptr;
    const gboolean launched = g_app_info_launch(appInfo, nullptr, launchContext, &error);
    g_object_unref(launchContext);
    if (!launched) {
        const QString message = error != nullptr
            ? QString::fromUtf8(error->message)
            : tr("The desktop did not provide more information.");
        statusLabel_->setText(tr("Could not open %1: %2").arg(appName(appInfo), message));
        if (error != nullptr) {
            g_error_free(error);
        }
        return;
    }

    statusLabel_->setText(tr("Opening %1…").arg(appName(appInfo)));
}

void AppLauncherWidget::togglePinnedApplication()
{
    QListWidgetItem *item = applicationList_->currentItem();
    if (item == nullptr || item->isHidden()) {
        statusLabel_->setText(tr("Select a visible app to pin or unpin."));
        return;
    }

    const QString id = item->data(Qt::UserRole).toString();
    const bool wasPinned = pinnedApplicationIds_.contains(id);
    if (wasPinned) {
        pinnedApplicationIds_.removeAll(id);
    } else {
        pinnedApplicationIds_.prepend(id);
    }
    savePinnedApplications();
    refreshList();
    statusLabel_->setText(wasPinned ? tr("Removed from pinned apps.") : tr("Pinned app to the top of the list."));

    for (int row = 0; row < applicationList_->count(); ++row) {
        if (applicationList_->item(row)->data(Qt::UserRole).toString() == id) {
            applicationList_->setCurrentRow(row);
            break;
        }
    }
}

void AppLauncherWidget::updatePinButton()
{
    const QListWidgetItem *item = applicationList_->currentItem();
    const bool pinned = item != nullptr && pinnedApplicationIds_.contains(item->data(Qt::UserRole).toString());
    pinButton_->setText(pinned ? tr("Unpin") : tr("Pin"));
    pinButton_->setEnabled(item != nullptr);
}

void AppLauncherWidget::savePinnedApplications() const
{
    QSettings settings;
    settings.setValue(kPinnedAppsSetting, pinnedApplicationIds_);
}
