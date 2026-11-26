// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "wallpaper_shader.h"
#include "theme_visuals.h"
#include "neon_landscape.h"
#include <QApplication>
#include <QImage>
#include <QLinearGradient>
#include <QPainter>
#include <QRadialGradient>
#include <QResizeEvent>
#include <QSettings>
#include <QTimer>
#include <QWidget>
#ifdef LINWIN_HAS_OPENGL
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QSurfaceFormat>
#endif

class DesktopSurface final : public QWidget {
public:
    explicit DesktopSurface(QWidget *parent = nullptr) : QWidget(parent)
    {
        setObjectName(QStringLiteral("windowsDesktop"));
        setAttribute(Qt::WA_OpaquePaintEvent);
        QSettings settings;
        effects_ = settings.value(QStringLiteral("appearance/desktopEffects"), true).toBool();
        preferGpu_ = settings.value(QStringLiteral("appearance/desktopGpu"), true).toBool();
        timer_.setSingleShot(true);
        timer_.setInterval(150);
        connect(&timer_, &QTimer::timeout, this, [this]() { rebuild(); });
    }

    void setTheme(const QString &id) { themeId_ = LinWinVisual::qstr(LinWinVisual::theme(id).id); invalidate(); }
    qint64 wallpaperCacheBytes() const { return static_cast<qint64>(wallpaper_.sizeInBytes()); }
    quint64 rebuildCount() const { return rebuildCount_; }
    bool effectsEnabled() const { return effects_ && LinWinVisual::theme(themeId_).effects; }
    bool gpuPreferred() const { return preferGpu_ && LinWinVisual::theme(themeId_).effects; }
    void setEffectsEnabled(bool enabled)
    {
        effects_ = enabled;
        QSettings().setValue(QStringLiteral("appearance/desktopEffects"), enabled);
        invalidate();
    }
    void setGpuPreferred(bool enabled)
    {
        preferGpu_ = enabled;
        QSettings().setValue(QStringLiteral("appearance/desktopGpu"), enabled);
        invalidate();
    }

protected:
    void resizeEvent(QResizeEvent *event) override
    {
        QWidget::resizeEvent(event);
        if (themeId_ != QStringLiteral("minimal")) timer_.start();
    }
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        if (themeId_ == QStringLiteral("minimal") || wallpaper_.isNull()) paintFallback(painter, rect());
        else {
            painter.setRenderHint(QPainter::SmoothPixmapTransform);
            painter.drawImage(rect(), wallpaper_);
        }
        if (width() >= 600 && height() >= 300) {
            const qreal mark = qBound(54.0, width()*0.075, 128.0);
            const qreal x = width()*0.41, y = height()*0.38;
            LinWinVisual::drawLogo(painter, QRectF(x,y,mark,mark));
            painter.setPen(Qt::white);
            QFont f = painter.font(); f.setPixelSize(qRound(mark*0.50)); f.setWeight(QFont::DemiBold); painter.setFont(f);
            painter.drawText(QRectF(x+mark+12,y, width()*0.35,mark*.72), Qt::AlignVCenter, QStringLiteral("LinWin"));
            f.setPixelSize(qMax(9,qRound(mark*.105))); f.setWeight(QFont::Medium); f.setLetterSpacing(QFont::AbsoluteSpacing,1.8); painter.setFont(f);
            painter.setPen(QColor("#c3e5f5"));
            painter.drawText(QRectF(x+mark+14,y+mark*.65,width()*.35,mark*.22),Qt::AlignVCenter,QStringLiteral("LINUX DESKTOP OS"));
        }
    }

