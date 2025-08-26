#include "calculate.h"
#include <QDateTime>
#include <QDate>
#include <QTime>
#include <QString>
#include <qDebug>

using namespace std;

QVector<double> Calculate::processDataBlock(const QVector<double> &block, const QString &fullTime, const QString &satelliteID,
                                            const double Xk, const double Yk, const double Zk)
{
    if (block.size() < 29) {
        qWarning("Data block size is less than 29 (Error in processDataBlock)");
        return QVector<double>();
    }

    // 将29个数值分别赋值给对应的变量
    double a0 = block[0];   // 卫星钟差(s)
    double a1 = block[1];   // 卫星钟速(s/s)
    double a2 = block[2];   // 卫星钟漂(s/s2)
    double AODE = block[3]; // 星历数据龄期
    double Crs = block[4];  // 卫星矢径的正弦调和项改正振幅(m)
    double delta_n = block[5];  // 平均角速度之差(rad)
    double M0 = block[6];       // 按参考历元计算的平近点角(rad)
    double Cuc = block[7];      // 升交距角的余弦调和项改正振幅(rad)
    double e = block[8];        // 轨道第一偏心率
    double Cus = block[9];      // 升交距角的正弦调和项改正振幅(rad)
    double sqrt_a = block[10];  // 轨道长半径平方根(/sqrt m)
    double t_oe = block[11];    // 星历表参考历元(s)
    double Cic = block[12];     // 轨道倾角的余弦调和项改正振幅(rad)
    double Omega0 = block[13];  // 按参考历元计算的升交点赤经(rad)
    double Cis = block[14];     // 轨道倾角的正弦调和项改正振幅(rad)
    double i0 = block[15];      // 按参考历元计算的轨道倾角(rad)
    double Crc = block[16];     // 卫星矢径的余弦调和项改正振幅(m)
    double omega = block[17];   // 近地点角距(rad)
    double Omega_dot = block[18];   // 升交点赤经变化率(rad/s)
    double i_dot = block[19];       // 轨道倾角变化率(rad/s)
    double L2 = block[20];          // L2上的码
    double GPSweek = block[21];     // GPS周
    double L2_P = block[22];        // L2 P码数据标记
    double accuracy = block[23];    // 卫星轨道精度(m)
    double healthy = block[24];     // 卫星健康状态
    double T_gd = block[25];    // 电离层延迟改正(s)
    double IODC = block[26];    // 星钟数据龄期
    double ttm = block[27];     // 电文传输时间(GPS周内秒)
    double f_i = block[28];     // 拟合区间(h)，如未知则为0

    QString category = satelliteID[0];
    int t = SecondsInWeek(fullTime, category);
    double Xp=0, Yp=0, Zp=0;
    satelliteCoordinate(sqrt_a, delta_n, t_oe, M0, e, omega, Cuc, Cus, Crc, Crs, Cic, Cis, i0, i_dot, Omega0, Omega_dot, t, Xp, Yp, Zp, satelliteID);
    double lkp=0, mkp=0, nkp=0, wkp=0;
    double c = 3e8;
    double dt = a0 + a1 * (t - t_oe) + a2 * pow(t - t_oe, 2);
    double roukp = sqrt(pow(Xp-Xk,2) + pow(Yp-Yk,2) + pow(Zp-Zk,2));
    lkp = (Xk-Xp)/roukp, mkp = (Yk-Yp)/roukp, nkp = (Zk-Zp)/roukp;
    wkp = -T_gd + c*dt;
    return {Xp, Yp, Zp, lkp, mkp, nkp, wkp};
}


