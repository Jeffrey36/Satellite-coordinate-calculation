#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <cmath>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QRegularExpression>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QTableView>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QtOpenGLWidgets>
#include "map.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void setgbFile();
    void setjmFile();
    void readgbFile();
    void readjmFile();
    void calculateData();
    QString formatPreciseTime(const QString &fullTime);
    void showGraphicsView();
    void openUrl();
    void openUrl2();

private:
    Ui::MainWindow *ui;
    QVector<QVector<QString>> stringDataBlocks;
    QVector<QVector<double>> numericDataBlocks;
    QMap<QString, QMap<QString, QVector<double>>> preciseEphemerisData;
    bool parseBlock(QTextStream &in, QVector<QVector<QString>> &stringDataBlocks, QVector<QVector<double>> &numericDataBlocks, int &cCount, int &gCount);
    QList<QPair<QString, QTime>> findClosestTimes(const QString &inputTime, const QMap<QString, QMap<QString, QVector<double>>> &preciseEphemerisData, int numClosest);
    QMap<QString, QVector<QVector<double>>> extractClosestCoordinates(const QList<QPair<QString, QTime>> &closestTimes, const QMap<QString, QMap<QString, QVector<double>>> &preciseEphemerisData);
    QPair<QVector<double>, bool> lagrangeInterpolation(const QList<QPair<QString, QTime>> &times, const QVector<QVector<double>> &coordinates, const QString &targetTime);
    QGraphicsView *m_graphicsView;
    QGraphicsScene *m_graphicsScene;
    Map *m_customOpenGLWidget;
    QTableView *tableView;
    QStandardItemModel *model;
    QStringListModel *m_textModel;
    QListWidget *m_listWidget;
};

#endif // MAINWINDOW_H
