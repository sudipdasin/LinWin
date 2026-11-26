// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QStyledItemDelegate>
#include <QPainter>
#include <QApplication>
#include <QFontMetrics>
#include <QTextOption>
#include <QStyleOptionViewItem>
#include <QIcon>
#include <QListView>
#include <QTextLayout>
namespace LinWinVisual {
inline QRect fittedIconRect(const QRect &slot, const QSize &source) {
    if(slot.isEmpty() || source.isEmpty()) return {};
    const QSize size=source.scaled(slot.size(),Qt::KeepAspectRatio);
    return QRect(slot.center()-QPoint((size.width()-1)/2,(size.height()-1)/2),size);
}
class PinnedTileDelegate final : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    QSize sizeHint(const QStyleOptionViewItem &option,const QModelIndex &index) const override {
        if(const auto *view=qobject_cast<const QListView *>(parent());view&&view->gridSize().isValid())
            return view->gridSize()-QSize(2,2);
        return QStyledItemDelegate::sizeHint(option,index);
    }
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
        const auto *view=qobject_cast<const QListView *>(parent());
        const QFontMetrics fm(o.font);
        const int lineHeight=fm.lineSpacing();
        // Reserve space for two complete text lines before sizing the icon.
        const int requested=view&&view->iconSize().isValid()?view->iconSize().width():44;
        const int side=qMax(1,qMin(requested,qMin(tile.width()-8,tile.height()-2*lineHeight-14)));
        const QRect slot(tile.center().x()-side/2,tile.top()+4,side,side);
        const QPixmap icon=o.icon.pixmap(QSize(96,96));
        if(!icon.isNull())p->drawPixmap(fittedIconRect(slot,icon.size()),icon);
        const QRect label(tile.left()+3,slot.bottom()+4,tile.width()-6,qMax(1,tile.bottom()-slot.bottom()-4));
        p->setPen(o.palette.color(o.state.testFlag(QStyle::State_Selected)?QPalette::HighlightedText:QPalette::Text));
        p->setFont(o.font);
        // Draw complete lines only; elide the final visible line instead of
        // painting partial glyphs below the item rectangle.
        QString remaining=o.text.simplified();
        QTextLayout text(remaining,o.font);
        QTextOption opt;opt.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);text.setTextOption(opt);
        const int rows=qMin(2,label.height()/qMax(1,lineHeight));
        text.beginLayout();
        for(int row=0;row<rows;++row){
            auto line=text.createLine();if(!line.isValid())break;
            line.setLineWidth(label.width());
            QString value=remaining.mid(line.textStart(),line.textLength()).trimmed();
            if(row==rows-1)value=fm.elidedText(remaining.mid(line.textStart()),Qt::ElideRight,label.width());
            const QRect bounds(label.x(),label.y()+row*lineHeight,label.width(),lineHeight);
            p->drawText(bounds,Qt::AlignHCenter|Qt::AlignVCenter|Qt::TextSingleLine,value);
        }
        text.endLayout();
        if(o.state.testFlag(QStyle::State_HasFocus)){p->setPen(QPen(o.palette.color(QPalette::Highlight),1));p->setBrush(Qt::NoBrush);p->drawRoundedRect(tile,7,7);}
        p->restore();
    }
};
}
