#ifndef BGTEST_H
#define BGTEST_H

#include "gsv.h"

#include <QMainWindow>
#include <QSerialPort>
#include<QtSerialPort/QSerialPortInfo>
#include <QMessageBox>
#include <QDebug>
#include <QTimer>
#include <QTextCodec>
#include <QAxObject>
#include <QDir>
#include "mythread.h"
#include <QThread>
#include <QMetaType>
#include <QDateTime>
#include <QPainter>
#include <QPaintEvent>
#include <QTextStream>
#include <QMessageBox>
namespace Ui {
class BGTEST;
}

class BGTEST : public QMainWindow
{
    Q_OBJECT

    class  gps_data
    {
    public:
        double lat; //纬度
        double lon; //经度
        QString GGA_data;
        QString GNVTG_data;
        QString GPSGSA_data;
        QString BDGSA_data;
        QStringList GPS_GSV_data;
        QStringList BD_GSV_data;
    };




public:
    explicit BGTEST(QWidget *parent = 0);
    ~BGTEST();
    void SearchSerialPortName();
    void readthings();

    //事件过滤器
     bool eventFilter(QObject *watched, QEvent *event);
     bool satellite_update_flag = false;//update flag

    QString postion;
    QString postion_lat;
    QString postion_lon;
    QString speed;
    QString course_angle;
    QString buffer;
    QString buff;
    int fd_gps;

    QDateTime current_time;
    QString pc_time;

    QTextCodec *codec;

    //gsv
    int getGSV(QByteArray date);

    QList<int> gpgsv_data_table_widget;
    QList<int> bdgsv_data_table_widget;

     void paint();

     //读取文件
     QFile f;
     unsigned int readfile_speed = 500;
     QTimer *readfieltimer;
     long int readfile_length = 0;
     int over_value = 100;
     void read_filedata();
     bool openfile_flag = false;
     bool savefile_flag = false;
     QString path;
     QTimer *timer_saveFile;//保存文件定时器
     void save_file();
     QString file_buffer;

     //两次定位偏差
     double now_lat = 40.065759333;
     double now_lon = 116.301878167;
     double location_error;
     QVector<double> sx_vec,xAxis_vec;
     int m_chartPoint_counter = 0;
     void paint_location_error(double lat,double lon);


signals: //数据发送信号
    void send_gsv_data(QStringList gpgsv_data,QStringList bdgsv_data);
    void send_gga_gsa_data(QString gngga_data,QString gpgsa_data,QString bdgsa_data,QString gnvtg_data);
    void send_gga_data(QString gngga_data);
    void send_lat_lon_height(QString lat,QString lon,QString height);
private slots:
    void on_open_com_clicked();

    void on_dingwei_clicked();

    void on_action_triggered();

    void on_action_2_triggered();

public slots:
    void recv_gsvdata(QList<int> gpgsv,QList<int> bdgsv);
    void recv_gga_gsa_data(QStringList gngga_data,QStringList gpgsa_data,QStringList bdgsa_data,QStringList gnvtg_data);
    void recv_gga_time(QStringList gngga_data);
    void recv_X_Y_Z(double X,double Y,double Z);
private:
    Ui::BGTEST *ui;
    QSerialPort my_serialPort;
    QString serialdata;
    QTimer * timer;
    gps_data mygpsdata;


    //线程对象
    mythread *mythread_gpgsv_analysis;
    QThread  *thread_gpgsv_analysis;
    mythread *mythread_gngga_gsa_analysis;
    QThread  *thread_gngga_gsa_analysis;

    mythread *mythread_time;
    QThread  *thread_time;

    mythread *mythread_WGS84;
    QThread  *thread_WGS84;



    QString lat;
    QString lon;
    QString height;
};



#endif // BGTEST_H
