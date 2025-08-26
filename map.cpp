#include "map.h"
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <cmath>
#include <QMouseEvent>
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <QPainter>

Map::Map(QWidget *parent)
    : QOpenGLWidget(parent), m_program(nullptr), m_xRotation(0), m_yRotation(0), m_scaleFactor(1.0f) {
    XYZ = {
        // 第一个 T
        {"T", "-5", "-3.00", "1.00"}, {"T", "-5", "-2.80", "1.00"}, {"T", "-5", "-2.60", "1.00"}, {"T", "-5", "-2.40", "1.00"}, {"T", "-5", "-2.20", "1.00"}, {"T", "-5", "-2.00", "1.00"}, {"T", "-5", "-1.80", "1.00"}, {"T", "-5", "-1.60", "1.00"},
        {"T", "-5", "-2.30", "-0.80"}, {"T", "-5", "-2.30", "-0.60"}, {"T", "-5", "-2.30", "-0.40"}, {"T", "-5", "-2.30", "-0.20"}, {"T", "-5", "-2.30", "0.00"}, {"T", "-5", "-2.30", "0.20"}, {"T", "-5", "-2.30", "0.40"}, {"T", "-5", "-2.30", "0.60"}, {"T", "-5", "-2.30", "0.80"}, {"T", "-5", "-2.30", "-1.00"},
        // E
        {"E", "-5", "-1.20", "1.00"}, {"E", "-5", "-1.00", "1.00"}, {"E", "-5", "-0.80", "1.00"}, {"E", "-5", "-0.60", "1.00"}, {"E", "-5", "-0.40", "1.00"}, {"E", "-5", "-0.20", "1.00"},
        {"E", "-5", "-1.20", "0.00"}, {"E", "-5", "-1.00", "0.00"}, {"E", "-5", "-0.80", "0.00"}, {"E", "-5", "-0.60", "0.00"}, {"E", "-5", "-0.40", "0.00"}, {"E", "-5", "-0.20", "0.00"},
        {"E", "-5", "-1.20", "-1.00"}, {"E", "-5", "-1.00", "-1.00"}, {"E", "-5", "-0.80", "-1.00"}, {"E", "-5", "-0.60", "-1.00"}, {"E", "-5", "-0.40", "-1.00"}, {"E", "-5", "-0.20", "-1.00"},
        {"E", "-5", "-1.20", "1.00"}, {"E", "-5", "-1.20", "0.80"}, {"E", "-5", "-1.20", "0.60"}, {"E", "-5", "-1.20", "0.40"}, {"E", "-5", "-1.20", "0.20"}, {"E", "-5", "-1.20", "0.00"}, {"E", "-5", "-1.20", "-0.20"}, {"E", "-5", "-1.20", "-0.40"}, {"E", "-5", "-1.20", "-0.60"}, {"E", "-5", "-1.20", "-0.80"}, {"E", "-5", "-1.20", "-1.00"},
        // S
        {"S", "-5", "1.25", "0.65"}, {"S", "-5", "1.15", "0.80"}, {"S", "-5", "1.05", "0.92"}, {"S", "-5", "0.90", "0.97"}, {"S", "-5", "0.75", "1.00"},
        {"S", "-5", "0.20", "0.50"}, {"S", "-5", "0.25", "0.65"}, {"S", "-5", "0.35", "0.80"}, {"S", "-5", "0.45", "0.92"}, {"S", "-5", "0.60", "0.97"},
        {"S", "-5", "0.25", "0.35"}, {"S", "-5", "0.35", "0.20"}, {"S", "-5", "0.45", "0.08"}, {"S", "-5", "0.60", "0.03"}, {"S", "-5", "0.75", "0.00"},
        {"S", "-5", "1.25", "-0.35"}, {"S", "-5", "1.15", "-0.20"}, {"S", "-5", "1.05", "-0.08"}, {"S", "-5", "0.90", "-0.03"},
        {"S", "-5", "1.30", "-0.50"}, {"S", "-5", "1.25", "-0.65"}, {"S", "-5", "1.15", "-0.80"}, {"S", "-5", "1.05", "-0.92"}, {"S", "-5", "0.90", "-0.97"},
        {"S", "-5", "0.25", "-0.65"}, {"S", "-5", "0.35", "-0.80"}, {"S", "-5", "0.45", "-0.92"}, {"S", "-5", "0.60", "-0.97"}, {"S", "-5", "0.75", "-1.00"},
        // 第二个 T
        {"T", "-5", "1.60", "1.00"}, {"T", "-5", "1.80", "1.00"}, {"T", "-5", "2.00", "1.00"}, {"T", "-5", "2.20", "1.00"}, {"T", "-5", "2.40", "1.00"}, {"T", "-5", "2.60", "1.00"}, {"T", "-5", "2.80", "1.00"}, {"T", "-5", "3.00", "1.00"},
        {"T", "-5", "2.30", "-0.80"}, {"T", "-5", "2.30", "-0.60"}, {"T", "-5", "2.30", "-0.40"}, {"T", "-5", "2.30", "-0.20"}, {"T", "-5", "2.30", "0.00"}, {"T", "-5", "2.30", "0.20"}, {"T", "-5", "2.30", "0.40"}, {"T", "-5", "2.30", "0.60"}, {"T", "-5", "2.30", "0.80"}, {"T", "-5", "2.30", "-1.00"}
    };
}

