#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "calculate.h"
#include <QVBoxLayout>
#include <QGraphicsProxyWidget>
#include <QDesktopServices>
#include <QUrl>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_graphicsView(new QGraphicsView(this))
    , m_graphicsScene(new QGraphicsScene(this))
    , m_customOpenGLWidget(new Map(this))
    , m_textModel(new QStringListModel(this))
    , m_listWidget(nullptr)
{
    ui->setupUi(this);

    m_listWidget = ui->listWidget;
    ui->gbButton->setText("选择文件");
    ui->jmButton->setText("选择文件");
    ui->Button3d->setText("生成视图");
    ui->calculateButton->setText("计算");
    ui->categoryComboBox->addItem("导航系统");
    ui->categoryComboBox->addItem("GPS");
    ui->categoryComboBox->addItem("BDS");
    ui->categoryComboBox->setCurrentIndex(0);
    ui->dateLineEdit->setPlaceholderText("YYYYMMDD");
    ui->timeLineEdit->setPlaceholderText("HHMMSS");

    // 连接按钮点击信号和槽函数
    connect(ui->gbButton, &QPushButton::clicked, this, &MainWindow::setgbFile);
    connect(ui->jmButton, &QPushButton::clicked, this, &MainWindow::setjmFile);
    connect(ui->calculateButton, &QPushButton::clicked, this, &MainWindow::calculateData);
    connect(ui->Button3d, &QPushButton::clicked, this, &MainWindow::showGraphicsView);
    connect(ui->urlButton, &QPushButton::clicked, this, &MainWindow::openUrl);
    connect(ui->urlButton2, &QPushButton::clicked, this, &MainWindow::openUrl2);

    // 设置 Graphics View
    m_graphicsView->setScene(m_graphicsScene);
    m_graphicsView->setRenderHint(QPainter::Antialiasing);
    m_graphicsView->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    m_graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    m_graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 获取 UI 文件中的布局管理器
    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(ui->verticalLayout->layout());

    // 添加 QOpenGLWidget 到布局
    layout->addWidget(m_customOpenGLWidget);

    // 初始化 QOpenGLWidget
    m_customOpenGLWidget->setFocusPolicy(Qt::StrongFocus);
    m_customOpenGLWidget->hide();  // 默认隐藏 QOpenGLWidget
}

void MainWindow::showGraphicsView() {
    m_customOpenGLWidget->show();
    m_customOpenGLWidget->update();
}

void MainWindow::openUrl() {
    QDesktopServices::openUrl(QUrl("http://www.igs.gnsswhu.cn/index.php"));
}

void MainWindow::openUrl2() {
    QDesktopServices::openUrl(QUrl("https://cddis.nasa.gov/archive/gnss/data/daily/2024/brdc/"));
}

