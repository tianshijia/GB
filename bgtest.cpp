#include "bgtest.h"
#include "ui_bgtest.h"
bool file_flag = "";
BGTEST::BGTEST(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::BGTEST)
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon("satellite.ico"));
    SearchSerialPortName();
    ui->GPS_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->BD_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->BD_tableWidget->verticalHeader()->setHidden(true);
    ui->GPS_tableWidget->verticalHeader()->setHidden(true);
    ui->GPS_tableWidget->setRowCount(50);
    ui->BD_tableWidget->setRowCount(50);

    this->setPalette(QPalette(QColor("#cdc5bf")));
    //安装事件过滤器
    ui->widget->installEventFilter(this);

    //ui->receiveDate->document()->setMaximumBlockCount(40);  //设置显示最多为40行

    //为线程对象申请内存空间
    mythread_gpgsv_analysis = new mythread;
    thread_gpgsv_analysis   = new QThread;


    mythread_gngga_gsa_analysis = new mythread;
    thread_gngga_gsa_analysis = new QThread;

    mythread_time = new mythread;
    thread_time = new QThread;

    mythread_WGS84 = new mythread;
    thread_WGS84 = new QThread;

    //使用movetothread实现托管线程
    mythread_gpgsv_analysis->moveToThread(thread_gpgsv_analysis);
    mythread_gngga_gsa_analysis->moveToThread(thread_gngga_gsa_analysis);
    mythread_time->moveToThread(thread_time);
    mythread_WGS84->moveToThread(thread_WGS84);


    //线程启动管理
    thread_gpgsv_analysis->start();
    thread_gngga_gsa_analysis->start();
    thread_time->start();
    thread_WGS84->start();
    qRegisterMetaType<QList<int>>("QList<int>"); //在多线程连接信号槽,发送Qlist<int>& 类型时抛出错误: 这是解决办法
    //ui 到 线程
    connect(this,SIGNAL(send_gsv_data(QStringList,QStringList)),mythread_gpgsv_analysis,SLOT(gsv_jiexi(QStringList,QStringList)));
    connect(this,SIGNAL(send_gga_gsa_data(QString,QString,QString,QString)),mythread_gngga_gsa_analysis,SLOT(gga_gsa_jiexi(QString,QString,QString,QString)));
    connect(this,SIGNAL(send_gga_data(QString)),mythread_time,SLOT(gga_time_jiexi(QString)));
    connect(this,SIGNAL(send_lat_lon_height(QString,QString,QString)),mythread_WGS84,SLOT(lat_lon_height_jiexi(QString,QString,QString)));
    //线程 到 ui
    connect(mythread_gpgsv_analysis,SIGNAL(send_gsv_data(QList<int>,QList<int>)),this,SLOT(recv_gsvdata(QList<int>,QList<int>)));
    connect(mythread_gngga_gsa_analysis,SIGNAL(send_gga_gsa_data(QStringList,QStringList,QStringList,QStringList)),this,SLOT(recv_gga_gsa_data(QStringList,QStringList,QStringList,QStringList)));
    connect(mythread_time,SIGNAL(send_gga_time(QStringList)),this,SLOT(recv_gga_time(QStringList)));
    connect(mythread_WGS84,SIGNAL(send_X_Y_Z(double,double,double)),this,SLOT(recv_X_Y_Z(double,double,double)));
    //
    connect(&my_serialPort,&QSerialPort::readyRead,this,&BGTEST::readthings);
    timer = new QTimer;
    //connect(timer,&QTimer::timeout,this,&BGTEST::readthings);

    //设置文件属性
     path = qApp->applicationDirPath() + "/data.txt";
    f.setFileName(path);
    readfieltimer = new QTimer(this);
    connect(ui->timerSlider,&QSlider::valueChanged,this,[&]{readfieltimer->start(ui->timerSlider->value());});

    //保存文件
    timer_saveFile  = new QTimer;//保存文件定时器
    connect(timer_saveFile,&QTimer::timeout,this,&BGTEST::save_file);
    file_flag = false;

    QString mapHtml;
    mapHtml=QDir::currentPath()+"../my1.html";
    ui->axWidget->setControl(QString::fromUtf8("{8856F961-340A-11D0-A96B-00C04FD705A2}"));//注册组件UUID
    ui->axWidget->setProperty("DisplayScrollBars",true); // 显示滚动条
    ui->axWidget->setProperty("DisplayAlerts",false);    //不显示任何警告信息。
    ui->axWidget->dynamicCall("Navigate(const QString&)",mapHtml);

    paint(); //绘制星空图

    QPixmap myPix(":/new/2.png");
     ui->label_16->setPixmap(myPix);
    ui->label_16->setScaledContents(true);

    //绘制定位偏差曲线
    //在location error 界面添加误差曲线
    QCPGraph *location_error = ui->widget_track->addGraph();

    // set dark background gradient:
    QLinearGradient gradient(0, 0, 0, 400);
    gradient.setColorAt(0, QColor("#cdc5bf"));//#a6a5c4
    gradient.setColorAt(0.38, QColor("#cdc5bf"));
    gradient.setColorAt(1, QColor("#cdc5bf"));
    ui->widget_track->setBackground(QBrush(gradient));

    //设置图表标题
    ui->widget_track->legend->setVisible(true); //显示图例
    ui->widget_track->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);// //可拖拽+可滚轮缩放
    ui->widget_track->graph(0)->setName("两次定位偏差");

    //设置坐标范围
    ui->widget_track->yAxis->setRange(0,2);


}

