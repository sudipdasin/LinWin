// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "wallpaper_shader.h"
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

    bool effectsEnabled() const { return effects_; }
    bool gpuPreferred() const { return preferGpu_; }
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
        timer_.start();
    }
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        if (wallpaper_.isNull()) paintFallback(painter, rect());
        else {
            painter.setRenderHint(QPainter::SmoothPixmapTransform);
            painter.drawImage(rect(), wallpaper_);
        }
    }

private:
    void invalidate()
    {
        wallpaper_ = {};
        update();
        timer_.start();
    }
    void paintFallback(QPainter &p, const QRect &r) const
    {
        QLinearGradient base(r.bottomLeft(), r.topRight());
        base.setColorAt(0.0, QColor(8, 22, 49));
        base.setColorAt(1.0, QColor(22, 65, 111));
        p.fillRect(r, base);
        if (!effects_) return;
        const QPointF center(r.left() + r.width()*0.72, r.top() + r.height()*0.43);
        QRadialGradient halo(center, qMax(1.0, r.width()*0.50));
        halo.setColorAt(0.0, QColor(48, 156, 224, 130));
        halo.setColorAt(1.0, QColor(48, 156, 224, 0));
        p.fillRect(r, halo);
    }
    void rebuild()
    {
        if (width() <= 0 || height() <= 0) return;
        // Bound the offscreen target; use logical painting for the final scale.
        QSize pixels(qMax(1, qRound(width()*devicePixelRatioF())),
                     qMax(1, qRound(height()*devicePixelRatioF())));
        if (pixels.width() > 2560 || pixels.height() > 1440)
            pixels.scale(2560, 1440, Qt::KeepAspectRatio);
        if (pixels.width() < 1 || pixels.height() < 1) return;
        QImage frame;
#ifdef LINWIN_HAS_OPENGL
        if (preferGpu_ && effects_ && !gpuFailed_
            && QApplication::platformName() != QStringLiteral("offscreen")
            && QApplication::platformName() != QStringLiteral("minimal")) {
            frame = renderShader(pixels);
            if (frame.isNull()) gpuFailed_ = true;
        }
#endif
        const bool usedShader = !frame.isNull();
        if (frame.isNull()) {
            frame = QImage(pixels, QImage::Format_RGB32);
            if (frame.isNull()) return;
            QPainter painter(&frame);
            paintFallback(painter, frame.rect());
        }
        setProperty("renderBackend", usedShader ? QStringLiteral("OpenGL") : QStringLiteral("QPainter"));
        wallpaper_ = frame;
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
    QTimer timer_;
    QImage wallpaper_;
    bool effects_ = true;
    bool preferGpu_ = true;
    bool gpuFailed_ = false;
};