bool MainWindow::parseBlock(QTextStream &in, QVector<QVector<QString>> &stringDataBlocks, QVector<QVector<double>> &numericDataBlocks, int &cCount, int &gCount)
{
    // 读取当前行
    QString line = in.readLine();
    if (line.isNull()) {
        return false;
    }

    // 提取前23个字符作为索引
    QString indexField = line.left(23).trimmed();

    // 检查索引字段是否以 'C' 或 'G' 开头
    if (!indexField.startsWith('C') && !indexField.startsWith('G')) {
        return true;
    }

    line = line.mid(23);    // 从第24个字符开始匹配数字字段

    // 使用正则表达式匹配每个字段
    QRegularExpression numericRegex(R"([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?)");
    QRegularExpressionMatchIterator numericMatches = numericRegex.globalMatch(line);

    QVector<double> numericFields;
    int expectedFieldsPerLine = 4; // 每行应该有4个数字字段
    int totalExpectedFields = 29;  // 每个数据块应该有29个数字字段

    // 读取第一行的数字字段
    while (numericMatches.hasNext()) {
        QRegularExpressionMatch match = numericMatches.next();
        bool ok;
        double value = match.captured().toDouble(&ok);
        if (ok) {
            numericFields.append(value);
        }
    }

    // 读取接下来的6行，每行应该有4个字段
    for (int i = 0; i < 6; i++) {
        if (in.atEnd()) {
            return false; // 如果文件结束，返回 false
        }

        line = in.readLine();
        if (line.isNull()) {
            return false; // 如果某一行为空，返回 false
        }

        numericMatches = numericRegex.globalMatch(line);
        while (numericMatches.hasNext()) {
            QRegularExpressionMatch match = numericMatches.next();
            bool ok;
            double value = match.captured().toDouble(&ok);
            if (ok) {
                numericFields.append(value);
            }
        }

        // 检查当前行的字段数量是否正确
        int currentLineFields = numericFields.size() - i * expectedFieldsPerLine;
        if (currentLineFields < expectedFieldsPerLine) {
            // 如果当前行字段数量不足，补齐为0
            for (int j = 0; j < expectedFieldsPerLine - currentLineFields; j++) {
                numericFields.append(0.0);
            }
            qDebug() << "警告：数据块不完整，索引: " << indexField << " 某行字段不足";
        }
    }

    // 读取最后一行，应该有2个字段
    if (in.atEnd()) {
        return false; // 如果文件结束，返回 false
    }

    line = in.readLine();
    if (line.isNull()) {
        return false; // 如果某一行为空，返回 false
    }

    numericMatches = numericRegex.globalMatch(line);
    while (numericMatches.hasNext()) {
        QRegularExpressionMatch match = numericMatches.next();
        bool ok;
        double value = match.captured().toDouble(&ok);
        if (ok) {
            numericFields.append(value);
        }
    }

    // 检查最后一行的字段数量是否为2
    int lastLineFields = numericFields.size() - 6 * expectedFieldsPerLine - 3;
    if (lastLineFields < 2) {
        // 如果最后一行字段数量不足，补齐为0
        for (int j = 0; j < 2 - lastLineFields; j++) {
            numericFields.append(0.0);
        }
        qDebug() << "警告：最后一行字段数量不足" << indexField;
    }

    // 检查总字段数量是否正确
    if (numericFields.size() < totalExpectedFields) {
        qDebug() << "警告：总字段数量不足" << indexField;
    }

    stringDataBlocks.append({indexField});
    numericDataBlocks.append(numericFields);

    // 更新类别计数
    if (indexField.startsWith('C')) {
        cCount++;
    } else if (indexField.startsWith('G')) {
        gCount++;
    }

    return true;
}


void MainWindow::setgbFile(){
    QString filePath = QFileDialog::getOpenFileName(this, "选择文件", "", "All Files (*)");
    if (filePath.isEmpty()) {
        return;
    }
    ui->guangbofilePathLineEdit->setText(filePath);
    readgbFile();
}

void MainWindow::setjmFile(){
    QString filePath = QFileDialog::getOpenFileName(this, "选择文件", "", "All Files (*)");
    if (filePath.isEmpty()) {
        return;
    }
    ui->jingmifilePathLineEdit->setText(filePath);
    readjmFile();
}

void MainWindow::readgbFile() {
    // 读取文件内容
    QFile file(ui->guangbofilePathLineEdit->text());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "警告", "无法打开广播星历文件");
        return;
    }

    stringDataBlocks.clear(); // 清空之前存储的字符串数据块
    numericDataBlocks.clear(); // 清空之前存储的数字数据块

    int cCount = 0;
    int gCount = 0;

    // 跳过表头部分
    QTextStream in(&file);
    bool headerSkipped = false;

    while (!in.atEnd()) { // 逐行读取文件内容
        if (!headerSkipped) {
            QString line = in.readLine();
            // 跳过表头部分，直到遇到 "END OF HEADER" 行
            if (line.trimmed() == "END OF HEADER") {
                headerSkipped = true;
                continue; // 跳过 "END OF HEADER" 行本身
            }
            continue; // 继续读取下一行
        }

        // 解析数据块
        if (!parseBlock(in, stringDataBlocks, numericDataBlocks, cCount, gCount)) {
            QMessageBox::critical(this, "警告", "解析文件时发生错误");
            file.close();
            return;
        }
    }

    file.close();

    QString message = QString("广播星历文件解析成功，共解析出 %1 组数据\n其中GPS数据 %2 条，BDS数据 %3 条")
                          .arg(stringDataBlocks.size())
                          .arg(gCount)
                          .arg(cCount);

    QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString timestampedFine = "- " + currentTime + " -\n" + message;
    m_listWidget->addItem(timestampedFine);
    m_listWidget->show();
    m_listWidget->scrollToBottom();
}