BGTEST::~BGTEST()
{
    delete ui;
}


void BGTEST::SearchSerialPortName()
{
    QStringList list;
    foreach (const QSerialPortInfo &info,QSerialPortInfo::availablePorts())
    {


       QString a;
        QSerialPort serial;
        serial.setPort(info);
        if(serial.open(QIODevice::ReadWrite))
        {
            list <<serial.portName();
            //qSort(list.begin(),list.end());
            //a = info.serialNumber();
            //ui->Com->addItems(list);
            serial.close();
            //qDebug()<<info.systemLocation();
        }
        ui->Com->setCurrentIndex(0);
    }
    ui->Com->addItems(list);
}

void BGTEST::on_open_com_clicked()
{
    //qDebug()<<"dakai";
    //指示有无开启
    ui->State_com->setText(" 已连接");

    my_serialPort.setPortName(ui->Com->currentText());


    switch (ui->Baud->currentIndex()) {
    case 0 :
        //qDebug()<<"我是9600";
        my_serialPort.setBaudRate(QSerialPort::Baud9600);
        break;
    case 1 : my_serialPort.setBaudRate(QSerialPort::Baud19200);
        break;
    case 2 : my_serialPort.setBaudRate(QSerialPort::Baud38400);
        break;
    case 3: my_serialPort.setBaudRate(QSerialPort::Baud115200);
        break;
    //default: my_serialPort.setBaudRate(QSerialPort::Baud204800);
      //  break;
    }

    my_serialPort.setParity(QSerialPort::NoParity);
    my_serialPort.setDataBits(QSerialPort::Data8);
    my_serialPort.setStopBits(QSerialPort::OneStop);

//按键使能


    if(ui->open_com->text() == "打开串口"){
        ui->open_com->setText("关闭串口");
        ui->State_com->setText("已连接");
        //打开串口
        bool flag = my_serialPort.open(QIODevice::ReadWrite);

        if(flag == false){

            QMessageBox::warning(this,"warning","串口不存在或被占用","确定");
            ui->State_com->setText("未连接");
            ui->open_com->setText("打开串口");
        }
        else{
            ui->State_com->setText("已连接");
            ui->statusBar->showMessage("正在使用串口接收数据！");
           // timer->start(100);
        }
    }
    else {
        ui->open_com->setText("打开串口");
        ui->State_com->setText(" 未连接");
        my_serialPort.close();
        my_serialPort.clear();
        ui->statusBar->showMessage("");
        //timer->stop();
    }

}

