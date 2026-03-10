#include "glwidget.h"
#include <GL/gl.h>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QMouseEvent>
#include <iostream>
#include "spriteopera.h"

GLWidget::GLWidget(QWidget *parent) : QOpenGLWidget{parent}, viewMin(0.f, 0.f), viewMax(1.f, -1.f), panning(false), panMouseAnchor(0.f, 0.f), frameWidth(100), frameHeight(100), mode(SPRITESHEET) {
}

void GLWidget::addFreeRect(const Rect& freeRect) {
    freeRects.push_back(freeRect);
}

Vec2f GLWidget::convertScreenSpaceToWorldSpace(const Vec2f& point) const {
    return {
        std::lerp(viewMin.x, viewMax.x, point.x),
        std::lerp(viewMin.y, viewMax.y, point.y)
    };
}
Vec2f GLWidget::convertWorldSpaceToScreenSpace(const Vec2f& point) const {
    return {
        inverseLerp(point.x, viewMin.x, viewMax.x),
        inverseLerp(point.y, viewMin.y, viewMax.y)
    };
}
void GLWidget::changeMode(ViewportMode newMode) {
    mode = newMode;
    update();
}
void GLWidget::resetZoom() {
    if (SpriteOpera::inst().spritesheets.size() > 0 && SpriteOpera::inst().currentSpritesheetIndex != -1) {
        float targetAspectRatio = (float)frameWidth / (float)frameHeight;

        std::shared_ptr<VirtualSpritesheet> spritesheet = SpriteOpera::inst().spritesheets.at(SpriteOpera::inst().currentSpritesheetIndex);
        viewMin = {0.f, 0.f};
        float sw = (float)spritesheet->getTotalWidth();
        float sh = (float)spritesheet->getTotalHeight();
        viewMax = {sw, sh};
        float sheetAspectRatio = sw / sh;
        if (targetAspectRatio < sheetAspectRatio) { // touching left and right
            viewMax.y = sw / targetAspectRatio;
            float offset = (sh - (viewMax.y - viewMin.y)) / 2.f;
            viewMin.y += offset;
            viewMax.y += offset;
        } else { // touching top and bottom
            viewMax.x = sh * targetAspectRatio;
            float offset = (sw - (viewMax.x - viewMin.x)) / 2.f;
            viewMin.x += offset;
            viewMax.x += offset;
        }

        std::swap(viewMin.y, viewMax.y);

        // zoom out slightly
        float multiply = 1.1f;
        Vec2f center = {(viewMin.x + viewMax.x) / 2.f, (viewMin.y + viewMax.y) / 2.f};
        viewMin = {(viewMin.x - center.x) * multiply + center.x, (viewMin.y - center.y) * multiply + center.y};
        viewMax = {(viewMax.x - center.x) * multiply + center.x, (viewMax.y - center.y) * multiply + center.y};

        update();
    }
}