void MainWindow::readjmFile() {
    QString jmfilePath = ui->jingmifilePathLineEdit->text();
    QFile file(jmfilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "警告", "无法打开精密星历文件");
        return;
    }

    QTextStream in(&file);
    QString currentTimestamp;
    QMap<QString, QVector<double>> currentSatelliteData;
    preciseEphemerisData.clear();

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        if (line.startsWith("*")) {
            // 处理时间信息行
            if (!currentTimestamp.isEmpty()) {
                // 存储上一个时间点的数据
                preciseEphemerisData[currentTimestamp] = currentSatelliteData;
                currentSatelliteData.clear();
            }

            currentTimestamp = line.mid(2).trimmed();
        } else {
            // 处理卫星数据行
            if (!currentTimestamp.isEmpty()) {
                QString satelliteID = line.mid(1, 3).trimmed();
                QStringList fields = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
                if (fields.size() >= 4) {
                    QVector<double> coordinates;
                    for (int i = 1; i <= 3; ++i) {
                        bool ok;
                        double value = fields[i].toDouble(&ok);
                        if (ok) {
                            coordinates.append(value);
                        } else {
                            QMessageBox::warning(this, "警告", QString("无效的坐标值: %1").arg(fields[i]));
                        }
                    }
                    currentSatelliteData[satelliteID] = coordinates;
                }
            }
        }
    }

    // 存储最后一个时间点的数据
    if (!currentTimestamp.isEmpty()) {
        preciseEphemerisData[currentTimestamp] = currentSatelliteData;
    }

    file.close();

    //QMessageBox::information(this, "精密星历", "精密星历文件解析完成");

    QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString fine;
    QString gbfilePath = ui->guangbofilePathLineEdit->text();
    int jmlength = jmfilePath.length();
    int gblength = gbfilePath.length();
    if (jmfilePath.mid(jmlength-23, 3) != gbfilePath.mid(gblength-18, 3)) {fine = "精密星历的年积日与广播星历不一致";}
    else if (jmfilePath.mid(jmlength-23, 3) == gbfilePath.mid(gblength-18, 3)) {fine = "精密星历文件解析完成";}
    else {fine = "精密星历文件名不是默认格式\n例如：GBM0MGXRAP_20242530000_01D_05M_ORB.SP3";}

    m_listWidget->addItem("- " + currentTime + " -\n" + fine);
    m_listWidget->show();
    m_listWidget->scrollToBottom();
}

QString MainWindow::formatPreciseTime(const QString &fullTime) {
    QStringList parts = fullTime.split(' ');
    for (int i = 0; i < 6; i++) {
        bool ok;
        int value = parts[i].toInt(&ok);
        if (ok) {
            if (i == 5) {  // 处理秒数部分，确保有8位小数
                parts[i] = QString::number(value, 'f', 8);
            } else {
                if (value < 10) {
                    parts[i] = " " + QString::number(value);
                }
            }
        }
    }
    return QString("%1 %2 %3 %4 %5  %6").arg(parts[0], parts[1], parts[2], parts[3], parts[4], parts[5]);
}

bool isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}
QVector<int> getDaysInMonth(int year) {
    QVector<int> daysInMonth = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (isLeapYear(year)) {
        daysInMonth[1] = 29;
    }
    return daysInMonth;
}
QString dateFromDayOfYear(int year, int dayOfYear) {
    QVector<int> daysInMonth = getDaysInMonth(year);
    int month = 1;
    int day = dayOfYear;

    for (int i = 0; i < 12; ++i) {
        if (day <= daysInMonth[i]) {
            month = i + 1;
            break;
        }
        day -= daysInMonth[i];
    }

    char buffer[9];
    sprintf(buffer, "%04d%02d%02d", year, month, day);
    return QString(buffer);
}