void BGTEST::recv_gsvdata(QList<int> gpgsv, QList<int> bdgsv)
{
    this->gpgsv_data_table_widget = gpgsv;
    this->bdgsv_data_table_widget = bdgsv;
    //qDebug()<<"ui接收 gpsgsv:::"<<gpgsv_data_table_widget;
    //qDebug()<<"ui接收 BDgsv:::"<<bdgsv_data_table_widget;
    //信噪比数据处理
      ui->BD_tableWidget->resizeRowsToContents();
      ui->GPS_tableWidget->resizeRowsToContents();
      ui->GPS_tableWidget->clearContents();
      ui->BD_tableWidget->clearContents();

    for (int i=0 ;i<this->gpgsv_data_table_widget.length();i++) {
         ui->GPS_tableWidget->setItem(i/(i*8+8),i,new QTableWidgetItem(QString::number(gpgsv[i])));

    }

    for (int i=0 ;i<this->bdgsv_data_table_widget.length();i++) {
         ui->BD_tableWidget->setItem(i/(i*8+8),i,new QTableWidgetItem(QString::number(bdgsv[i])));
    }
    ui->GPS_tableWidget->repaint();
    ui->BD_tableWidget->repaint();



    //星空图数据处理
    this->satellite_update_flag = true;
    update();


}

void BGTEST::recv_gga_gsa_data(QStringList gngga_data, QStringList gpgsa_data, QStringList bdgsa_data,QStringList gnvtg_data)
{
    //qDebug()<<"gpgsa:"<<gpgsa_data;
   // qDebug()<<"bdgsa:"<<bdgsa_data;
    //qDebug()<<"gngga:"<<gngga_data;
    if(!gngga_data.isEmpty()){
       // ui->time_lineEdit->setText(gngga_data[0]);
        ui->latitude->setText(gngga_data[0]+"   "+gngga_data[1]);
        ui->longitude->setText(gngga_data[2]+"   "+gngga_data[3]);
        ui->height->setText(gngga_data[4]);
        lat = gngga_data[0];
        lon = gngga_data[2];
        height = gngga_data[4];
        //qDebug()<<"lat"<<lat;
        //qDebug()<<"lon"<<lon;
        if(!lat.isEmpty()&&!lon.isEmpty()&& !height.isEmpty()){
            emit send_lat_lon_height(lat,lon,height);
        }


        paint_location_error(gngga_data[0].toDouble(),gngga_data[2].toDouble());

        QString fun=QString("addpoint(%1,%2)").arg(lon).arg(lat);
        QAxObject *document = ui->axWidget->querySubObject("Document");
        QAxObject *parentWindow = document->querySubObject("parentWindow");
        parentWindow->dynamicCall("execScript(QString,QString)",fun,"JavaScript");

    }

    if(!gpgsa_data.isEmpty()){
        int j = gpgsa_data.length()-2;
        QString gps_id;
        for(int i = 0; i< j;i++){
         gps_id.append(gpgsa_data[i]+"  ");
        }
        //qDebug()<<gpgsa_data.last();//倒数第一位
       // qDebug()<<gpgsa_data.at(gpgsa_data.size()-2);//倒数第二位
         ui->gps_num_id->setText(gps_id);

    }
    if(!bdgsa_data.isEmpty()){
        int j = bdgsa_data.length()-2;
        QString bd_id;
        for(int i = 0; i< j;i++){
         bd_id.append(bdgsa_data[i]+"  ");

         // qDebug()<<"提取bd卫星号"<<bdgsa_data[i];
        }
        ui->BD_num_id->setText(bd_id);
        ui->pdop->setText("GPS: " + gpgsa_data.last() +"     BD: "+bdgsa_data.last() );
        ui->gps_num_use->setText("("+ QString::number(gpgsv_data_table_widget.length()/4) +"，"+ gpgsa_data.at(gpgsa_data.size()-2)+")");
        ui->bd_num_use->setText("("+ QString::number(bdgsv_data_table_widget.length()/4)  +"，"+ bdgsa_data.at(bdgsa_data.size()-2)+")");
    }
    if(!gnvtg_data.isEmpty()){
        if(gnvtg_data[0].toDouble()>0&& gnvtg_data[0].toDouble()<90){
            ui->course_angle->setText("东北 "+gnvtg_data[0]);
        }
        else if(gnvtg_data[0].toDouble()>90&& gnvtg_data[0].toDouble()<180){
            ui->course_angle->setText("东南 "+gnvtg_data[0]);
        }
        else if(gnvtg_data[0].toDouble()>180&& gnvtg_data[0].toDouble()<270){
            ui->course_angle->setText("西南 "+gnvtg_data[0]);
        }
        else if(gnvtg_data[0].toDouble()>270&& gnvtg_data[0].toDouble()<360){
            ui->course_angle->setText("西北 "+gnvtg_data[0]);
        }
        else if(gnvtg_data[0].toDouble() == 0){
            ui->course_angle->setText("正北 "+gnvtg_data[0]);
        }
        else if(gnvtg_data[0].toDouble() == 90){
            ui->course_angle->setText("正东 "+gnvtg_data[0]);
        }
        else if(gnvtg_data[0].toDouble() == 180){
            ui->course_angle->setText("正南 "+gnvtg_data[0]);
        }
        else if(gnvtg_data[0].toDouble() == 270){
            ui->course_angle->setText("正西 "+gnvtg_data[0]);
        }

        ui->speed->setText(gnvtg_data[1]+"  km/h");
        if(gnvtg_data[2] == 'A'){
            ui->positioning_state->setText("自主定位："+gnvtg_data[2]);
        }
        else if(gnvtg_data[2] == 'D'){
            ui->positioning_state->setText("差分定位："+gnvtg_data[2]);
        }
        else if(gnvtg_data[2] == 'E'){
            ui->positioning_state->setText("估算定位："+gnvtg_data[2]);
        }
        else if(gnvtg_data[2] == 'N'){
            ui->positioning_state->setText("无效定位："+gnvtg_data[2]);
        }
    }

}

