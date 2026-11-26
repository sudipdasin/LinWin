// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "snap_geometry.h"
#include "shell_icons.h"
#include <QDialog>
#include <QMenu>
#include <QMessageBox>
#include <QScreen>
#include <QShortcut>
#include <QToolButton>
#include <QHBoxLayout>
#include <functional>

class SnapController final : public QObject {
public:
    using Zone = LinWinSnap::Zone;
    explicit SnapController(QDialog *window, QWidget *desktopArea)
        : QObject(window), window_(window), desktopArea_(desktopArea) {}

    QToolButton *createButton() {
        auto *button = new QToolButton(window_);
        button->setObjectName(QStringLiteral("linwinSnapMenu"));
        button->setText(tr("Snap layouts"));
        button->setIcon(LinWinVisual::shellIcon(LinWinVisual::Glyph::Snap));
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button->setPopupMode(QToolButton::InstantPopup);
        auto *menu = new QMenu(button);
        const QList<QPair<QString,Zone>> zones{
            {tr("Left half"),Zone::Left},{tr("Right half"),Zone::Right},
            {tr("Top left"),Zone::TopLeft},{tr("Top right"),Zone::TopRight},
            {tr("Bottom left"),Zone::BottomLeft},{tr("Bottom right"),Zone::BottomRight},
            {tr("Fill workspace"),Zone::Fill}
        };
        for (const auto &entry : zones)
            menu->addAction(entry.first,this,[this,entry]{ snap(entry.second); });
        menu->addSeparator();
        menu->addAction(tr("Restore previous size"),this,[this]{ restore(); });
        button->setMenu(menu);
        bind(QStringLiteral("Alt+Shift+Left"),[this]{snap(Zone::Left);});
        bind(QStringLiteral("Alt+Shift+Right"),[this]{snap(Zone::Right);});
        bind(QStringLiteral("Alt+Shift+Up"),[this]{snap(Zone::Fill);});
        bind(QStringLiteral("Alt+Shift+Down"),[this]{restore();});
        button->setToolTip(tr("Alt+Shift+arrow: left, right, fill or restore"));
        return button;
    }

    bool snap(Zone zone) {
        if (!window_->isVisible() || !window_->isEnabled()) return false;
        const auto area=workArea();
        const QRect frame=window_->frameGeometry(), client=window_->geometry();
        const QSize minimum=window_->minimumSizeHint().expandedTo(window_->minimumSize()).expandedTo(QSize(1,1));
        const LinWinSnap::Insets margins{qMax(0,client.left()-frame.left()),qMax(0,client.top()-frame.top()),
                                        qMax(0,frame.right()-client.right()),qMax(0,frame.bottom()-client.bottom())};
        auto target=LinWinSnap::target({area.x(),area.y(),area.width(),area.height()},zone,margins,minimum.width(),minimum.height());
        if (!target) {
            QMessageBox::information(window_,tr("Snap layouts"),tr("This window needs more space. Choose a larger layout or display."));
            return false;
        }
        if (!previous_.isValid()) previous_=window_->isMaximized()?window_->normalGeometry():client;
        if (window_->isMaximized() || window_->isMinimized()) window_->showNormal();
        window_->setGeometry(target->x,target->y,target->w,target->h);
        window_->raise();
        return true;
    }
    void restore() {
        const QRect area=workArea();
        if (!previous_.isValid() || !area.isValid()) return;
        const QSize size=previous_.size().boundedTo(area.size());
        const int x=qBound(area.left(),previous_.x(),area.right()-size.width()+1);
        const int y=qBound(area.top(),previous_.y(),area.bottom()-size.height()+1);
        window_->showNormal();
        window_->setGeometry(QRect(QPoint(x,y),size));
        previous_={};
        window_->raise();
    }
private:
    QRect workArea() const {
        const QScreen *screen=window_->screen();
        if (!screen) return {};
        QRect area=screen->availableGeometry();
        if (desktopArea_ && desktopArea_->isVisible() && desktopArea_->window()->isFullScreen()
            && desktopArea_->screen()==screen) {
            const QRect desktop(desktopArea_->mapToGlobal(QPoint(0,0)),desktopArea_->size());
            const QRect clipped=area.intersected(desktop);
            if(clipped.isValid()) area=clipped;
        }
        return area;
    }
    void bind(const QString &key,std::function<void()> fn) {
        auto *shortcut=new QShortcut(QKeySequence(key),window_);
        shortcut->setContext(Qt::WindowShortcut);
        connect(shortcut,&QShortcut::activated,this,std::move(fn));
    }
    QDialog *window_;
    QWidget *desktopArea_;
    QRect previous_;
};
