#include "mythread.h"
#include<QThread>
#include <QDebug>
extern  bool file_flag;
mythread::mythread()
{
    //qDebug()<<"mythread构造函数ID:"<<QThread::currentThreadId();
}

void mythread::gsv_jiexi(QStringList gpgsv_data, QStringList bdgsv_data)
{
    //qDebug()<<gpgsv_data;
    QList<int>gpgsv;
    QList<int>bdgsv;
   if(!gpgsv_data.isEmpty()){
       for(int i=0;i<gpgsv_data.length();i++){
           QString gpgsvdata = gpgsv_data[i];
           gpgsvdata = gpgsvdata.left(gpgsvdata.length()-5);//去除校验和

           QStringList datalist1 = gpgsvdata.split(',');

           //qDebug()<<datalist1; // 以上代码分割成这个样子
           /*
              ("GPGSV", "3", "1", "12", "10", "80", "233", "31", "12", "39", "090", "24", "15", "08", "098", "20", "21", "13", "311", "*7")
              ("GPGSV", "3", "2", "12", "22", "31", "288", "", "23", "52", "158", "37", "24", "33", "052", "22", "25", "36", "147", "32*7")
              ("GPGSV", "3", "3", "12", "31", "16", "230", "41", "32", "51", "304", "30", "194", "45", "132", "33", "199", "42", "164", "32*7")
            */
           if(datalist1.length()/4==5){
               for(int j=4;j<20;j++){
                   gpgsv.append(datalist1[j].toUInt());
                   //qDebug()<<datalist1[j].toUInt();  把从第一颗卫星的卫星号开始分割出来
               }
           }
           else if (datalist1.length()/4==4) {
               for(int j=4;j<16;j++){
                   gpgsv.append(datalist1[j].toUInt());
                   //qDebug()<<datalist1[j].toUInt();
               }
           }
           else if (datalist1.length()/4==3) {
               for(int j=4;j<12;j++){
                   gpgsv.append(datalist1[j].toUInt());
                   //qDebug()<<datalist1[j].toUInt();
               }
           }
           else if (datalist1.length()/4==2) {
               for(int j=4;j<8;j++){
                   if(datalist1[4].toUInt()>32){break;}
                   gpgsv.append(datalist1[j].toUInt());
                   //qDebug()<<datalist1[j].toUInt();
               }
           }

           else if (datalist1.length()/4==1) {
               for(int j=4;j<4;j++){
                   if(datalist1[4].toUInt()>32){break;}
                   gpgsv.append(datalist1[j].toUInt());
                   //qDebug()<<datalist1[j].toUInt();
               }
           }
       }
   }

   if(!bdgsv_data.isEmpty()){
       for(int i=0;i<bdgsv_data.length();i++){
           QString bdgsvdata = bdgsv_data[i];
           bdgsvdata = bdgsvdata.left(bdgsvdata.length()-5);//去除校验和

           QStringList datalist2 = bdgsvdata.split(',');

           if(datalist2.length()/4==5){
               for(int j=4;j<20;j++){
                   bdgsv.append(datalist2[j].toUInt());
               }
           }
           else if (datalist2.length()/4==4) {
               for(int j=4;j<16;j++){
                   bdgsv.append(datalist2[j].toUInt());
               }
           }
           else if (datalist2.length()/4==3) {
               for(int j=4;j<12;j++){
                   bdgsv.append(datalist2[j].toUInt());
               }
           }
           else if (datalist2.length()/4==2) {
               for(int j=4;j<8;j++){
                   if(datalist2[4].toUInt()>32){break;}
                   bdgsv.append(datalist2[j].toUInt());
               }

           }
       }
   }

   if(!bdgsv.isEmpty()&&!gpgsv.isEmpty()){
     //qDebug()<<"线程发出"<<gpgsv;
     //qDebug()<<"线程发出"<<bdgsv;
      emit send_gsv_data(gpgsv,bdgsv);

   }
}