void BGTEST::recv_gga_time(QStringList gngga_data)
{
    if(!gngga_data.isEmpty()){
        ui->time_lineEdit->setText(gngga_data[0]);
    }

}

void BGTEST::recv_X_Y_Z(double X, double Y, double Z)
{
    QString x = QString::number(X,'f',4);
    QString y = QString::number(Y,'f',4);
    QString z = QString::number(Z,'f',4);
    ui->X->setText(x);
    ui->Y->setText(y);
    ui->Z->setText(z);
}


void BGTEST::readthings()
{
    QByteArray buf;
    codec = QTextCodec::codecForName("GBK");//指定QString的编码方式
    buf = my_serialPort.readLine();
     getGSV(buf);
    if(savefile_flag == true){
        file_buffer.append(buf);
    }




}

bool BGTEST::eventFilter(QObject *watched, QEvent *event) //用过滤器eventFilter（）拦截QLabel中的QEvent::Paint事件
{
    if(watched ==ui->widget && event->type() == QEvent::Paint)
    {
        paint();
        if(this->satellite_update_flag == true)
        {

        }
    }
    return QWidget::eventFilter(watched,event);
}

int BGTEST::getGSV(QByteArray date)
{

    if(!date.isEmpty())
    {

          QString str_buf=codec->toUnicode(date);//buf转换成QString类型
//        qDebug()<< str_buf;
        serialdata.append(str_buf);
        if(str_buf.contains("GPTXT"))
        {

            QStringList datalist = serialdata.split("$");
            //qDebug()<< datalist;
            for (int i=0;i<datalist.length();i++) {
                if(datalist[i].contains("GPGSV")){
                    mygpsdata.GPS_GSV_data.append(datalist[i]);
                    //qDebug()<<datalist[i];
                }
                else if (datalist[i].contains("BDGSV")) {
                    mygpsdata.BD_GSV_data.append(datalist[i]);
                }
                else if(datalist[i].contains("GPGSA")){
                    mygpsdata.GPSGSA_data = datalist[i];
                }
                else if (datalist[i].contains("BDGSA")) {
                    mygpsdata.BDGSA_data = datalist[i];
                }
                else if (datalist[i].contains("GNGGA")) {
                    mygpsdata.GGA_data = datalist[i];
                }
                else if(datalist[i].contains("GNVTG")) {
                   mygpsdata.GNVTG_data = datalist[i];
                }

            }
            emit send_gsv_data(mygpsdata.GPS_GSV_data,mygpsdata.BD_GSV_data);
           emit send_gga_gsa_data(mygpsdata.GGA_data,mygpsdata.GPSGSA_data,mygpsdata.BDGSA_data,mygpsdata.GNVTG_data);
            emit send_gga_data(mygpsdata.GGA_data);
            //qDebug()<<"串口发送给线程GPS:"<<mygpsdata.GNVTG_data;
            mygpsdata.GPS_GSV_data.clear();
            mygpsdata.BD_GSV_data.clear();
            mygpsdata.GGA_data.clear();
            mygpsdata.GPSGSA_data.clear();
            mygpsdata.BDGSA_data.clear();
            mygpsdata.GNVTG_data.clear();
            serialdata.clear();

        }

    }
    ui->receiveDate->insertPlainText(date);
    ui->receiveDate->moveCursor(QTextCursor::End);
    if(ui->receiveDate->blockCount()>35){
        ui->receiveDate->setPlainText("");
        ui->receiveDate->clear();
    }

    date.clear();


}