Map::~Map() {
    if (m_program) {
        delete m_program;
    }
}

void Map::initializeGL() {
    initializeOpenGLFunctions();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);  // 启用深度测试

    // 初始化球体着色器程序
    m_program = new QOpenGLShaderProgram(this);
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, R"(
        #version 330
        in vec3 vertexPosition;
        uniform mat4 projectionMatrix;
        uniform mat4 viewMatrix;
        uniform mat4 modelMatrix;
        void main() {
            gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vertexPosition, 1.0);
        }
    )");
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, R"(
        #version 330
        out vec4 fragColor;
        void main() {
            fragColor = vec4(1.0, 1.0, 1.0, 1.0);  // 白色
        }
    )");
    m_program->link();

    // 初始化点着色器程序
    m_pointProgram = new QOpenGLShaderProgram(this);
    m_pointProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, R"(
        #version 330
        in vec3 vertexPosition;
        in vec4 vertexColor;
        uniform mat4 projectionMatrix;
        uniform mat4 viewMatrix;
        uniform mat4 modelMatrix;
        out vec4 fragmentColor;
        void main() {
            gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vertexPosition, 1.0);
            fragmentColor = vertexColor;
        }
    )");
    m_pointProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, R"(
        #version 330
        in vec4 fragmentColor;
        out vec4 fragColor;
        void main() {
            fragColor = fragmentColor;
        }
    )");
    m_pointProgram->link();
}


void Map::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
}

void Map::setXYZData(const QVector<QVector<QString>> &xyzData) {
    XYZ = xyzData;
    update();
}