void MainWindow::calculateData() {
    readgbFile();
    readjmFile();

    // 提示窗口
    QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString fine;

    // 获取文件路径
    QString gbPath = ui->guangbofilePathLineEdit->text(), jmPath = ui->jingmifilePathLineEdit->text();
    if (gbPath.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择广播星历文件");
        return;
    }

    // 获取类别
    QString categoryStr = ui->categoryComboBox->currentText();
    QString category;
    if (categoryStr == "GPS") {
        category = 'G';
    } else if (categoryStr == "BDS") {
        category = 'C';
    } else {
        QMessageBox::warning(this, "警告", "请选择导航系统（例如GPS或BDS）");
        return;
    }
    qDebug() << "Input category:" << category;

    // 获取日期和时间
    QString date = ui->dateLineEdit->text(), time = ui->timeLineEdit->text(), PRN = ui->IDLineEdit->text();
    bool timeEmpty = time.isEmpty();
    bool prnEmpty = PRN.isEmpty();

    if (date.length() != 8) {
        QMessageBox::warning(this, "警告", "请输入有效的日期格式（例如20240909"); return;
    }

    qDebug() << "Input date:" << date;
    qDebug() << "Input time:" << time;
    qDebug() << "Input PRN:" << PRN;

    // 组合索引字符串
    QString formattedDate = date.insert(4, " ").insert(7, " ");
    QString formattedTime = time.insert(2, " ").insert(5, " ");
    QString fullTime = formattedDate + " " + formattedTime;
    QString formattedPRN = category + PRN;

    // 初始化一个字典来存储每个PRN的最近数据块索引
    QMap<QString, int> closestIndices;
    QVector<int> allIndices;
    int categorynum = 0;
    int unmatchednum = 0;
    QString unmatchedtime;
    closestIndices.clear();
    allIndices.clear();

    // 查找匹配的数据块
    for (int i = 0; i < stringDataBlocks.size(); ++i) {
        const QVector<QString> &block = stringDataBlocks[i];
        if (block[0].startsWith(category)) {
            categorynum += 1;
            QString blockPRN = block[0].left(3);
            if (block[0].mid(4,10) != formattedDate) {
                unmatchednum += 1;
                unmatchedtime = block[0].mid(4,10);
            }
            if (prnEmpty || blockPRN == formattedPRN) {
                if (timeEmpty) {
                    allIndices.append(i);
                    closestIndices[blockPRN] = i;
                } else {
                    QTime blockTime(block[0].mid(15, 2).toInt(), block[0].mid(18, 2).toInt(), block[0].mid(21, 2).toInt());
                    QTime inputTime(time.mid(0, 2).toInt(), time.mid(2, 2).toInt(), time.mid(4, 2).toInt());
                    int diff = abs(inputTime.secsTo(blockTime));
                    if (!closestIndices.contains(blockPRN) || diff < abs(inputTime.secsTo(
                        QTime::fromString(stringDataBlocks[closestIndices[blockPRN]][0].mid(15, 8), "hh mm ss")))) {
                        closestIndices[blockPRN] = i;
                    }
                }
            }
        }
    }

    // 当输入日期与该卫星系统超半数的数据块日期不匹配时，怀疑输入日期错误
    if (unmatchednum > categorynum/2) {
        QString gbfilePath = ui->guangbofilePathLineEdit->text();
        int gblength = gbfilePath.length();
        fine = "【警告】输入的日期 " + ui->dateLineEdit->text() + " 与广播星历内的日期不符\n您是否要输入 "
               + dateFromDayOfYear(gbfilePath.mid(gblength-22, 4).toInt(), gbfilePath.mid(gblength-18, 3).toInt()) + " ?\n";
    }

    if (closestIndices.isEmpty() && allIndices.isEmpty()) {
        if (timeEmpty){
            QMessageBox::warning(this, "警告", QString("找不到卫星 %1 的数据").arg(formattedPRN));
            fine = "【警告】广播星历中找不到卫星" + formattedPRN + "的数据\n";
        } else if (prnEmpty) {
            QMessageBox::warning(this, "警告", QString("找不到 %1 时刻的数据").arg(fullTime));
            fine = "【警告】广播星历中找不到" + fullTime + "时刻的数据\n";
        }
        return;
    }

    // 根据不同输入情况选择匹配的数据块
    QVector<int> matchedIndices;
    QVector<QString> matchedIDs;
    QVector<QString> matchedTimes;
    matchedIndices.clear();
    matchedIDs.clear();
    matchedTimes.clear();
    if (timeEmpty && prnEmpty) {
        for (int index : allIndices) {
            matchedIndices.append(index);
            matchedIDs.append(stringDataBlocks[index][0].left(3));
            matchedTimes.append(stringDataBlocks[index][0].mid(4, 19));
        }
    } else if (timeEmpty && !prnEmpty) {
        for (int index : allIndices) {
            matchedIndices.append(index);
            matchedIDs.append(stringDataBlocks[index][0].left(3));
            matchedTimes.append(stringDataBlocks[index][0].mid(4, 19));
        }
    } else if (!timeEmpty && prnEmpty) {
        for (auto it = closestIndices.begin(); it != closestIndices.end(); ++it) {
            int index = it.value();
            matchedIndices.append(index);
            matchedIDs.append(stringDataBlocks[index][0].left(3));
            matchedTimes.append(fullTime);
        }
    } else {
        int index = closestIndices[formattedPRN];
        matchedIndices.append(index);
        matchedIDs.append(stringDataBlocks[index][0].left(3));
        matchedTimes.append(fullTime);
    }

    // 提取并处理匹配的数据块
    QVector<QVector<double>> matchedNumericBlocks;
    for (int index : matchedIndices) {
        const QVector<double> &numericBlock = numericDataBlocks[index];
        matchedNumericBlocks.append(numericBlock);
    }

    // 创建Calculate实例和QStandardItemModel
    Calculate calculator;
    QVector<double> result;
    QVector<QVector<QString>> XYZ;
    QStandardItemModel *model = new QStandardItemModel(3 * matchedIndices.size(), 5, this);

    // 设置表头
    model->setHorizontalHeaderItem(0, new QStandardItem("PRN/时刻"));
    model->setHorizontalHeaderItem(1, new QStandardItem("坐标\n维度"));
    model->setHorizontalHeaderItem(2, new QStandardItem("广播星历\n计算结果"));
    model->setHorizontalHeaderItem(3, new QStandardItem("精密星历\n计算结果"));
    model->setHorizontalHeaderItem(4, new QStandardItem("差值\n(m)"));

    // 【计算并展示每个卫星的数据】
    int row = 0;
    for (int i = 0; i < matchedNumericBlocks.size(); ++i) {
        const auto &numericBlock = matchedNumericBlocks[i];
        QString satelliteID = matchedIDs[i];
        QStringList Timeparts = matchedTimes[i].split(' ');
        QString satelliteDate = Timeparts[0] + "年" + Timeparts[1] + "月" + Timeparts[2] + "日";
        QString satelliteTime = Timeparts[3] + ":" + Timeparts[4] + ":" + Timeparts[5];
        result = calculator.processDataBlock(numericBlock, matchedTimes[i], satelliteID, 0, 0, 0);

        QVector<QString> satelliteData;
        satelliteData.append(satelliteID);
        satelliteData.append(QString::number(result[0]/1e7, 'f', 3));
        satelliteData.append(QString::number(result[1]/1e7, 'f', 3));
        satelliteData.append(QString::number(result[2]/1e7, 'f', 3));
        XYZ.append(satelliteData);

        // 卫星编号
        model->setItem(row, 0, new QStandardItem(satelliteID));
        model->setItem(row + 1, 0, new QStandardItem(satelliteTime));

        // 位置维度
        model->setItem(row, 1, new QStandardItem("X"));
        model->setItem(row + 1, 1, new QStandardItem("Y"));
        model->setItem(row + 2, 1, new QStandardItem("Z"));

        // 广播星历
        model->setItem(row, 2, new QStandardItem(QString::number(result[0], 'f', 3)));
        model->setItem(row + 1, 2, new QStandardItem(QString::number(result[1], 'f', 3)));
        model->setItem(row + 2, 2, new QStandardItem(QString::number(result[2], 'f', 3)));

        // 精密星历
        QString preciseTime = formatPreciseTime(matchedTimes[i]);
        if (preciseEphemerisData.contains(preciseTime) && preciseEphemerisData[preciseTime].contains(satelliteID)) {
            QVector<double> preciseXYZ = preciseEphemerisData[preciseTime][satelliteID];
            model->setItem(row, 3, new QStandardItem(QString::number(preciseXYZ[0]*1000, 'f', 3)));
            model->setItem(row + 1, 3, new QStandardItem(QString::number(preciseXYZ[1]*1000, 'f', 3)));
            model->setItem(row + 2, 3, new QStandardItem(QString::number(preciseXYZ[2]*1000, 'f', 3)));

            model->setItem(row, 4, new QStandardItem(QString::number(result[0] - preciseXYZ[0]*1000, 'f', 3)));
            model->setItem(row + 1, 4, new QStandardItem(QString::number(result[1] - preciseXYZ[1]*1000, 'f', 3)));
            model->setItem(row + 2, 4, new QStandardItem(QString::number(result[2] - preciseXYZ[2]*1000, 'f', 3)));
        } else {
            // 查找最接近的时间点
            QList<QPair<QString, QTime>> closestTimes = findClosestTimes(fullTime, preciseEphemerisData, 5);

            // 提取这些时间点的卫星坐标
            QMap<QString, QVector<QVector<double>>> closestCoordinates = extractClosestCoordinates(closestTimes, preciseEphemerisData);

            if (closestCoordinates.contains(satelliteID)) {
                QPair<QVector<double>, bool> interpolationResult = lagrangeInterpolation(closestTimes, closestCoordinates[satelliteID], fullTime);
                QVector<double> preciseXYZ = interpolationResult.first;
                bool isValid = interpolationResult.second;

                if (isValid) {
                    model->setItem(row, 3, new QStandardItem(QString::number(preciseXYZ[0]*1000, 'f', 3)));
                    model->setItem(row + 1, 3, new QStandardItem(QString::number(preciseXYZ[1]*1000, 'f', 3)));
                    model->setItem(row + 2, 3, new QStandardItem(QString::number(preciseXYZ[2]*1000, 'f', 3)));

                    model->setItem(row, 4, new QStandardItem(QString::number(result[0] - preciseXYZ[0]*1000, 'f', 3)));
                    model->setItem(row + 1, 4, new QStandardItem(QString::number(result[1] - preciseXYZ[1]*1000, 'f', 3)));
                    model->setItem(row + 2, 4, new QStandardItem(QString::number(result[2] - preciseXYZ[2]*1000, 'f', 3)));
                } else {
                    model->setItem(row, 3, new QStandardItem("N/A"));
                    model->setItem(row + 1, 3, new QStandardItem("N/A"));
                    model->setItem(row + 2, 3, new QStandardItem("N/A"));

                    model->setItem(row, 4, new QStandardItem("N/A"));
                    model->setItem(row + 1, 4, new QStandardItem("N/A"));
                    model->setItem(row + 2, 4, new QStandardItem("N/A"));
                }
            } else {
                qDebug() << "在精密星历文件中，没有卫星" << satelliteID << "在" << matchedTimes[i] << "时刻的数据";
                if (!prnEmpty){
                    fine += "在精密星历文件中，没有卫星" + satelliteID + "在" + satelliteDate + satelliteTime + "时刻的数据\n";
                }
                model->setItem(row, 3, new QStandardItem("N/A"));
                model->setItem(row + 1, 3, new QStandardItem("N/A"));
                model->setItem(row + 2, 3, new QStandardItem("N/A"));

                model->setItem(row, 4, new QStandardItem("N/A"));
                model->setItem(row + 1, 4, new QStandardItem("N/A"));
                model->setItem(row + 2, 4, new QStandardItem("N/A"));
            }
        }
        row += 3;
    }

    fine += "卫星星历计算结果已生成";
    m_listWidget->addItem("- " + currentTime + " -\n" + fine);
    m_customOpenGLWidget->setXYZData(XYZ);

    // 设置模型
    m_listWidget->show();
    m_listWidget->scrollToBottom();
    ui->tableView->setModel(model);
    ui->tableView->resizeColumnsToContents();

    stringDataBlocks.clear();
    result.clear();
}