void BGTEST::paint()
{
    int elevation,azimuth,id,bd_id;
    double cosLen,x,y;
    QPainter painter(ui->widget);
    painter.translate(200,190);//坐标变换  //y轴向下边为正方向
    painter.setRenderHint(QPainter::Antialiasing, true);

    //painter.setPen(QPen(QColor("#98F5FF"), 2));
    //painter.setBrush(QColor(	255 ,250 ,250));
    //painter.drawEllipse(QPointF(0, 0), 190, 190);

   // painter.setPen(QPen(QColor("#76EEC6"), 2));
    //painter.setBrush(QColor(	255 ,250 ,250));
    //painter.drawEllipse(QPointF(0, 0), 120, 120);

    //painter.setPen(QPen(QColor("#FFE4B5"), 2));
    //painter.setBrush(QColor(	255 ,250 ,250));
    //painter.drawEllipse(QPointF(0, 0), 60, 60);

    //painter.setPen(QPen(QColor("#696969"), 3));
    //painter.drawLine(QPointF(0, 190), QPointF(0,-190));
    //painter.drawLine(QPointF(-190,0), QPointF(190,0));

    //elevation：仰角 、（0-90）azimuth：方位角（0-360）

         //01,25,037,28,02,05,233,,03,40,083,38,06,38,237,
         for (int i=0;i<this->bdgsv_data_table_widget.length()/4;i++) {
           id = this->bdgsv_data_table_widget[i*4];
           elevation = this->bdgsv_data_table_widget[i*4+1];
           azimuth = this->bdgsv_data_table_widget[i*4+2];

           cosLen = cos(elevation*3.141592/180)*180;
           y = cos(azimuth*3.141592/180)*cosLen;
           x = sin(azimuth*3.141592/180)*cosLen;
           painter.setPen(QPen(QColor("#FF0000"), 2));//bd 颜色  #FF0000  #66FFFF
           painter.setBrush(QColor("#FFFAFA"));
           painter.drawEllipse(QPointF(x,-y),10,10);
           painter.setBrush(QColor(255,48,48));
           if(id>10){
           painter.drawText(QPointF(x-7,-y+5),QString::number(id));
           }
           else {
           painter.drawText(QPointF(x-3,-y+5),QString::number(id));
           }
         }


           for (int i=0;i<this->gpgsv_data_table_widget.length()/4;i++) {
             bd_id = this->gpgsv_data_table_widget[i*4];
             elevation = this->gpgsv_data_table_widget[i*4+1];
             azimuth = this->gpgsv_data_table_widget[i*4+2];

             cosLen = cos(elevation*3.141592/180)*180;
             y = cos(azimuth*3.141592/180)*cosLen;
             x = sin(azimuth*3.141592/180)*cosLen;
             painter.setPen(QPen(QColor("#FF77FF"), 2));//#8470ff
             painter.setBrush(QColor("#FFFAFA"));
             painter.drawEllipse(QPointF(x,-y),10,10);
             painter.setBrush(QColor(255,48,48));
             if(bd_id>10){
             painter.drawText(QPointF(x-7,-y+5),QString::number(bd_id));
             }
             else {
             painter.drawText(QPointF(x-3,-y+5),QString::number(bd_id));
             }
           }

}

