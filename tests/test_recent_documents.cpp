// SPDX-License-Identifier: GPL-3.0-or-later
#include "recent_documents.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QFile>
#include <QUrl>
#include <iostream>
int main(int argc,char **argv) {
    QCoreApplication app(argc,argv);
    QTemporaryDir tmp;
    if(!tmp.isValid()) return 1;
    auto put=[](const QString &p,const QByteArray &bytes){QFile f(p);return f.open(QIODevice::WriteOnly|QIODevice::Truncate)&&f.write(bytes)==bytes.size();};
    const auto a=tmp.filePath("old.txt"),b=tmp.filePath("new.pdf"),exe=tmp.filePath("unsafe.desktop"),registry=tmp.filePath("recent.xbel");
    if(!put(a,"old")||!put(b,"new")||!put(exe,"unsafe")) return 2;
    auto row=[](const QString &p,const char *date){return QStringLiteral("<bookmark href=\"%1\" modified=\"%2\"/>").arg(QUrl::fromLocalFile(p).toString(QUrl::FullyEncoded).toHtmlEscaped(),QString::fromLatin1(date));};
    const auto xml=QStringLiteral("<xbel>")+row(a,"2026-01-01T00:00:00Z")+row(b,"2026-01-02T00:00:00Z")+row(b,"2026-01-03T00:00:00Z")+row(exe,"2026-01-04T00:00:00Z")+row(tmp.filePath("missing.txt"),"2026-01-05T00:00:00Z")+QStringLiteral("<bookmark href=\"https://example.org/file.pdf\"/></xbel>");
    if(!put(registry,xml.toUtf8())) return 3;
    auto entries=LinWinRecent::read(registry);
    if(entries.size()!=2||entries[0].path!=b||entries[1].path!=a) return 4;
    if(LinWinRecent::read(registry,1).size()!=1||!LinWinRecent::read(registry,0).isEmpty()) return 5;
    if(!put(registry,xml.left(xml.size()-4).toUtf8())||!LinWinRecent::read(registry).isEmpty()) return 6;
    if(!put(registry,"<!DOCTYPE xbel [<!ENTITY x 'test'>]><xbel/>")||!LinWinRecent::read(registry).isEmpty()) return 7;
    if(!put(registry,QByteArray(1024*1024+1,'x'))||!LinWinRecent::read(registry).isEmpty()) return 8;
    if(LinWinRecent::allowedDocument(exe)||LinWinRecent::allowedDocument(tmp.path())) return 9;
    std::cout<<"PASS: local recent documents, ordering, deduplication, limit, unsafe targets and malformed input\n";
}
