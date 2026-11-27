// SPDX-License-Identifier: GPL-3.0-or-later
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>
#include "pin_suggestions.h"

#include "app_launcher_widget.h"
#include "theme_visuals.h"
#include "shell_icons.h"
#include "recent_documents.h"
#include "pinned_tile_delegate.h"
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
#include <QStackedWidget>
#include <QFrame>
#include <QStyleOption>
#include <QStyle>
#include <QMenu>
#include <QFileInfo>
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QResizeEvent>
#include <QButtonGroup>
#include <QKeyEvent>
#include <QShortcut>
#include <QEvent>

#include <algorithm>
#include <utility>

namespace {
class PinnedGrid final:public QListWidget {
public:
    using QListWidget::QListWidget;
protected:
    void resizeEvent(QResizeEvent *event) override {
        QListWidget::resizeEvent(event);
        // QListView reserves a style-dependent scrollbar and layout inset.
        // An exact viewport/4 division previously produced only three real cells.
        const int scrollbar=style()->pixelMetric(QStyle::PM_ScrollBarExtent,nullptr,this);
        const int usable=qMax(1,qMin(viewport()->width(),contentsRect().width()-scrollbar)-12);
        const int columns=qBound(1,usable/72,4);
        const QSize target(qMax(1,usable/columns),106);
        if(gridSize()!=target)setGridSize(target);
    }
};
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
    return QIcon::fromTheme(QStringLiteral("application-x-executable"),LinWinVisual::shellIcon(LinWinVisual::Glyph::Desktop));
}
}