QList<QPair<QString, QTime>> MainWindow::findClosestTimes(const QString &inputTime, const QMap<QString, QMap<QString, QVector<double>>> &preciseEphemerisData, int numClosest) {
    QList<QPair<QString, QTime>> times;
    for (const auto &timestamp : preciseEphemerisData.keys()) {
        // 提取时间部分 "hh mm ss"
        QString timePart = timestamp.mid(11, 8);
        QString hour = timePart.mid(0, 2).trimmed().rightJustified(2, '0');
        QString minute = timePart.mid(3, 2).trimmed().rightJustified(2, '0');
        QString second = timePart.mid(6, 2).trimmed().rightJustified(2, '0');
        QString formattedTimePart = hour + " " + minute + " " + second;

        QTime time = QTime::fromString(formattedTimePart, "hh mm ss");
        if (time.isValid()) {
            times.append({timestamp, time});
        } else {
            qDebug() << "Invalid time format for timestamp:" << timestamp;
        }
    }

    std::sort(times.begin(), times.end(), [inputTime](const QPair<QString, QTime> &a, const QPair<QString, QTime> &b) {
        // 提取输入时间的时间部分 "hh mm ss"
        QString inputTimePart = inputTime.mid(11, 8);
        QString hour = inputTimePart.mid(0, 2).trimmed().rightJustified(2, '0');
        QString minute = inputTimePart.mid(3, 2).trimmed().rightJustified(2, '0');
        QString second = inputTimePart.mid(6, 2).trimmed().rightJustified(2, '0');
        QString formattedInputTimePart = hour + " " + minute + " " + second;

        QTime input = QTime::fromString(formattedInputTimePart, "hh mm ss");
        if (!input.isValid()) {
            qDebug() << "Invalid input time format:" << inputTime;
            return false;
        }
        return abs(input.secsTo(a.second)) < abs(input.secsTo(b.second));
    });

    return times.mid(0, numClosest);
}

