#ifndef GLWIDGET_H
#define GLWIDGET_H

#include "sprite.h"
#include "math.h"
#include <QWidget>
#include <QOpenGLWidget>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>

enum ViewportMode {
    SPRITESHEET,
    SPRITEEDITOR
};
class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    int frameWidth, frameHeight;
    std::vector<Rect> freeRects;
    ViewportMode mode;

    void changeMode(ViewportMode newMode);
    void resetZoom();

    explicit GLWidget(QWidget *parent = nullptr);
    void addFreeRect(const Rect& freeRect);
    Vec2f convertScreenSpaceToWorldSpace(const Vec2f& point) const;
    Vec2f convertWorldSpaceToScreenSpace(const Vec2f& point) const;
protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
signals:
private:
    void drawLine(QMatrix4x4& viewMat, Vec2f start, Vec2f end, float thickness, bool screenSpaceThickness, QVector4D color);

    QOpenGLShaderProgram* spriteShaderProgram;
    QOpenGLShaderProgram* checkerShaderProgram;
    QOpenGLShaderProgram* solidColorShaderProgram;

    QOpenGLVertexArrayObject texturedQuadVAO;
    QOpenGLBuffer texturedQuadVBO{QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer texturedQuadEBO{QOpenGLBuffer::IndexBuffer};
    QOpenGLVertexArrayObject solidQuadVAO;
    QOpenGLBuffer solidQuadVBO{QOpenGLBuffer::VertexBuffer};
    QOpenGLBuffer solidQuadEBO{QOpenGLBuffer::IndexBuffer};


    Vec2f viewMin;
    Vec2f viewMax;
    bool panning;
    Vec2f panMouseAnchor;

    void resetViewHeight();
};

#endif // GLWIDGET_H