AppLauncherWidget::AppLauncherWidget(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground,true);
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 14, 16, 12);
    layout->setSpacing(10);

    auto *brand = new QHBoxLayout;
    auto *logo = new QLabel(this);
    logo->setPixmap(LinWinVisual::logoIcon().pixmap(36,36));
    brand->addWidget(logo);
    auto *title = new QLabel(tr("LinWin"), this);
    title->setAccessibleName(tr("LinWin Start menu"));
    title->setObjectName(QStringLiteral("pageTitle"));
    auto *description = new QLabel(
        tr("Find and open applications installed on this computer. Pin the ones you use most."), this);
    description->setObjectName(QStringLiteral("intro"));
    description->setWordWrap(true);

    searchField_ = new QLineEdit(this);
    searchField_->setObjectName(QStringLiteral("applicationSearch"));
    searchField_->setPlaceholderText(tr("Search installed applications"));
    searchField_->setClearButtonEnabled(true);
    searchField_->setAccessibleName(tr("Search installed applications"));
    searchField_->installEventFilter(this);
    searchField_->addAction(LinWinVisual::shellIcon(LinWinVisual::Glyph::Search), QLineEdit::LeadingPosition);

    applicationList_ = new QListWidget(this);
    applicationList_->setObjectName(QStringLiteral("applicationList"));
    applicationList_->setSelectionMode(QAbstractItemView::SingleSelection);
    applicationList_->setAccessibleName(tr("All installed applications"));
    applicationList_->setUniformItemSizes(true);
    applicationList_->setIconSize(QSize(32, 32));
    applicationList_->setAlternatingRowColors(false);
    applicationList_->setMouseTracking(true);
    applicationList_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(applicationList_,&QListWidget::customContextMenuRequested,this,[this](const QPoint &pos){
        auto *item=applicationList_->itemAt(pos);if(!item||item->isHidden())return;
        applicationList_->setCurrentItem(item);
        QMenu menu(this);menu.addAction(tr("Open"),this,&AppLauncherWidget::launchSelectedApplication);
        menu.addAction(pinnedApplicationIds_.contains(item->data(Qt::UserRole).toString())?tr("Unpin from Start"):tr("Pin to Start"),this,&AppLauncherWidget::togglePinnedApplication);
        menu.exec(applicationList_->viewport()->mapToGlobal(pos));
    });

    auto *actions = new QHBoxLayout;
    auto *openButton = new QPushButton(tr("Open"), this);
    pinButton_ = new QPushButton(tr("Pin"), this);
    pinButton_->setObjectName("linwinPinAction");
    openButton->setObjectName(QStringLiteral("primaryAction"));
    actions->addWidget(openButton);
    actions->addWidget(pinButton_);
    actions->addStretch();

    statusLabel_ = new QLabel(this);
    statusLabel_->setObjectName(QStringLiteral("launcherStatus"));
    statusLabel_->setWordWrap(true);
    statusLabel_->setMaximumHeight(44);
    statusLabel_->setTextFormat(Qt::PlainText);

    auto *brandText=new QVBoxLayout;
    brandText->setSpacing(0);
    brandText->addWidget(title);
    auto *tagline=new QLabel(tr("Linux Desktop OS"),this);
    tagline->setObjectName(QStringLiteral("launcherTagline"));brandText->addWidget(tagline);
    brand->addLayout(brandText);
    brand->addSpacing(12);
    brand->addWidget(searchField_,1);
    layout->addLayout(brand);
    description->hide();
    auto *columns = new QHBoxLayout;
    auto *rail = new QFrame(this);rail_=rail;
    rail->setObjectName(QStringLiteral("linwinStartRail"));
    rail->setFixedWidth(142);
    auto *navigation=new QVBoxLayout(rail);
    navigation->setContentsMargins(0,0,8,0);
    navigation->setSpacing(4);
    auto *allButton=new QPushButton(tr("All apps"),rail);
    auto *pinnedButton=new QPushButton(tr("Pinned"),rail);
    allButton->setObjectName(QStringLiteral("linwinAllApps"));
    pinnedButton->setObjectName(QStringLiteral("linwinPinnedView"));
    allButton->setIcon(QIcon::fromTheme(QStringLiteral("view-grid"),LinWinVisual::shellIcon(LinWinVisual::Glyph::Desktop)));
    pinnedButton->setIcon(QIcon::fromTheme(QStringLiteral("starred"),LinWinVisual::shellIcon(LinWinVisual::Glyph::More)));
    allButton->setCheckable(true);pinnedButton->setCheckable(true);
    auto *viewGroup=new QButtonGroup(rail);viewGroup->setExclusive(true);viewGroup->addButton(allButton);viewGroup->addButton(pinnedButton);
    allButton->setIconSize(QSize(23,23));pinnedButton->setIconSize(QSize(23,23));
    pinnedButton->setChecked(true);
    navigation->addWidget(allButton);
    navigation->addWidget(pinnedButton);
    columns->addWidget(rail);
    auto *pins = new QVBoxLayout;
    auto *pinnedTitle=new QLabel(tr("Pinned"),this);pinnedTitle->setObjectName(QStringLiteral("linwinStartSection"));
    auto *pinHeader=new QHBoxLayout;
    pinHeader->addWidget(pinnedTitle);pinHeader->addStretch();
    auto *editPins=new QPushButton(tr("Edit"),this);
    editPins->setObjectName(QStringLiteral("linwinEditPins"));
    auto *editMenu=new QMenu(editPins);
    editMenu->addAction(tr("Add suggested installed apps"),this,[this]{addSuggestedPins();refreshList();});
    auto *suggest=new QPushButton(tr("Add apps"),this);suggest->setObjectName("linwinSuggestedPins");suggest->setToolTip(tr("Browse installed applications and right-click to pin"));pinHeader->addWidget(suggest);
    connect(suggest,&QPushButton::clicked,this,[this]{appViews_->setCurrentIndex(1);searchField_->setFocus();});
    editMenu->addAction(tr("Browse all apps"),this,[this]{appViews_->setCurrentIndex(1);searchField_->setFocus();});
    editPins->setMenu(editMenu);pinHeader->addWidget(editPins);pins->addLayout(pinHeader);
    pinnedList_ = new PinnedGrid(this);
    pinnedList_->setObjectName(QStringLiteral("pinnedApps"));
    pinnedList_->setViewMode(QListView::IconMode);
    pinnedList_->setFlow(QListView::LeftToRight);
    pinnedList_->setWrapping(true);
    pinnedList_->setUniformItemSizes(true);
    pinnedList_->setResizeMode(QListView::Adjust);
    pinnedList_->setMovement(QListView::Static);
    pinnedList_->setIconSize(QSize(32,32));
    pinnedList_->setGridSize(QSize(94,106));
    pinnedList_->setSelectionMode(QAbstractItemView::SingleSelection);
    pinnedList_->setIconSize(QSize(44,44));
    pinnedList_->setItemDelegate(new LinWinVisual::PinnedTileDelegate(pinnedList_));
    pinnedList_->setMouseTracking(true);
    pinnedList_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    pinnedList_->setTextElideMode(Qt::ElideNone);
    pinnedList_->setSpacing(0);
    pinnedList_->setWordWrap(true);
    pinnedList_->setAccessibleName(tr("Pinned applications"));
    pinnedList_->setContextMenuPolicy(Qt::CustomContextMenu);
    auto *earlier=new QAction(tr("Move earlier"),pinnedList_);earlier->setObjectName("linwinPinEarlier");
    auto *later=new QAction(tr("Move later"),pinnedList_);later->setObjectName("linwinPinLater");
    connect(earlier,&QAction::triggered,this,[this]{moveCurrentPin(-1);});
    connect(later,&QAction::triggered,this,[this]{moveCurrentPin(1);});
    connect(pinnedList_,&QListWidget::customContextMenuRequested,this,[this,earlier,later](const QPoint &pos){
        auto *item=pinnedList_->itemAt(pos);if(!item||item->isHidden())return;
        pinnedList_->setCurrentItem(item);
        const QString id=item->data(Qt::UserRole).toString();
        QMenu menu(this);menu.addAction(tr("Open"),this,[this]{launchSelectedApplication();});
        const int row=pinnedApplicationIds_.indexOf(id);
        earlier->setEnabled(adjacentPin(row,-1)>=0);later->setEnabled(adjacentPin(row,1)>=0);
        menu.addAction(earlier);menu.addAction(later);menu.addSeparator();
        menu.addAction(tr("Unpin from Start"),this,[this,id]{pinnedApplicationIds_.removeAll(id);savePinnedApplications();refreshList();});
        menu.exec(pinnedList_->viewport()->mapToGlobal(pos));
    });
    emptyPins_=new QLabel(tr("No pinned apps yet. Choose Add apps, then right-click an installed app to pin it here."),this);
    emptyPins_->setObjectName("linwinEmptyPins");emptyPins_->setWordWrap(true);emptyPins_->setAlignment(Qt::AlignCenter);emptyPins_->setMargin(18);pins->addWidget(emptyPins_);
    pins->addWidget(pinnedList_, 1);
    auto *pinnedPage=new QWidget(this);
    pins->setContentsMargins(0,0,0,0);
    pinnedPage->setLayout(pins);
    appViews_=new QStackedWidget(this);
    appViews_->setObjectName(QStringLiteral("linwinStartViews"));
    appViews_->addWidget(pinnedPage);
    appViews_->addWidget(applicationList_);
    connect(appViews_,&QStackedWidget::currentChanged,this,[allButton,pinnedButton](int index){allButton->setChecked(index==1);pinnedButton->setChecked(index==0);});
    auto *content=new QVBoxLayout;
    content->addWidget(appViews_,1);
    columns->addLayout(content,5);
    columns->setSpacing(12);
    columns->setStretch(0,0);
    layout->addLayout(columns, 1);
    connect(allButton,&QPushButton::clicked,this,[this]{appViews_->setCurrentIndex(1);});
    connect(pinnedButton,&QPushButton::clicked,this,[this]{searchField_->clear();appViews_->setCurrentIndex(0);});
    auto *recentHeader=new QHBoxLayout;
    recentTitle_=new QLabel(tr("Recent documents"),this);recentTitle_->setObjectName(QStringLiteral("linwinStartSection"));recentHeader->addWidget(recentTitle_);
    auto *recentToggle=new QCheckBox(tr("Show recent documents"),this);
    recentToggle->setObjectName(QStringLiteral("linwinRecentToggle"));
    recentToggle->setChecked(QSettings().value(QStringLiteral("launcher/showRecent"),true).toBool());
    recentHeader->addStretch(); recentHeader->addWidget(recentToggle);
    content->addLayout(recentHeader);
    recentList_=new QListWidget(this);
    recentList_->setObjectName(QStringLiteral("recentDocuments"));
    recentList_->setMaximumHeight(100);
    recentList_->setMinimumHeight(44);
    content->addWidget(recentList_);
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
    connect(pinnedList_, &QListWidget::currentItemChanged,this,[this](QListWidgetItem *pin,QListWidgetItem *) {
        if(!pin || pin->isHidden()) return;
        const auto id=pin->data(Qt::UserRole).toString();
        for(int row=0;row<applicationList_->count();++row) if(applicationList_->item(row)->data(Qt::UserRole).toString()==id) {applicationList_->setCurrentRow(row);break;}
    });
    const auto launchPin=[this](QListWidgetItem *pin) {
        if (!pin || pin->isHidden()) return;
        const auto id = pin->data(Qt::UserRole).toString();
        for (int row=0;row<applicationList_->count();++row) {
            auto *item=applicationList_->item(row);
            if (!item->isHidden() && item->data(Qt::UserRole).toString()==id) {
                applicationList_->setCurrentRow(row); launchSelectedApplication(); break;
            }
        }
    };
    connect(pinnedList_,&QListWidget::itemClicked,this,launchPin);
    // Enter launches the selected tile; no second launch on double click.
    pinnedList_->setProperty("linwinSingleClickLaunch",true);
    auto *enterPin=new QShortcut(QKeySequence(Qt::Key_Return),pinnedList_);enterPin->setContext(Qt::WidgetShortcut);
    connect(enterPin,&QShortcut::activated,this,[this,launchPin]{launchPin(pinnedList_->currentItem());});
    auto *keypadPin=new QShortcut(QKeySequence(Qt::Key_Enter),pinnedList_);keypadPin->setContext(Qt::WidgetShortcut);
    connect(keypadPin,&QShortcut::activated,this,[this,launchPin]{launchPin(pinnedList_->currentItem());});
    content->addLayout(actions);
    openButton->setMinimumHeight(30);pinButton_->setMinimumHeight(30);
    auto *quick=navigation;
    for (const auto &tool : QList<QPair<int,QString>>{{1,tr("Files")},{2,tr("Task Manager")},{3,tr("Clipboard")},{4,tr("Settings")}}) {
        auto *button=new QPushButton(tool.second,rail);
        button->setObjectName(QStringLiteral("linwinStartTool_%1").arg(tool.first));
        const LinWinVisual::Glyph glyphs[]{LinWinVisual::Glyph::Files,LinWinVisual::Glyph::Tasks,LinWinVisual::Glyph::Clipboard,LinWinVisual::Glyph::Settings};
        button->setIcon(LinWinVisual::shellIcon(glyphs[tool.first-1]));
        button->setIconSize(QSize(23,23));
        quick->addWidget(button);
        connect(button,&QPushButton::clicked,this,[this,tool]{ emit toolRequested(tool.first); });
    }
    quick->addStretch();
    auto *power=new QPushButton(QIcon::fromTheme(QStringLiteral("system-shutdown"),LinWinVisual::shellIcon(LinWinVisual::Glyph::More)),tr("Power"),rail);
    power->setObjectName(QStringLiteral("linwinStartPower"));
    auto *powerMenu=new QMenu(power);
    auto *back=powerMenu->addAction(tr("Return to Linux desktop"));
    auto *exit=powerMenu->addAction(tr("Close LinWin"));
    connect(back,&QAction::triggered,this,&AppLauncherWidget::returnDesktopRequested);
    connect(exit,&QAction::triggered,this,&AppLauncherWidget::exitRequested);
    power->setMenu(powerMenu);quick->addWidget(power);
    for(auto *button:rail->findChildren<QPushButton *>()) {button->setProperty("linwinFullLabel",button->text());button->setAccessibleName(button->text());button->setToolTip(button->text());}
    auto *footer=new QFrame(this);footer->setObjectName(QStringLiteral("linwinStartFooter"));
    auto *footerLayout=new QHBoxLayout(footer);footerLayout->setContentsMargins(6,5,6,2);
    auto *account=new QLabel(tr("Local user"),footer);
    account->setObjectName(QStringLiteral("linwinStartAccount"));
    const QString name=qEnvironmentVariable("USER");
    if(!name.isEmpty()) account->setText(name);
    account->setTextFormat(Qt::PlainText);
    auto *avatar=new QLabel(footer);
    avatar->setPixmap(QIcon::fromTheme(QStringLiteral("avatar-default"),LinWinVisual::shellIcon(LinWinVisual::Glyph::Desktop)).pixmap(30,30));
    footerLayout->addWidget(avatar);footerLayout->addWidget(account);footerLayout->addStretch();
    auto *preferences=new QPushButton(LinWinVisual::shellIcon(LinWinVisual::Glyph::Settings),tr("Settings"),footer);
    preferences->setObjectName(QStringLiteral("linwinFooterSettings"));
    connect(preferences,&QPushButton::clicked,this,[this]{emit toolRequested(4);});
    footerLayout->addWidget(preferences);
    layout->addWidget(statusLabel_);
    layout->addWidget(footer);
    refreshRecentDocuments();

    QSettings settings;
    pinnedApplicationIds_ = settings.value(kPinnedAppsSetting).toStringList();
    pinnedApplicationIds_.removeDuplicates();
    loadApplications();
    // Fresh profiles only; existing pins change only through the explicit Edit action.
    if (!settings.contains(kPinnedAppsSetting)) {addSuggestedPins();savePinnedApplications();}
    refreshList();

    connect(searchField_, &QLineEdit::textChanged,
            this, &AppLauncherWidget::filterApplications);
    connect(searchField_, &QLineEdit::textChanged,this,[this](const QString &query){appViews_->setCurrentIndex(query.trimmed().isEmpty()?0:1);});
    connect(openButton, &QPushButton::clicked,
            this, &AppLauncherWidget::launchSelectedApplication);
    connect(pinButton_, &QPushButton::clicked,
            this, &AppLauncherWidget::togglePinnedApplication);
    connect(applicationList_,&QListWidget::itemClicked,this,[this](QListWidgetItem *item){if(item&&!item->isHidden()){applicationList_->setCurrentItem(item);launchSelectedApplication();}});
    for(const auto key:{Qt::Key_Return,Qt::Key_Enter}){
        auto *shortcut=new QShortcut(QKeySequence(key),applicationList_);shortcut->setContext(Qt::WidgetShortcut);
        connect(shortcut,&QShortcut::activated,this,&AppLauncherWidget::launchSelectedApplication);
    }
    connect(applicationList_, &QListWidget::currentRowChanged,
            this, [this](int) { updatePinButton(); });
    connect(searchField_, &QLineEdit::returnPressed,this,[this]{
        auto *list=appViews_->currentIndex()==0?pinnedList_:applicationList_;
        if(!list->currentItem()||list->currentItem()->isHidden()){
            for(int row=0;row<list->count();++row)if(!list->item(row)->isHidden()){
                list->setCurrentRow(row);break;
            }
        }
        auto *item=list->currentItem();
        if(!item||item->isHidden())return;
        if(list==pinnedList_) {
            const QString id=item->data(Qt::UserRole).toString();
            for(int row=0;row<applicationList_->count();++row)if(!applicationList_->item(row)->isHidden()&&applicationList_->item(row)->data(Qt::UserRole).toString()==id){
                applicationList_->setCurrentRow(row);launchSelectedApplication();return;
            }
        } else launchSelectedApplication();
    });

    updatePinButton();
}

