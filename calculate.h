#ifndef CALCULATE_H
#define CALCULATE_H

#include <QVector>
#include <cmath>
#include <Eigen/Dense>

class Calculate
{
public:
    QVector<double> processDataBlock(const QVector<double> &block, const QString &fullTime, const QString &satelliteID,
                                     const double Xk0, const double Yk0, const double Zk0);
    void satelliteCoordinate(const double &sqrt_a, const double &delta_n, const double &t_oe, const double &M0, const double &e, const double &omega,
                             const double &Cuc, const double &Cus, const double &Crc, const double &Crs, const double &Cic, const double &Cis,
                             const double &i0, const double &i_dot, const double &Omega0, const double &Omega_dot, const int &t,
                             double &Xk, double &Yk, double &Zk, const QString &satelliteID);
    void stationCoordinate(const double &a0, const double &a1, const double &a2, const double &t, const double &t_oe, const double Ikp,
                           const double &Xp, const double &Yp, const double &Zp, double &lkp, double &mkp, double &nkp, double &wkp);
    int SecondsInWeek(const QString &fullTime, const QString &category);

};

#endif
