// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSet>
#include <QStandardPaths>
#include <QUrl>
#include <QXmlStreamReader>
#include <algorithm>

namespace LinWinRecent {
struct Entry { QString path; QDateTime used; };
inline bool allowedDocument(const QString &path) {
    const QFileInfo file(path);
    static const QSet<QString> suffixes{QStringLiteral("pdf"),QStringLiteral("txt"),QStringLiteral("md"),QStringLiteral("odt"),QStringLiteral("ods"),QStringLiteral("odp"),QStringLiteral("docx"),QStringLiteral("xlsx"),QStringLiteral("pptx"),QStringLiteral("csv"),QStringLiteral("png"),QStringLiteral("jpg"),QStringLiteral("jpeg"),QStringLiteral("webp")};
    return file.isFile() && file.isReadable() && !file.isSymLink() && !file.isExecutable() && suffixes.contains(file.suffix().toLower());
}
inline QList<Entry> read(const QString &registry, int limit=5) {
    QFile file(registry);
    constexpr qint64 maxBytes=1024*1024;
    if (limit<=0 || !file.open(QIODevice::ReadOnly) || file.size()>maxBytes) return {};
    const auto data=file.read(maxBytes+1);
    if (data.size()>maxBytes) return {};
    QXmlStreamReader xml(data);
    QList<Entry> entries;
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isDTD()) return {};
        if (!xml.isStartElement() || xml.name()!=QStringLiteral("bookmark")) continue;
        const auto attrs=xml.attributes();
        const QUrl url(attrs.value(QStringLiteral("href")).toString(), QUrl::StrictMode);
        if (!url.isValid() || !url.isLocalFile() || !url.host().isEmpty() || url.hasQuery() || url.hasFragment()) continue;
        const auto path=QDir::cleanPath(url.toLocalFile());
        if (!QDir::isAbsolutePath(path) || !allowedDocument(path)) continue;
        auto used=QDateTime::fromString(attrs.value(QStringLiteral("modified")).toString(),Qt::ISODate);
        if (!used.isValid()) used=QDateTime::fromString(attrs.value(QStringLiteral("visited")).toString(),Qt::ISODate);
        if (!used.isValid()) used=QFileInfo(path).lastModified();
        entries.append({path,used});
    }
    if (xml.hasError()) return {};
    std::stable_sort(entries.begin(),entries.end(),[](const Entry &a,const Entry &b){return a.used>b.used;});
    QList<Entry> result; QSet<QString> seen;
    for (const auto &entry: entries) {
        if (seen.contains(entry.path)) continue;
        seen.insert(entry.path); result.append(entry);
        if (result.size()>=limit) break;
    }
    return result;
}
inline QString registryPath() {
    return QDir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)).filePath(QStringLiteral("recently-used.xbel"));
}
}
