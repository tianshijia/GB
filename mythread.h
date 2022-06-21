#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QObject>
#include <QList>
#include <complex>
#define PI 3.14159265358979
class mythread : public QObject
{
    Q_OBJECT
public:
    //explicit mythread(QObject *parent = nullptr);
    QString time_str;//shijian
    mythread();
    ~mythread();
signals:
    void send_gsv_data(QList<int>gpgsv,QList<int>bdgsv);
    void send_gga_gsa_data(QStringList gngga,QStringList gpgsa,QStringList bdgsa,QStringList gnvtg);
    void send_gga_time(QStringList gngga);
    void send_X_Y_Z(double X,double Y,double Z);

public slots:
    void gsv_jiexi(QStringList gpgsv_data,QStringList bdgsv_data);
    void gga_gsa_jiexi(QString gngga_data,QString gpgsa_data,QString bdgsa_data,QString gnvtg_data);
    void gga_time_jiexi(QString gngga_data);
    void lat_lon_height_jiexi(QString lat,QString lon,QString height);

};




#endif // MYTHREAD_H