AppLauncherWidget::~AppLauncherWidget()
{
    for (GAppInfo *appInfo : applications_) {
        g_object_unref(appInfo);
    }
}

void AppLauncherWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if(!rail_)return;
    const bool narrow=width()<520;
    rail_->setFixedWidth(narrow?52:142);
    for(auto *button:rail_->findChildren<QPushButton *>()){
        const auto label=button->property("linwinFullLabel").toString();
        if(!label.isEmpty())button->setText(narrow?QString():label);
    }
}

int AppLauncherWidget::adjacentPin(int row,int offset) const
{
    if(row<0||(offset!=1&&offset!=-1))return -1;
    for(int next=row+offset;next>=0&&next<pinnedApplicationIds_.size();next+=offset)
        if(applications_.contains(pinnedApplicationIds_.at(next)))return next;
    return -1;
}

void AppLauncherWidget::moveCurrentPin(int offset)
{
    auto *item=pinnedList_->currentItem();if(!item||item->isHidden()||(offset!=1&&offset!=-1))return;
    const QString id=item->data(Qt::UserRole).toString();
    const int from=pinnedApplicationIds_.indexOf(id),to=adjacentPin(from,offset);
    if(from<0||to<0||to>=pinnedApplicationIds_.size())return;
    pinnedApplicationIds_.swapItemsAt(from,to);savePinnedApplications();refreshList();
    for(int i=0;i<pinnedList_->count();++i)if(pinnedList_->item(i)->data(Qt::UserRole).toString()==id){pinnedList_->setCurrentRow(i);break;}
}