void Map::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // 清空图例列表和已添加的图例集合
    legendItems.clear();
    addedLegends.clear();

    QMatrix4x4 projection;
    projection.perspective(45.0f, width() / float(height()), 0.1f, 100.0f);

    QMatrix4x4 view;
    view.translate(0.0f, 0.0f, -5.0f);
    view.rotate(m_xRotation, 1.0f, 0.0f, 0.0f);
    view.rotate(m_yRotation, 0.0f, 1.0f, 0.0f);
    view.scale(m_scaleFactor);

    QMatrix4x4 model;
    model.scale(1.0f);

    // 绑定球体着色器程序
    m_program->bind();
    m_program->setUniformValue("projectionMatrix", projection);
    m_program->setUniformValue("viewMatrix", view);
    m_program->setUniformValue("modelMatrix", model);

    drawAxis();
    drawSphere(0.6371);

    m_program->release();

    // 绑定点着色器程序
    m_pointProgram->bind();
    m_pointProgram->setUniformValue("projectionMatrix", projection);
    m_pointProgram->setUniformValue("viewMatrix", view);
    m_pointProgram->setUniformValue("modelMatrix", model);

    QVector<QVector3D> points;
    QVector<QVector4D> colors;
    int yPosition = 20;  // 图例的起始 y 坐标

    for (const auto &data : XYZ) {
        QString id = data[0];
        int number = id.mid(1).toInt();
        float x = data[2].toFloat();
        float y = data[3].toFloat();
        float z = data[1].toFloat();
        points.append(QVector3D(x, y, z));

        QString legendText;
        if (id.startsWith('G')) {
            colors.append(QVector4D(0.0f, 1.0f, 1.0f, 1.0f));  // 青色
            legendText = "青色: GPS卫星";
        } else if (id.startsWith('C')) {
            if ((number >= 1 && number <= 5) || (number >= 59 && number <= 62)) {
                colors.append(QVector4D(1.0f, 0.0f, 0.0f, 1.0f));  // 红色
                legendText = "红色: GEO";
            } else if ((number >= 6 && number <= 10) || number == 13 || number == 16 || (number >= 38 && number <= 40)) {
                colors.append(QVector4D(0.0f, 1.0f, 0.0f, 1.0f));  // 绿色
                legendText = "绿色: IGSO";
            } else {
                colors.append(QVector4D(0.0f, 1.0f, 1.0f, 1.0f));  // 青色
                legendText = "青色: MEO";
            }
        } else {
            colors.append(QVector4D(1.0f, 1.0f, 1.0f, 1.0f));  // 白色
            legendText = "白色: 其他";
        }

        if (!addedLegends.contains(legendText)) {
            addedLegends.insert(legendText);
            legendItems.append({legendText, QPoint(10, yPosition)});
            yPosition += 20;
        }
    }

    QVector<QVector<QString>> label= {
                                       {"X", "7.00", "0.0", "0.4"},
                                       {"X", "7.05", "0.0", "0.3"}, {"X", "7.10", "0.0", "0.2"}, {"X", "7.15", "0.0", "0.1"},
                                       {"X", "7.05", "0.0", "0.5"}, {"X", "7.10", "0.0", "0.6"}, {"X", "7.15", "0.0", "0.7"},
                                       {"X", "6.95", "0.0", "0.3"}, {"X", "6.90", "0.0", "0.2"}, {"X", "6.85", "0.0", "0.1"},
                                       {"X", "6.95", "0.0", "0.5"}, {"X", "6.90", "0.0", "0.6"}, {"X", "6.85", "0.0", "0.7"},
                                       {"Y", "0.0", "7.00", "0.4"},
                                       {"Y", "0.0", "7.05", "0.5"}, {"Y", "0.0", "7.10", "0.6"}, {"Y", "0.0", "7.15", "0.7"},
                                       {"Y", "0.0", "6.95", "0.5"}, {"Y", "0.0", "6.90", "0.6"}, {"Y", "0.0", "6.85", "0.7"},
                                       {"Y", "0.0", "7.00", "0.3"}, {"Y", "0.0", "7.00", "0.2"}, {"Y", "0.0", "7.00", "0.1"},
                                       {"Z", "0.0", "0.40", "7.00"},
                                       {"Z", "0.0", "0.45", "7.10"}, {"Z", "0.0", "0.50", "7.20"},
                                       {"Z", "0.0", "0.35", "6.90"}, {"Z", "0.0", "0.30", "6.80"},
                                       {"Z", "0.0", "0.37", "6.80"}, {"Z", "0.0", "0.43", "6.80"}, {"Z", "0.0", "0.50", "6.80"},
                                       {"Z", "0.0", "0.43", "7.20"}, {"Z", "0.0", "0.37", "7.20"}, {"Z", "0.0", "0.30", "7.20"},
                                       };

    for (const auto &data : label) {
        float x = data[2].toFloat();
        float y = data[3].toFloat();
        float z = data[1].toFloat();
        points.append(QVector3D(x, y, z));
        colors.append(QVector4D(1.0f, 1.0f, 1.0f, 1.0f));
    }

    drawPoints(points, colors);

    m_pointProgram->release();
}



bool Map::project(const QVector3D &point, const QMatrix4x4 &modelview, const QMatrix4x4 &projection, const QSize &viewport, float &winX, float &winY, float &winZ) {
    // 3D坐标系转换到像素坐标（失败）
    QVector4D tmp = modelview * projection * QVector4D(point, 1.0f);
    if (tmp.w() == 0.0f) return false;
    tmp /= tmp.w();
    winX = (tmp.x() * 0.5f + 0.5f) * viewport.width();
    winY = (tmp.y() * 0.5f + 0.5f) * viewport.height();
    winZ = (tmp.z() * 0.5f + 0.5f) * 1.0f;
    return true;
}

void Map::mousePressEvent(QMouseEvent *event) {
    m_lastPos = event->pos();
}