void BGTEST::read_filedata()
{
    if(this->openfile_flag == false)
    {
        if(!this->f.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qDebug()<< "Open failed."  <<endl;
        }
        else {
            this->openfile_flag = true;
            file_flag = true;
        }
    }
    QString data;
    QString lineStr;

    //读取数据段 以$GPTXT子帧结束
    while(!f.atEnd())
    {
        lineStr = f.readLine();
        data.append(lineStr);
        //readcount++;
        this->readfile_length = this->readfile_length  + lineStr.length() + 1;
       if(lineStr.contains("GPTXT")==true)
        {
            ui->receiveDate->insertPlainText(data);
            ui->receiveDate->moveCursor(QTextCursor::End);
          break;

        }

    }
    //更新进度条
    //qDebug()<<"读取文件："<<readfile_length;
    //qDebug()<<"文件大小："<<f.size();
   // qDebug()<<"文件行数："<<readcount;
    ui->readfile_progressBar->setValue(readfile_length);

    QStringList datalist = data.split("$");
    //qDebug()<<datalist;
     for (int i=0;i<datalist.length();i++) {
        if(datalist[i].contains("GPGSV")){
            mygpsdata.GPS_GSV_data.append(datalist[i]);
            //qDebug()<<mygpsdata.GPS_GSV_data;
        }
        else if (datalist[i].contains("BDGSV")) {
            mygpsdata.BD_GSV_data.append(datalist[i]);
        }
        else if(datalist[i].contains("GPGSA")){
            mygpsdata.GPSGSA_data = datalist[i];
        }
        else if (datalist[i].contains("BDGSA")) {
            mygpsdata.BDGSA_data = datalist[i];
        }
        else if (datalist[i].contains("GNGGA")) {
            mygpsdata.GGA_data = datalist[i];
        }
        else if(datalist[i].contains("GNVTG")) {
           mygpsdata.GNVTG_data = datalist[i];
        }

    }


    emit send_gsv_data(mygpsdata.GPS_GSV_data,mygpsdata.BD_GSV_data);
    emit send_gga_gsa_data(mygpsdata.GGA_data,mygpsdata.GPSGSA_data,mygpsdata.BDGSA_data,mygpsdata.GNVTG_data);
     emit send_gga_data(mygpsdata.GGA_data);
    //qDebug()<<"文件读取发送给线程GPS:"<<mygpsdata.GNVTG_data;
     mygpsdata.GPS_GSV_data.clear();
     mygpsdata.BD_GSV_data.clear();
     mygpsdata.GGA_data.clear();
     mygpsdata.GPSGSA_data.clear();
     mygpsdata.BDGSA_data.clear();
     mygpsdata.GNVTG_data.clear();
    data.clear();

}