private:
    void invalidate()
    {
        wallpaper_ = QImage();
        timer_.stop();
        if (themeId_ == QStringLiteral("minimal")) setProperty("renderBackend", QStringLiteral("QPainter solid"));
        else timer_.start();
        update();
    }
    void paintFallback(QPainter &p, const QRect &r) const
    {
        QLinearGradient base(r.bottomLeft(), r.topRight());
        const auto &theme = LinWinVisual::theme(themeId_);
        const QColor sky(LinWinVisual::qstr(theme.sky)), horizon(LinWinVisual::qstr(theme.horizon));
        if (themeId_ == QStringLiteral("minimal")) { p.fillRect(r, sky); return; }
        base.setColorAt(0.0, sky);
        base.setColorAt(1.0, themeId_==QStringLiteral("neon")?QColor("#051a35"):horizon);
        p.fillRect(r, base);
        if (!effectsEnabled()) return;
        const QPointF center(r.left() + r.width()*0.72, r.top() + r.height()*0.43);
        QRadialGradient halo(center, qMax(1.0, r.width()*0.50));
        QColor glow(LinWinVisual::qstr(theme.accent)); glow.setAlpha(themeId_==QStringLiteral("neon")?45:100);
        halo.setColorAt(0.0, glow); glow.setAlpha(0); halo.setColorAt(1.0, glow);
        p.fillRect(r, halo);
        if (themeId_ == QStringLiteral("neon")) LinWinVisual::drawNeonLandscape(p,r);
    }
    void rebuild()
    {
        if (themeId_ == QStringLiteral("minimal") || width() <= 0 || height() <= 0) return;
        // Bound the offscreen target; use logical painting for the final scale.
        QSize pixels(qMax(1, qRound(width()*devicePixelRatioF())),
                     qMax(1, qRound(height()*devicePixelRatioF())));
        if (pixels.width() > 2560 || pixels.height() > 1440)
            pixels.scale(2560, 1440, Qt::KeepAspectRatio);
        if (pixels.width() < 1 || pixels.height() < 1) return;
        QImage frame;
#ifdef LINWIN_HAS_OPENGL
        if (themeId_ == QStringLiteral("neon") && gpuPreferred() && effectsEnabled() && !gpuFailed_
            && QApplication::platformName() != QStringLiteral("offscreen")
            && QApplication::platformName() != QStringLiteral("minimal")) {
            frame = renderShader(pixels);
            if (frame.isNull()) gpuFailed_ = true;
        }
#endif
        const bool usedShader = !frame.isNull();
        if (usedShader) { QPainter painter(&frame); LinWinVisual::drawNeonLandscape(painter,frame.rect()); }
        if (frame.isNull()) {
            frame = QImage(pixels, QImage::Format_RGB32);
            if (frame.isNull()) return;
            QPainter painter(&frame);
            paintFallback(painter, frame.rect());
        }
        setProperty("renderBackend", usedShader ? QStringLiteral("OpenGL") : QStringLiteral("QPainter"));
        wallpaper_ = frame;
        ++rebuildCount_;
        update();
    }
#ifdef LINWIN_HAS_OPENGL
    static QImage renderShader(const QSize &size)
    {
        QSurfaceFormat format;
        format.setRenderableType(QSurfaceFormat::OpenGL);
        format.setVersion(2, 0);
        format.setProfile(QSurfaceFormat::NoProfile);
        QOpenGLContext context;
        context.setFormat(format);
        if (!context.create()) return {};
        QOffscreenSurface surface;
        surface.setFormat(context.format());
        surface.create();
        if (!surface.isValid() || !context.makeCurrent(&surface)) return {};
        QImage result;
        {
            auto *gl = context.functions();
            QOpenGLFramebufferObject fbo(size);
            QOpenGLShaderProgram program;
            if (fbo.isValid() && fbo.bind()
                && program.addShaderFromSourceCode(QOpenGLShader::Vertex, LinWinVisual::vertexShader)
                && program.addShaderFromSourceCode(QOpenGLShader::Fragment, LinWinVisual::fragmentShader)
                && program.link() && program.bind()) {
                gl->glViewport(0, 0, size.width(), size.height());
                program.setUniformValue("aspect", static_cast<float>(size.width()) / static_cast<float>(size.height()));
                program.setUniformValue("glow", 1.0f);
                const GLfloat vertices[] = {-1.f,-1.f,1.f,-1.f,-1.f,1.f,1.f,1.f};
                const int loc = program.attributeLocation("position");
                if (loc >= 0) {
                    gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
                    program.enableAttributeArray(loc);
                    program.setAttributeArray(loc, GL_FLOAT, vertices, 2);
                    gl->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                    program.disableAttributeArray(loc);
                    if (gl->glGetError() == GL_NO_ERROR) result = fbo.toImage();
                }
                program.release();
                fbo.release();
            }
        }
        context.doneCurrent();
        return result;
    }
#endif
    QString themeId_ = QStringLiteral("neon");
    QTimer timer_;
    QImage wallpaper_;
    quint64 rebuildCount_=0;
    bool effects_ = true;
    bool preferGpu_ = true;
    bool gpuFailed_ = false;
};
