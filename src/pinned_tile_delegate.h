// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QStyledItemDelegate>
#include <QPainter>
#include <QApplication>
#include <QFontMetrics>
#include <QTextOption>
#include <QStyleOptionViewItem>
#include <QIcon>
namespace LinWinVisual {
inline QRect fittedIconRect(const QRect &slot, const QSize &source) {
    if(slot.isEmpty() || source.isEmpty()) return {};
    const QSize size=source.scaled(slot.size(),Qt::KeepAspectRatio);
    return QRect(slot.center()-QPoint((size.width()-1)/2,(size.height()-1)/2),size);
}
class PinnedTileDelegate final : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter *p,const QStyleOptionViewItem &option,const QModelIndex &index) const override {
        QStyleOptionViewItem o(option);initStyleOption(&o,index);
        const QRect tile=o.rect.adjusted(3,2,-3,-2);
        if(tile.width()<12||tile.height()<20)return;
        p->save();p->setRenderHint(QPainter::Antialiasing);p->setRenderHint(QPainter::SmoothPixmapTransform);
        p->setClipRect(o.rect);
        if(o.state.testFlag(QStyle::State_Selected)||o.state.testFlag(QStyle::State_MouseOver)) {
            p->setPen(Qt::NoPen);
            p->setBrush(o.palette.color(QPalette::Highlight));
            p->setOpacity(o.state.testFlag(QStyle::State_Selected)?.78:.35);
            p->drawRoundedRect(tile,7,7);p->setOpacity(1);
        }
        const int side=qMin(44,qMin(tile.width()-8,tile.height()/2));
        const QRect slot(tile.center().x()-side/2,tile.top()+4,side,side);
        const QPixmap icon=o.icon.pixmap(QSize(96,96));
        if(!icon.isNull())p->drawPixmap(fittedIconRect(slot,icon.size()),icon);
        const QRect label(tile.left()+3,slot.bottom()+4,tile.width()-6,qMax(1,tile.bottom()-slot.bottom()-4));
        p->setPen(o.palette.color(o.state.testFlag(QStyle::State_Selected)?QPalette::HighlightedText:QPalette::Text));
        p->setFont(o.font);
        QTextOption text; text.setAlignment(Qt::AlignHCenter|Qt::AlignTop);text.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        p->drawText(QRectF(label),o.text,text);
        if(o.state.testFlag(QStyle::State_HasFocus)){p->setPen(QPen(o.palette.color(QPalette::Highlight),1));p->setBrush(Qt::NoBrush);p->drawRoundedRect(tile,7,7);}
        p->restore();
    }
};
}