void BGTEST::on_dingwei_clicked()
{
    if(!lon.isEmpty() && !lat.isEmpty()){

            QString fun=QString("addpoint(%1,%2)").arg(lon).arg(lat);
            QAxObject *document = ui->axWidget->querySubObject("Document");
            QAxObject *parentWindow = document->querySubObject("parentWindow");
            parentWindow->dynamicCall("execScript(QString,QString)",fun,"JavaScript");
    }
    else {
        QMessageBox::warning(this,"警告","请检查串口是否打开，或者是有无接收到经纬度数据");
    }

}

void BGTEST::on_action_triggered()
{
    if(ui->action->text() == "读取文件回放"){
        file_flag = true;
        ui->timerSlider->setEnabled(true);
        readfieltimer->start(ui->timerSlider->value()*readfile_speed);
        connect(readfieltimer,&QTimer::timeout,this,[&]{read_filedata();});
        int j = f.size();
       ui->readfile_progressBar->setMaximum(j);
        ui->action->setText("正在读取");
    }
    else if(ui->action->text() == "正在读取") {
       ui->action->setText("读取文件回放");
       readfieltimer->stop();
       this->f.close();
       this->openfile_flag = false;
       file_flag = false;
       ui->readfile_progressBar->setValue(0);
       readfile_length = 0;
    }


}

void BGTEST::on_action_2_triggered()
{
    if(ui->action_2->text() == "保存原始文件"){
        ui->action_2->setText("停止保存");
        if(!path.isEmpty()){
            f.open(QIODevice::WriteOnly);
            timer_saveFile->start(1000);
            savefile_flag = true;
            file_zhengduan = false;
        }
    }
    else if(ui->action_2->text() == "停止保存"){
        ui->action_2->setText("保存原始文件");
        file_zhengduan = true;

        QString messagebox = QString("%1%2%3").arg("文件大小为: ").arg(f.size()).arg("  byte");
        QMessageBox::warning(this,"提示!",messagebox);
    }
}

void BGTEST::save_file(){
    QTextStream stream(&f);
    QString left_f;
    if(file_buffer.contains("GPTXT")&& file_buffer.contains("\r\n")){
        stream << file_buffer;
        file_buffer.clear();
    }
     if(file_zhengduan == true && file_buffer.contains("GPTXT")&&file_buffer.contains("\r\n")){
         timer_saveFile->stop();
         //qDebug()<<"到达这里";
        f.close();
        savefile_flag = false;
        serialdata.clear();
        file_buffer.clear();
    }

}

void BGTEST::paint_location_error(double lat, double lon)
{
    m_chartPoint_counter++;
    double lat1 = (M_PI /180)*this->now_lat;
    double lat2 = (M_PI /180)*lat;
    double lon1 = (M_PI /180)*this->now_lon;
    double lon2 = (M_PI /180)*lon;
    double R = 6371.393;

    this->location_error = acos(sin(lat1)*sin(lat2)+cos(lat1)*cos(lat2)*cos(lon2-lon1))*R;

    this->location_error = this->location_error*1000;

    if(m_chartPoint_counter<30){
        this->sx_vec.append(this->location_error );
        this->xAxis_vec.append(m_chartPoint_counter);
        ui->widget_track->xAxis->setRange(0,xAxis_vec.at(xAxis_vec.size()-1));
    }
    else {
        sx_vec.removeFirst();
        xAxis_vec.removeFirst();

        xAxis_vec.append(m_chartPoint_counter);
        this->sx_vec.append(this->location_error );
        ui->widget_track->xAxis->setRange(xAxis_vec.at(0),xAxis_vec.at(xAxis_vec.size()-1));

    }

    ui->widget_track->graph(0)->setData(this->xAxis_vec,this->sx_vec);

    //x轴名字
    ui->widget_track->xAxis->setLabel("X");
    //Y轴名字
    ui->widget_track->yAxis->setLabel("Y");
    ui->widget_track->replot();

    this->now_lat = lat;
    this->now_lon = lon;
}
