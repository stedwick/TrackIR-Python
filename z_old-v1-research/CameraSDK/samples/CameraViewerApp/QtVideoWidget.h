#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QOpenGLWindow>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QColor>
#include <QByteArray>
#include <atomic>

#include "cameralibrary.h"

class VideoWidget : public QOpenGLWindow, protected QOpenGLFunctions {
    Q_OBJECT
public:
    explicit VideoWidget(UpdateBehavior behavior = QOpenGLWindow::NoPartialUpdate);
    ~VideoWidget() override;

public slots:
    void updateFrameFromBitmap(CameraLibrary::Bitmap* bmp);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    GLuint gl_texture = 0;
    int    texture_width = 0;
    int    texture_height = 0;
    QOpenGLVertexArrayObject vertex_array;
    QOpenGLBuffer            vertext_buffer{QOpenGLBuffer::VertexBuffer};
    std::unique_ptr<QOpenGLShaderProgram> program_shader;
    int position_attribute = -1;
    int uv_attribute  = -1;
    int sampler_uniform  = -1;

    int   frame_width = 0;
    int   frame_height = 0;
    QColor background_color = Qt::black;

    QByteArray byte_array_staging;
    int pending_width = 0;
    int pending_height = 0;
    int pending_bpp = 0;
    int pending_stride = 0;
    std::atomic<bool> has_pending{false};

    // Swizzle cache to avoid re-setting every frame
    enum class SwizzleMode { DefaultRGBA, RedToRGB };
    SwizzleMode swizzle_mode = SwizzleMode::DefaultRGBA;
    GLint  current_internal_format = 0;
    GLenum current_format = 0;
    GLenum current_type = 0;
    int    current_bpp = 0;

private:
    void ensureProgram();
    void ensureVaoVbo();
    void updateQuad(float dstX, float dstY, float dstW, float dstH);
    void setSwizzleIfNeeded(SwizzleMode want);
};

#endif // VIDEOWIDGET_H