void AppLauncherWidget::paintEvent(QPaintEvent *)
{
    QStyleOption option;option.initFrom(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget,&option,&painter,this);
    if(glassBackdrop_.isNull())return;
    painter.setRenderHint(QPainter::Antialiasing);painter.setRenderHint(QPainter::SmoothPixmapTransform);
    QPainterPath outline;outline.addRoundedRect(QRectF(rect()).adjusted(1,1,-1,-1),12,12);
    painter.save();painter.setClipPath(outline);
    painter.drawImage(rect(),glassBackdrop_);
    QLinearGradient tint(0,0,width(),height());
    tint.setColorAt(0,QColor(9,35,62,241));tint.setColorAt(1,QColor(8,34,60,244));
    painter.fillRect(rect(),tint);painter.restore();
    painter.setBrush(Qt::NoBrush);painter.setPen(QPen(QColor(54,178,225,180),1));painter.drawPath(outline);
}

bool AppLauncherWidget::eventFilter(QObject *object,QEvent *event)
{
    if(object==searchField_&&event->type()==QEvent::KeyPress){
        const auto *key=static_cast<QKeyEvent *>(event);
        if(key->key()==Qt::Key_Down||key->key()==Qt::Key_Up){
            auto *list=appViews_->currentIndex()==0?pinnedList_:applicationList_;
            const int step=key->key()==Qt::Key_Down?1:-1;
            int row=list->currentRow();
            if(row<0)row=step>0?-1:list->count();
            for(row+=step;row>=0&&row<list->count();row+=step)if(!list->item(row)->isHidden()){
                list->setCurrentRow(row);list->scrollToItem(list->currentItem());break;
            }
            return true;
        }
    }
    return QWidget::eventFilter(object,event);
}