void Calculate::satelliteCoordinate(const double &sqrt_a, const double &delta_n, const double &t_oe, const double &M0, const double &e, const double &omega,
                    const double &Cuc, const double &Cus, const double &Crc, const double &Crs, const double &Cic, const double &Cis,
                    const double &i0, const double &i_dot, const double &Omega0, const double &Omega_dot, const int &t,
                    double &Xk, double &Yk, double &Zk, const QString &satelliteID){
    double miu = 3.986004418e14; // G*m_地(m^3/s^2)
    double omegae = 7.29211515e-5;
    double tk = 0;
    QString category = satelliteID[0];

    if (category == 'G'){
        miu = 3.986005e14;
        omegae = 7.29211515e-5;
        tk = t - t_oe;
    }
    else if (category == 'C'){
        miu = 3.986004418e14;
        omegae = 7.292115e-5;
        tk = t - 14 - t_oe; // BDST 减去14秒
    }
    double omegaline0 = sqrt(miu) / pow(sqrt_a, 3);
    double omegaline = omegaline0 + delta_n;

    if (tk > 302400) tk = tk - 604800;
    else if (tk < -302400) tk = tk + 604800;

    double Mk = M0 + omegaline * tk;
    double Ek = Mk;
    for (int i = 0; i < 3; i++) {
        Ek = Mk + e * sin(Ek);
    }

    double Vk = atan2(sqrt(1 - pow(e, 2)) * sin(Ek) , (cos(Ek) - e));
    double Faik = Vk + omega;

    double du = Cuc * cos(2 * Faik) + Cus * sin(2 * Faik);
    double dr = Crc * cos(2 * Faik) + Crs * sin(2 * Faik);
    double di = Cic * cos(2 * Faik) + Cis * sin(2 * Faik);

    double uk = Faik + du;
    double rk = pow(sqrt_a, 2) * (1 - e * cos(Ek)) + dr;
    double ik = i0 + di + i_dot * tk;

    double xk = rk * cos(uk);
    double yk = rk * sin(uk);
    // double zk = 0;

    double Lk;
    QString IDnumber = satelliteID.mid(1, 2);
    int number = IDnumber.toInt();

    if (category == 'C' && (number <= 5 || number >= 59)){  // BDS GEO计算
        Lk = Omega0 + Omega_dot * tk - omegae * t_oe;
        double Xk0 = xk * cos(Lk) - yk * cos(ik) * sin(Lk);
        double Yk0 = xk * sin(Lk) + yk * cos(ik) * cos(Lk);
        double Zk0 = yk * sin(ik);
        double l = omegae * tk;
        Eigen::MatrixXd XYZk(3, 3);
        Eigen::MatrixXd XYZ0(3, 1);
        XYZk << cos(l), sin(l)*cos(-M_PI/36), sin(l)*sin(-M_PI/36),
                -sin(l), cos(l)*cos(-M_PI/36), cos(l)*sin(-M_PI/36),
                0, -sin(-M_PI/36), cos(-M_PI/36);
        XYZ0 << Xk0, Yk0, Zk0;
        Eigen::MatrixXd result = XYZk * XYZ0;
        Xk = result(0);
        Yk = result(1);
        Zk = result(2);
    }
    else {
        Lk = Omega0 + (Omega_dot - omegae) * tk - omegae * t_oe;
        Xk = xk * cos(Lk) - yk * cos(ik) * sin(Lk);
        Yk = xk * sin(Lk) + yk * cos(ik) * cos(Lk);
        Zk = yk * sin(ik);
    }
}


int Calculate::SecondsInWeek(const QString &fullTime, const QString &category) {
    QStringList parts = fullTime.split(' ');
    if (parts.size() != 6) {
        qWarning("Invalid fullTime format");
        return -1;
    }
    int year = parts[0].toInt();
    int month = parts[1].toInt();
    int day = parts[2].toInt();
    int hour = parts[3].toInt();
    int minute = parts[4].toInt();
    int second = parts[5].toInt();

    // 创建QDateTime对象
    QDateTime inputDateTime = QDateTime(QDate(year, month, day), QTime(hour, minute, second));
    QDateTime Start;

    // 定义起算起点
    if (category == 'G'){
        Start = QDateTime(QDate(1980, 1, 6), QTime(0, 0, 0));
    }
    else if (category == 'C'){
        Start = QDateTime(QDate(2006, 1, 1), QTime(0, 0, 0));
    }

    // 计算两个日期之间的总秒数
    qint64 totalSeconds = Start.secsTo(inputDateTime);

    // 计算输入时间点在其所在周中的秒数
    int secondsInWeek = totalSeconds % 604800;
    return secondsInWeek;
}