void mythread::gga_gsa_jiexi(QString gngga_data, QString gpgsa_data, QString bdgsa_data, QString gnvtg_data)
{
    QStringList gpgsa;
    QStringList bdgsa;
    QStringList gngga;
    QStringList gnvtg;
    int count = 0;

    if(!gpgsa_data.isEmpty()){

        gpgsa_data = gpgsa_data.left(gpgsa_data.length()-5);
        QStringList gpgsa_data_list = gpgsa_data.split(',');
        //qDebug()<<"gpgsa_data_list:"<<gpgsa_data_list;
        //gpgsa.append(gpgsa_data_list[2]);//是否定位
        for (int i=0;i<12;i++) {
            if(!gpgsa_data_list[i+3].isEmpty()){
                count++;
                gpgsa.append(gpgsa_data_list[i+3]);
            }
        }
        gpgsa.append(QString::number(count));
        gpgsa.append(gpgsa_data_list[15]);
        //gpgsa.append(gpgsa_data_list[16]);
       // gpgsa.append(gpgsa_data_list[17]);
      // qDebug()<<gpgsa;

    }
    if(!bdgsa_data.isEmpty()){
        count = 0;
        bdgsa_data = bdgsa_data.left(bdgsa_data.length()-5);
        QStringList bdgsa_data_list = bdgsa_data.split(',');
        //qDebug()<<"bdgsa_data_list:"<<bdgsa_data_list;
         //bdgsa.append(bdgsa_data_list[2]);
        for (int i=0;i<12;i++) {
            if(!bdgsa_data_list[i+3].isEmpty()){
                count++;
                bdgsa.append(bdgsa_data_list[i+3]);
            }
        }
        bdgsa.append(QString::number(count));
        bdgsa.append(bdgsa_data_list[15]);
        //bdgsa.append(bdgsa_data_list[16]);
       // bdgsa.append(bdgsa_data_list[17]);
        //qDebug()<<bdgsa;
    }


    ////"GNGGA,095409.000,2518.79964,N,11024.23665,E,1,15,1.5,181.6,M,0.0,M,,*74\n"
    if(!gngga_data.isEmpty()){
        int lat_highbit;
        double lat_lowbit;
        int lon_highbit;
        double lon_lowbit;
        //QString utc_time;
        double lat; //纬度
        double lon; //经度
        double altitude;
        QStringList datalist = gngga_data.split(",");
        QStringList lat_list = datalist[2].split(".");  // start analysis lat
        if(lat_list.length()>1)
        {
             lat_highbit = (lat_list[0].toInt()/100);
             lat_lowbit = (lat_list[0].right(2).toDouble()+(lat_list[1].toDouble()/100000))/60;// end analysis lat
             lat = lat_highbit + lat_lowbit;

        }
        QStringList lon_list = datalist[4].split("."); //start analysis lon'
        if(lon_list.length()>1)
        {
            lon_highbit = lon_list[0].toInt()/100;
            lon_lowbit = (lon_list[0].right(2).toDouble()+(lon_list[1].toDouble()/100000))/60; //end analysis lon
            lon = lon_highbit + lon_lowbit;
        }
        lat = lat_highbit + lat_lowbit;
        lon = lon_highbit + lon_lowbit;
        altitude = datalist[9].toDouble(); // get altitude

        //gngga.append(utc_time);
        gngga.append(QString::number(lat,'f',9));
        gngga.append(datalist[3]);//lat_hemisphere
        gngga.append(QString::number(lon,'f',9));
        gngga.append(datalist[5]);//lon_hemisphere
        gngga.append(QString::number(altitude,'f',3));
    }
    if(!gnvtg_data.isEmpty()){
        if(file_flag == false){
            gnvtg_data = gnvtg_data.left(gnvtg_data.length()-5);
            QStringList gnvtg_data_list = gnvtg_data.split(',');
            gnvtg.append(gnvtg_data_list[1]);
            gnvtg.append(gnvtg_data_list[7]);
            gnvtg.append(gnvtg_data_list[9]);
           //qDebug()<<"线程解析gnvtg: "<<gnvtg;
        }
        else {
            gnvtg_data = gnvtg_data.left(gnvtg_data.length()-4);
            QStringList gnvtg_data_list = gnvtg_data.split(',');
            gnvtg.append(gnvtg_data_list[1]);
            gnvtg.append(gnvtg_data_list[7]);
            gnvtg.append(gnvtg_data_list[9]);
           //qDebug()<<"线程解析gnvtg: "<<gnvtg;
        }
    }

   if(!gnvtg.isEmpty()&& !gngga.isEmpty() && !bdgsa.isEmpty() && !gpgsa.isEmpty()){
    emit send_gga_gsa_data(gngga,gpgsa,bdgsa,gnvtg);
   }


}

void mythread::gga_time_jiexi(QString gngga_data)
{
    QStringList gngga;
    if(!gngga_data.isEmpty()){
        QString utc_time;
        QStringList datalist = gngga_data.split(",");

        QStringList time_list = datalist[1].split("."); //start analysis lon'
        if(time_list.length()>1){
            QString time_data =time_list[0];
            if(time_data.mid(0,2).toInt()>=16){
                int shi = time_data.mid(0,2).toInt()+8-24;
                time_str = QString::number(shi)+("时")+time_data.mid(2,2)+("分")+time_data.mid(4,2)+("秒");//东八区
            }else {
            time_str = QString::number(time_data.mid(0,2).toInt()+8)+("时")+time_data.mid(2,2)+("分")+time_data.mid(4,2)+("秒");//东八区
        }

        }
        utc_time = time_str;
        gngga.append(utc_time);

    }
    if(!gngga.isEmpty()){
        emit send_gga_time(gngga);
    }


}

void mythread::lat_lon_height_jiexi(QString lat, QString lon, QString height)
{
    if(!lat.isEmpty()&&!lon.isEmpty()&& !height.isEmpty()){
        double L = lon.toDouble();
        double B = lat.toDouble();//纬度
        double Height = height.toDouble();
        double a = 6378137.0;
        double e2 = 0.00669438002290;//转换为弧度
        L = L * PI/180;
        B = B * PI/180;

        double fac1 = 1-e2*sin(B)*sin(B);
        double N = a/sqrt(fac1); //卯酉圈曲率半径
        double Daita_h = 0;      //高程异常,默认为0
        double h = Daita_h + Height;

        double X = (N+h)*cos(B)*cos(L);
        double Y = (N+h)*cos(B)*sin(L);
        double Z = ( N*(1-e2)+ h ) * sin(B);
        emit send_X_Y_Z(X,Y,Z);
    }
}

mythread::~mythread()
{

}