void Map::mouseMoveEvent(QMouseEvent *event) {
    int dx = event->x() - m_lastPos.x();
    int dy = event->y() - m_lastPos.y();

    m_xRotation += dy;
    m_yRotation += dx;

    m_lastPos = event->pos();

    QMatrix4x4 view;
    view.translate(0.0f, 0.0f, -5.0f);
    view.rotate(m_xRotation, 1.0f, 0.0f, 0.0f);
    view.rotate(m_yRotation, 0.0f, 1.0f, 0.0f);
    view.scale(m_scaleFactor);

    update();
}

void Map::wheelEvent(QWheelEvent *event) {
    int delta = event->angleDelta().y();
    if (delta > 0) {
        m_scaleFactor *= 1.1f;  // 放大
    } else {
        m_scaleFactor /= 1.1f;  // 缩小
    }
    update();
}

void Map::drawPoints(const QVector<QVector3D> &points, const QVector<QVector4D> &colors) {
    QOpenGLVertexArrayObject vao;
    vao.create();
    vao.bind();

    QOpenGLBuffer vbo;
    vbo.create();
    vbo.bind();

    std::vector<float> vertices;
    for (size_t i = 0; i < points.size(); ++i) {
        vertices.push_back(points[i].x());
        vertices.push_back(points[i].y());
        vertices.push_back(points[i].z());
        vertices.push_back(colors[i].x());
        vertices.push_back(colors[i].y());
        vertices.push_back(colors[i].z());
        vertices.push_back(colors[i].w());
    }

    vbo.allocate(vertices.data(), vertices.size() * sizeof(float));

    // 设置位置属性
    m_pointProgram->enableAttributeArray(0);
    m_pointProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3, 7 * sizeof(float));

    // 设置颜色属性
    m_pointProgram->enableAttributeArray(1);
    m_pointProgram->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(float), 4, 7 * sizeof(float));

    glPointSize(5.0f);
    glDrawArrays(GL_POINTS, 0, points.size());

    vbo.release();
    vao.release();
}

void Map::paintEvent(QPaintEvent *event) {
    QOpenGLWidget::paintEvent(event);
    QPainter painter(this);
    painter.setPen(Qt::white);
    for (const auto &item : legendItems) {
        painter.drawText(item.position, item.text);
    }
    painter.end();
}


void Map::drawAxis() {
    QOpenGLVertexArrayObject vao;
    vao.create();
    vao.bind();

    QOpenGLBuffer vbo;
    vbo.create();
    vbo.bind();

    const float vertices[] = {
        -10.0f, 0.0f, 0.0f,
        10.0f, 0.0f, 0.0f,

        0.0f, -10.0f, 0.0f,
        0.0f, 10.0f, 0.0f,

        0.0f, 0.0f, -10.0f,
        0.0f, 0.0f, 10.0f
    };

    vbo.allocate(vertices, 18 * sizeof(float));

    m_program->enableAttributeArray(0);
    m_program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 0);

    glDrawArrays(GL_LINES, 0, 6);

    vbo.release();
    vao.release();
}

void Map::drawSphere(float radius) {
    const int slices = 50;
    const int stacks = 50;

    QOpenGLVertexArrayObject vao;
    vao.create();
    vao.bind();

    QOpenGLBuffer vbo;
    vbo.create();
    vbo.bind();

    std::vector<float> vertices;
    for (int i = 0; i <= stacks; ++i) {
        float v = i / float(stacks) * M_PI;
        for (int j = 0; j <= slices; ++j) {
            float u = j / float(slices) * 2 * M_PI;
            float x = radius * sin(v) * cos(u);
            float y = radius * sin(v) * sin(u);
            float z = radius * cos(v);
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        }
    }

    vbo.allocate(vertices.data(), vertices.size() * sizeof(float));

    m_program->enableAttributeArray(0);
    m_program->setAttributeBuffer(0, GL_FLOAT, 0, 3, 0);

    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            int index = (i * (slices + 1) + j) * 3;
            glBegin(GL_TRIANGLE_STRIP);
            glVertex3fv(&vertices[index]);
            glVertex3fv(&vertices[index + 3]);
            glVertex3fv(&vertices[index + 3 * (slices + 1)]);
            glVertex3fv(&vertices[index + 3 * (slices + 1) + 3]);
            glEnd();
        }
    }

    vbo.release();
    vao.release();
}