void GLWidget::initializeGL() {
    initializeOpenGLFunctions();

    // gl settings
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.7, 0.6, 0.6, 1);



    // init sprite shader program
    // define source code
    QOpenGLShader* spriteVertexShader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    const char* spriteVertexShaderSource =
        "attribute highp vec3 a_vertex;\n"
        "attribute mediump vec2 a_texCoord;\n"
        "varying mediump vec2 v_texCoord;\n"
        "uniform mediump mat4 u_matrix;\n"
        "void main(void) {\n"
        "    gl_Position = u_matrix * vec4(a_vertex, 1.);\n"
        "    v_texCoord = a_texCoord;\n"
        "}\n";
    QOpenGLShader* spriteFragmentShader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    const char* spriteFragmentShaderSource =
        "uniform sampler2D u_texture;\n"
        "uniform sampler2D u_normalMap;\n"
        "uniform vec2 u_uvMin;\n"
        "uniform vec2 u_uvMax;\n"
        "varying vec2 v_texCoord;\n"
        "void main(void) {\n"
        "    vec2 uv = mix(u_uvMin, u_uvMax, v_texCoord);\n"
        "    uv.y = 1.0 - uv.y;\n"
        "    vec3 normal = texture2D(u_normalMap, uv).rgb;\n"
        "    vec3 sun = normalize(vec3(-1., -1., 1.));\n"
        "    vec4 color = texture2D(u_texture, uv);\n"
        "    color.rgb *= dot(sun, normal);\n"
        "    gl_FragColor = color;\n"
        "}\n";

    // compile
    spriteVertexShader->compileSourceCode(spriteVertexShaderSource);
    spriteFragmentShader->compileSourceCode(spriteFragmentShaderSource);

    // display errors
    if (!spriteVertexShader->compileSourceCode(spriteVertexShaderSource)) {
        std::cout << "Vertex Shader Compile Error:" << spriteVertexShader->log().toStdString() << std::endl;
    }
    if (!spriteFragmentShader->compileSourceCode(spriteFragmentShaderSource)) {
        std::cout << "Fragment Shader Compile Error:" << spriteFragmentShader->log().toStdString() << std::endl;
    }

    spriteShaderProgram = new QOpenGLShaderProgram();
    spriteShaderProgram->addShader(spriteVertexShader);
    spriteShaderProgram->addShader(spriteFragmentShader);
    spriteShaderProgram->bindAttributeLocation("a_vertex", 0);
    spriteShaderProgram->bindAttributeLocation("a_texCoord", 1);

    spriteShaderProgram->link();

    spriteShaderProgram->bind();
    spriteShaderProgram->setUniformValue("u_texture", 0);
    spriteShaderProgram->setUniformValue("u_normalMap", 1);

    // set up vertex attributes
    int spriteVertexAttributeLocation = spriteShaderProgram->attributeLocation("a_vertex");
    int spriteTexCoordAttributeLocation = spriteShaderProgram->attributeLocation("a_texCoord");

    // unbind
    spriteShaderProgram->release();


    // init checkerboard shader program
    // define source code
    QOpenGLShader* checkerVertexShader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    const char* checkerVertexShaderSource =
        "attribute highp vec3 a_vertex;\n"
        "attribute mediump vec2 a_texCoord;\n"
        "varying mediump vec2 v_texCoord;\n"
        "uniform mediump mat4 u_matrix;\n"
        "void main(void) {\n"
        "    gl_Position = u_matrix * vec4(a_vertex, 1.);\n"
        "    v_texCoord = a_texCoord;\n"
        "}\n";
    QOpenGLShader* checkerFragmentShader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    const char* checkerFragmentShaderSource =
        "uniform vec3 color1;\n"
        "uniform vec3 color2;\n"
        "uniform int width;\n"
        "uniform int height;\n"
        "uniform int tileSize;\n"
        "varying vec2 v_texCoord;\n"
        "void main(void) {\n"
        "    float checkerX = (v_texCoord.x * float(width)) / float(tileSize);\n"
        "    float checkerY = (v_texCoord.y * float(height)) / float(tileSize);\n"
        "    gl_FragColor = mod(checkerY, 2.) < 1. ? (mod(checkerX, 2.) < 1. ? vec4(color1, 1.0) : vec4(color2, 1.0)) : (mod(checkerX, 2.) < 1. ? vec4(color2, 1.0) : vec4(color1, 1.0));\n"
        "}\n";

    // compile
    checkerVertexShader->compileSourceCode(checkerVertexShaderSource);
    checkerFragmentShader->compileSourceCode(checkerFragmentShaderSource);

    // display errors
    if (!checkerVertexShader->compileSourceCode(checkerVertexShaderSource)) {
        std::cout << "Vertex Shader Compile Error:" << checkerVertexShader->log().toStdString() << std::endl;
    }
    if (!checkerFragmentShader->compileSourceCode(checkerFragmentShaderSource)) {
        std::cout << "Fragment Shader Compile Error:" << checkerFragmentShader->log().toStdString() << std::endl;
    }

    checkerShaderProgram = new QOpenGLShaderProgram();
    checkerShaderProgram->addShader(checkerVertexShader);
    checkerShaderProgram->addShader(checkerFragmentShader);
    checkerShaderProgram->bindAttributeLocation("a_vertex", 0);
    checkerShaderProgram->bindAttributeLocation("a_texCoord", 1);

    checkerShaderProgram->link();

    checkerShaderProgram->bind();

    // set up vertex attributes
    int checkerVertexAttributeLocation = checkerShaderProgram->attributeLocation("a_vertex");
    int checkerTexCoordAttributeLocation = checkerShaderProgram->attributeLocation("a_texCoord");

    // unbind
    checkerShaderProgram->release();


    // init solid color shader program
    // define source code
    QOpenGLShader* solidColorVertexShader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    const char* solidColorVertexShaderSource =
        "attribute highp vec3 a_vertex;\n"
        "uniform mediump mat4 u_matrix;\n"
        "void main(void) {\n"
        "    gl_Position = u_matrix * vec4(a_vertex, 1.);\n"
        "}\n";
    QOpenGLShader* solidColorFragmentShader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    const char* solidColorFragmentShaderSource =
        "uniform vec4 u_color;\n"
        "void main(void) {\n"
        "    gl_FragColor = u_color;\n"
        "}\n";

    // compile
    solidColorVertexShader->compileSourceCode(solidColorVertexShaderSource);
    solidColorFragmentShader->compileSourceCode(solidColorFragmentShaderSource);

    // display errors
    if (!solidColorVertexShader->compileSourceCode(solidColorVertexShaderSource)) {
        std::cout << "Vertex Shader Compile Error:" << solidColorVertexShader->log().toStdString() << std::endl;
    }
    if (!solidColorFragmentShader->compileSourceCode(solidColorFragmentShaderSource)) {
        std::cout << "Fragment Shader Compile Error:" << solidColorFragmentShader->log().toStdString() << std::endl;
    }

    solidColorShaderProgram = new QOpenGLShaderProgram();
    solidColorShaderProgram->addShader(solidColorVertexShader);
    solidColorShaderProgram->addShader(solidColorFragmentShader);
    solidColorShaderProgram->bindAttributeLocation("a_vertex", 0);

    solidColorShaderProgram->link();

    solidColorShaderProgram->bind();

    // set up vertex attributes
    int solidColorVertexAttributeLocation = solidColorShaderProgram->attributeLocation("a_vertex");


    // unbind
    solidColorShaderProgram->release();

    // init texture quad ==============================================
    static const float texturedQuadVertices[] = { // x, y, z, u, v
        0.f, 0.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f, 1.f,
        1.f, 1.f, 0.f, 1.f, 1.f,
        1.f, 0.f, 0.f, 1.f, 0.f
    };
    static const unsigned int texturedQuadIndices[] = {
        0, 1, 2,
        0, 2, 3
    };

    texturedQuadVAO.create();
    texturedQuadVAO.bind();

    texturedQuadVBO.create();
    texturedQuadVBO.bind();
    texturedQuadVBO.allocate(texturedQuadVertices, sizeof(texturedQuadVertices));

    texturedQuadEBO.create();
    texturedQuadEBO.bind();
    texturedQuadEBO.allocate(texturedQuadIndices, sizeof(texturedQuadIndices));

    //set attribute buffers
    spriteShaderProgram->enableAttributeArray(spriteVertexAttributeLocation);
    spriteShaderProgram->setAttributeBuffer(spriteVertexAttributeLocation, GL_FLOAT, 0, 3, 5 * sizeof(float));

    spriteShaderProgram->enableAttributeArray(spriteTexCoordAttributeLocation);
    spriteShaderProgram->setAttributeBuffer(spriteTexCoordAttributeLocation, GL_FLOAT, 3 * sizeof(float), 2, 5 * sizeof(float));

    checkerShaderProgram->enableAttributeArray(checkerVertexAttributeLocation);
    checkerShaderProgram->setAttributeBuffer(checkerVertexAttributeLocation, GL_FLOAT, 0, 3, 5 * sizeof(float));

    checkerShaderProgram->enableAttributeArray(checkerTexCoordAttributeLocation);
    checkerShaderProgram->setAttributeBuffer(checkerTexCoordAttributeLocation, GL_FLOAT, 3 * sizeof(float), 2, 5 * sizeof(float));

    texturedQuadVAO.release();
    texturedQuadVBO.release();
    texturedQuadEBO.release();


    // init color quad
    static const float solidQuadVertices[] = { // x, y, z
        0.f, 0.f, 0.f,
        0.f, 1.f, 0.f,
        1.f, 1.f, 0.f,
        1.f, 0.f, 0.f
    };
    static const unsigned int solidQuadIndices[] = {
        0, 1, 2,
        0, 2, 3
    };

    solidQuadVAO.create();
    solidQuadVAO.bind();

    solidQuadVBO.create();
    solidQuadVBO.bind();
    solidQuadVBO.allocate(solidQuadVertices, sizeof(solidQuadVertices));

    solidQuadEBO.create();
    solidQuadEBO.bind();
    solidQuadEBO.allocate(solidQuadIndices, sizeof(solidQuadIndices));

    // set attribute buffers
    solidColorShaderProgram->enableAttributeArray(solidColorVertexAttributeLocation);
    solidColorShaderProgram->setAttributeBuffer(solidColorVertexAttributeLocation, GL_FLOAT, 0, 3, 3 * sizeof(float));

    solidQuadVAO.release();
    solidQuadVBO.release();
    solidQuadEBO.release();

    resetViewHeight();
}
void GLWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);

    frameWidth = w;
    frameHeight = h;

    resetViewHeight();
}
void GLWidget::drawLine(QMatrix4x4& viewMat, Vec2f start, Vec2f end, float thickness, bool screenSpaceThickness, QVector4D color) {
    QMatrix4x4 modelMat;
    modelMat.setToIdentity();

    float length = std::sqrt((end.x - start.x) * (end.x - start.x) + (end.y - start.y) * (end.y - start.y));
    Vec2f center = {(start.x + end.x) * 0.5f, (start.y + end.y) * 0.5f};
    float angle = std::atan2(end.y - start.y, end.x - start.x);
    if (screenSpaceThickness) {
        float viewWidth = viewMax.x - viewMin.x;
        thickness *= viewWidth / (float)frameWidth;
    }

    modelMat.translate(center.x, center.y);
    modelMat.rotate(angle * (180.f / M_PI), QVector3D(0.f, 0.f, 1.f));
    modelMat.scale(length, thickness);
    modelMat.translate(-0.5f, -0.5f); // center quad

    QMatrix4x4 transMat = viewMat * modelMat;


    solidColorShaderProgram->bind();
    solidColorShaderProgram->setUniformValue("u_matrix", transMat);
    solidColorShaderProgram->setUniformValue("u_color", color);

    solidQuadVAO.bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    solidQuadVAO.release();
    solidColorShaderProgram->release();
}
void GLWidget::paintGL() {
    glClearColor(0.7, 0.6, 0.6, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QMatrix4x4 viewMat;
    viewMat.setToIdentity();
    viewMat.ortho(viewMin.x, viewMax.x, viewMin.y, viewMax.y, -1.f, 1.f);

    // render background
    QMatrix4x4 fullscreenQuadMat;
    fullscreenQuadMat.setToIdentity();
    fullscreenQuadMat.translate(-1.f, -1.f);
    fullscreenQuadMat.scale(2.f, 2.f);
    checkerShaderProgram->bind();
    checkerShaderProgram->setUniformValue("u_matrix", fullscreenQuadMat);
    checkerShaderProgram->setUniformValue("color1", QVector3D(0.3f, 0.3f, 0.3f));
    checkerShaderProgram->setUniformValue("color2", QVector3D(0.35f, 0.35f, 0.35f));
    checkerShaderProgram->setUniformValue("width", frameWidth);
    checkerShaderProgram->setUniformValue("height", frameHeight);
    checkerShaderProgram->setUniformValue("tileSize", 10);
    texturedQuadVAO.bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    texturedQuadVAO.release();
    checkerShaderProgram->release();

    if (mode == SPRITESHEET) {
        // render sprites
        if (SpriteOpera::inst().spritesheets.size() > 0 && SpriteOpera::inst().currentSpritesheetIndex != -1) {
            std::shared_ptr<VirtualSpritesheet> spritesheet = SpriteOpera::inst().spritesheets.at(SpriteOpera::inst().currentSpritesheetIndex);
            for (const SpritePositionedInRaster& sprite : spritesheet->sprites) {
                if ((float)sprite.position.x > viewMax.x || (float)(sprite.position.y + sprite.getHeight()) < viewMax.y ||
                    (float)(sprite.position.x + sprite.getWidth()) < viewMin.x ||
                    (float)(sprite.position.y) > viewMin.y) {
                    continue;
                }
                QMatrix4x4 modelMat;
                modelMat.setToIdentity();
                modelMat.translate(sprite.position.x, sprite.position.y);
                modelMat.scale(
                    (float)(sprite.sprite.region->getMax().x - sprite.sprite.region->getMin().x),
                    (float)(sprite.sprite.region->getMax().y - sprite.sprite.region->getMin().y)
                    );

                QMatrix4x4 transMat = viewMat * modelMat;

                spriteShaderProgram->bind();
                spriteShaderProgram->setUniformValue("u_matrix", transMat);
                float imageWidth = static_cast<float>(sprite.sprite.image->getWidth());
                float imageHeight = static_cast<float>(sprite.sprite.image->getHeight());
                spriteShaderProgram->setUniformValue("u_uvMin", QVector2D(static_cast<float>(sprite.sprite.region->getMin().x) / imageWidth, static_cast<float>(sprite.sprite.region->getMin().y) / imageHeight));
                spriteShaderProgram->setUniformValue("u_uvMax", QVector2D(static_cast<float>(sprite.sprite.region->getMax().x) / imageWidth, static_cast<float>(sprite.sprite.region->getMax().y) / imageHeight));

                sprite.sprite.image->bindTexture();

                texturedQuadVAO.bind();
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

                texturedQuadVAO.release();
                spriteShaderProgram->release();

                // outlines
                QVector4D lineColor{0.8f, 0.8f, 0.1f, 1.f};
                drawLine(
                    viewMat,
                    {(float)(sprite.position.x), (float)(sprite.position.y)},
                    {(float)(sprite.position.x), (float)(sprite.position.y + sprite.getHeight())},
                    2.f, true, lineColor
                    );
                drawLine(
                    viewMat,
                    {(float)(sprite.position.x), (float)(sprite.position.y + sprite.getHeight())},
                    {(float)(sprite.position.x + sprite.getWidth()), (float)(sprite.position.y + sprite.getHeight())},
                    2.f, true, lineColor
                    );
                drawLine(
                    viewMat,
                    {(float)(sprite.position.x + sprite.getWidth()), (float)(sprite.position.y + sprite.getHeight())},
                    {(float)(sprite.position.x + sprite.getWidth()), (float)(sprite.position.y)},
                    2.f, true, lineColor
                    );
                drawLine(
                    viewMat,
                    {(float)(sprite.position.x + sprite.getWidth()), (float)(sprite.position.y)},
                    {(float)(sprite.position.x), (float)(sprite.position.y)},
                    2.f, true, lineColor
                    );
            }
            // render free rects
            for (size_t i = 0; i < freeRects.size(); i++) {
                QMatrix4x4 modelMat;
                modelMat.setToIdentity();
                modelMat.translate(freeRects[i].min.x(), freeRects[i].min.y());
                modelMat.scale((float)(freeRects[i].max.x() - freeRects[i].min.x() + 1), (float)(freeRects[i].max.y() - freeRects[i].min.y() + 1));

                QMatrix4x4 transMat = viewMat * modelMat;

                solidColorShaderProgram->bind();
                solidColorShaderProgram->setUniformValue("u_matrix", transMat);
                solidColorShaderProgram->setUniformValue("u_color", QVector4D(0.f, 0.f, 1.f, 0.2f));

                solidQuadVAO.bind();
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

                solidQuadVAO.release();
                solidColorShaderProgram->release();
            }
        }
    } else if (mode == SPRITEEDITOR) {

    }
}
void GLWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        panning = true;
        QPointF pos = event->position();
        Vec2f screenSpacePos = {(float)pos.x() / (float)frameWidth, 1.f - (float)pos.y() / (float)frameHeight};
        panMouseAnchor = convertScreenSpaceToWorldSpace({screenSpacePos.x, screenSpacePos.y});
    }
}
void GLWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) panning = false;
}
void GLWidget::mouseMoveEvent(QMouseEvent *event) { // only gets called while mouse is down by default
    if (panning) {
        QPointF mousePos = event->position();
        Vec2f screenSpaceMouse = {(float)mousePos.x() / (float)frameWidth, 1.f - (float)mousePos.y() / (float)frameHeight};

        Vec2f screenSpacePanAnchor = convertWorldSpaceToScreenSpace(panMouseAnchor);
        Vec2f newViewCenter = convertScreenSpaceToWorldSpace(screenSpacePanAnchor - screenSpaceMouse + 0.5f);

        float viewWidth  = viewMax.x - viewMin.x;
        float viewHeight = viewMax.y - viewMin.y;

        viewMin = {
            newViewCenter.x - viewWidth / 2.f,
            newViewCenter.y - viewHeight / 2.f
        };
        viewMax = {
            newViewCenter.x + viewWidth / 2.f,
            newViewCenter.y + viewHeight / 2.f
        };

        update(); // trigger repaint
    }
}
void GLWidget::wheelEvent(QWheelEvent *event) {
    QPointF mousePos = event->position();
    Vec2f screenSpaceMouse = {(float)mousePos.x() / (float)frameWidth, 1.F - (float)mousePos.y() / (float)frameHeight};
    QPoint scrollDelta = event->angleDelta();

    Vec2f center = convertScreenSpaceToWorldSpace(screenSpaceMouse);
    float zoomMult = std::pow(1.1f, -(float)scrollDelta.y() / 120.f);
    viewMin.x = (viewMin.x - center.x) * zoomMult + center.x;
    viewMin.y = (viewMin.y - center.y) * zoomMult + center.y;
    viewMax.x = (viewMax.x - center.x) * zoomMult + center.x;
    viewMax.y = (viewMax.y - center.y) * zoomMult + center.y;
    update(); // trigger repaint
}

void GLWidget::resetViewHeight() {
    float viewWidth = viewMax.x - viewMin.x;
    float viewCenterY = (viewMin.y + viewMax.y) / 2.f;
    float ratio = (float)frameHeight / (float)frameWidth;
    float viewHeight = viewWidth * ratio;
    viewMin.y = viewCenterY + viewHeight / 2.f;
    viewMax.y = viewCenterY - viewHeight / 2.f;
}