void AppLauncherWidget::focusSearch()
{
    // Re-enumerate the host desktop registry when Start opens, including new packages.
    loadApplications();
    searchField_->clear();
    appViews_->setCurrentIndex(0);
    refreshList();
    refreshRecentDocuments();
    searchField_->setFocus(Qt::OtherFocusReason);
}

void AppLauncherWidget::refreshRecentDocuments()
{
    recentList_->clear();
    const bool enabled=QSettings().value(QStringLiteral("launcher/showRecent"),true).toBool();
    recentList_->setVisible(enabled);
    recentTitle_->setVisible(enabled);
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

void AppLauncherWidget::addSuggestedPins()
{
    std::vector<LinWinPins::App> apps;
    for(auto it=applications_.cbegin();it!=applications_.cend();++it){
        LinWinPins::App candidate{it.key().toStdString(),{}};
        if(G_IS_DESKTOP_APP_INFO(it.value())){
            const auto *categories=g_desktop_app_info_get_categories(G_DESKTOP_APP_INFO(it.value()));
            if(categories)for(const auto &category:QString::fromUtf8(categories).split(QLatin1Char(';'),Qt::SkipEmptyParts))
                candidate.categories.push_back(category.toStdString());
        }
        apps.push_back(std::move(candidate));
    }
    std::vector<std::string> existing;
    for(const auto &id:pinnedApplicationIds_)existing.push_back(id.toStdString());
    for(const auto &id:LinWinPins::suggestions(std::move(apps),existing))
        pinnedApplicationIds_.append(QString::fromStdString(id));
    savePinnedApplications();
}

void AppLauncherWidget::refreshList()
{
    applicationList_->clear();
    pinnedList_->clear();
    QStringList ids = applications_.keys();
    QCollator collator;
    std::sort(ids.begin(), ids.end(), [this, &collator](const QString &left, const QString &right) {
        const int order=collator.compare(appName(applications_.value(left)),appName(applications_.value(right)));
        return order==0?left<right:order<0;
    });

    for (const QString &id : ids) {
        GAppInfo *appInfo = applications_.value(id);
        auto *item = new QListWidgetItem(appIcon(appInfo), appName(appInfo), applicationList_);
        item->setData(Qt::UserRole, id);
        const QString description = appDescription(appInfo);
        item->setToolTip(description.isEmpty() ? appName(appInfo) : description);
        if (pinnedApplicationIds_.contains(id)) {
            item->setText(appName(appInfo) + tr("  •  Pinned"));

        }
    }

    for(const auto &id:pinnedApplicationIds_){
        auto *info=applications_.value(id,nullptr);
        if(!info)continue;
        auto *pin=new QListWidgetItem(appIcon(info),appName(info),pinnedList_);
        pin->setData(Qt::UserRole,id);pin->setToolTip(appName(info)+(appDescription(info).isEmpty()?QString():QStringLiteral("\n")+appDescription(info)));
    }
    emptyPins_->setVisible(pinnedList_->count()==0);
    // Keep the empty grid in the layout so the Add apps entry and empty state remain stable.
    filterApplications(searchField_->text());
    updatePinButton();
    if (applications_.isEmpty()) {
        statusLabel_->setText(tr("No installed applications were found in the desktop application registry."));
    } else {
        statusLabel_->setText(tr("%1 installed apps · Right-click to manage pins")
                                  .arg(static_cast<qlonglong>(applications_.size())));
    }
}

void AppLauncherWidget::filterApplications(const QString &query)
{
    const QString needle = query.simplified();
    const auto terms=needle.split(QLatin1Char(' '),Qt::SkipEmptyParts);
    const auto matchesTerms=[&terms](const QString &text){return std::all_of(terms.cbegin(),terms.cend(),[&text](const QString &word){return text.contains(word,Qt::CaseInsensitive);});};
    int visibleCount = 0;
    for (int row = 0; row < applicationList_->count(); ++row) {
        auto *item = applicationList_->item(row);
        const QString searchable = item->text() + QLatin1Char(' ') + item->toolTip();
        const bool matches = matchesTerms(searchable);
        item->setHidden(!matches);
        if (matches) {
            ++visibleCount;
        }
    }

    for (int row=0;row<pinnedList_->count();++row) {
        auto *pin=pinnedList_->item(row);
        pin->setHidden(!matchesTerms(pin->text()+QLatin1Char(' ')+pin->toolTip()));
    }
    if (applicationList_->currentItem() != nullptr && applicationList_->currentItem()->isHidden()) {
        applicationList_->setCurrentRow(-1);
    }
    // A typed query must leave a usable selection, not a disabled Pin/Open
    // action. Never select an invisible item or launch anything automatically.
    if(!needle.isEmpty()&&(!applicationList_->currentItem()||applicationList_->currentItem()->isHidden())){
        for(int row=0;row<applicationList_->count();++row)if(!applicationList_->item(row)->isHidden()){
            applicationList_->setCurrentRow(row);break;
        }
    }
    updatePinButton();

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
    emit applicationLaunched();
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
    pinButton_->setEnabled(item != nullptr && !item->isHidden());
}

void AppLauncherWidget::savePinnedApplications() const
{
    QSettings settings;
    settings.setValue(kPinnedAppsSetting, pinnedApplicationIds_);
}