QMap<QString, QVector<QVector<double>>> MainWindow::extractClosestCoordinates(const QList<QPair<QString, QTime>> &closestTimes, const QMap<QString, QMap<QString, QVector<double>>> &preciseEphemerisData) {
    QMap<QString, QVector<QVector<double>>> closestCoordinates;
    for (const auto &timePair : closestTimes) {
        const QString &timestamp = timePair.first;
        for (const auto &satelliteID : preciseEphemerisData[timestamp].keys()) {
            if (!closestCoordinates.contains(satelliteID)) {
                closestCoordinates[satelliteID] = QVector<QVector<double>>();
            }
            closestCoordinates[satelliteID].append(preciseEphemerisData[timestamp][satelliteID]);
        }
    }
    return closestCoordinates;
}

QPair<QVector<double>, bool> MainWindow::lagrangeInterpolation(const QList<QPair<QString, QTime>> &times, const QVector<QVector<double>> &coordinates, const QString &targetTime) {
    // 提取时间部分 "hh mm ss"
    QString timePart = targetTime.mid(11, 8);
    QString hour = timePart.mid(0, 2).trimmed().rightJustified(2, '0');
    QString minute = timePart.mid(3, 2).trimmed().rightJustified(2, '0');
    QString second = timePart.mid(6, 2).trimmed().rightJustified(2, '0');
    QString formattedTimePart = hour + " " + minute + " " + second;
    QTime target = QTime::fromString(formattedTimePart, "hh mm ss");

    if (!target.isValid()) {
        qDebug() << "Invalid target time format:" << formattedTimePart;
        return {QVector<double>(3, 0.0), false};
    }

    int n = times.size();
    QVector<double> result(3, 0.0);

    for (int i = 0; i < n; ++i) {
        double L = 1.0;
        for (int j = 0; j < n; ++j) {
            if (i != j) {
                int timeDiff = times[i].second.secsTo(times[j].second);
                if (timeDiff == 0) {
                    // 如果时间差为零，跳过当前时间点
                    L = 0.0;
                    break;
                }
                L *= static_cast<double>(target.secsTo(times[j].second)) / timeDiff;
            }
        }
        for (int k = 0; k < 3; ++k) {
            result[k] += L * coordinates[i][k];
        }
    }

    // 检查结果是否为 (0, 0, 0)
    bool isValid = !(result[0] == 0.0 && result[1] == 0.0 && result[2] == 0.0);
    return {result, isValid};
}
