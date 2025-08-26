#ifndef MAP_H
#define MAP_H

#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QVector3D>
#include <QMouseEvent>
#include <QWheelEvent>

#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QVector>
#include <QString>
#include <QPoint>
#include <QPainter>

class Map : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
public:
    explicit Map(QWidget *parent = nullptr);
    ~Map();

    void setXYZData(const QVector<QVector<QString>> &xyzData);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    bool project(const QVector3D &point, const QMatrix4x4 &modelview, const QMatrix4x4 &projection, const QSize &viewport, float &winX, float &winY, float &winZ);

private:
    QOpenGLShaderProgram *m_program;
    QOpenGLShaderProgram *m_pointProgram;
    Map *m_mapWidget;
    void drawAxis();
    void drawSphere(float radius);
    void drawPoints(const QVector<QVector3D> &points, const QVector<QVector4D> &colors);

    float m_xRotation;
    float m_yRotation;
    float m_scaleFactor;
    QPoint m_lastPos;
    QVector<QVector<QString>> XYZ;

    struct LegendItem {
        QString text;
        QPoint position;
        // 重载 operator== 操作符
        bool operator==(const LegendItem &other) const {
            return text == other.text && position == other.position;
        }
    };

    QList<LegendItem> legendItems;
    QSet<QString> addedLegends;  // 记录已添加的图例文本
};

#endif // MAP_H
