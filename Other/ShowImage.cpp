#include "ShowImage.h"
#include <QFile>
#include <QDataStream>
#include <qtimer.h>
#include <qlogging.h>
#include <QTime>
#include "Other/Logger.h"
#include <QImageReader>

using namespace std;

ShowImage::ShowImage(QObject *parent) :
    QObject(parent)
{
    m_pImgProvider = new ImageProvider();
}

void ShowImage::setImage(QImage image)
{
    if(image.isNull())
        return;
    m_pImgProvider->img = image;
    emit callQmlRefeshImg(indexImage);
}

short ShowImage::getUint(char xx){
    short ret=xx;
    if (xx<0){
        ret=255+xx+1;
    }
    return ret;

}

void ShowImage::initData(int index)
{
#if 1
    indexImage=index;
    QImageReader reader;
    if(indexImage==0){
        reader.setFileName("cache/camera0.jpg");
    }else if(indexImage==1){
        reader.setFileName("cache/camera1.jpg");
    }else if(indexImage==2){
        reader.setFileName("cache/camera2.jpg");
    }else if(indexImage==3){
        reader.setFileName("cache/camera3.jpg");
    }

    if(indexImage==4){
        reader.setFileName("cache/camera_identify_m.jpg");
    }else if(indexImage==5){
        reader.setFileName("cache/camera_identify_l.jpg");
    }else if(indexImage==6){
        reader.setFileName("cache/camera_identify_r.jpg");
    }else if(indexImage==7){
        reader.setFileName("cache/camera_identify_h.jpg");
    }

    if(indexImage==8){
        reader.setFileName("cache/camera_cure_m.jpg");
    }else if(indexImage==9){
        reader.setFileName("cache/camera_cure_l.jpg");
    }else if(indexImage==10){
        reader.setFileName("cache/camera_cure_r.jpg");
    }else if(indexImage==11){
        reader.setFileName("cache/camera_cure_h.jpg");
    }


//    QImageReader reader("/home/yunze/code/app/camera0.jpg");
    reader.setAutoTransform(true);
    const QImage image = reader.read();
#else
    QFile file("/Users/xjt/Downloads/build-qtDemo-Desktop_Qt_5_12_11_clang_64bit-Debug/jpg.bin");
    if (!file.exists()){
        return;
    }

    long onecount=720*1280;
    long alllength=onecount*3;
    file.open(QIODevice::ReadOnly);
    if (file.size()!=alllength){
        return;
    }

    QTime time;
    time.start();

    QByteArray dataArr=file.read(alllength);
    file.close();
    int width = 1280;
    int height = 720;
    //不是在窗口绘图不需要重写绘图事件

       //1 创建绘图设备 指定透明背景且只能保存为原来格式才能透明 --例如本来png改成jpg就不是透明
       QImage image(width,height,QImage::Format_RGB32);   //比QPixmap多一个参数 参三格式设置为透明
//LOG_INFO << "deal image1 "<<time.elapsed();
       //QImage可以修改图片
       //对图片的2500个像素进行修改
       for(int i=0;i<height;i++)
       {
           for(int j=0;j<width;j++)
           {
               //设置像素
               int index=(i*width+j)*3;
               short blue=getUint(dataArr[index]);
               short green=getUint(dataArr[index+1]);
               short red=getUint(dataArr[index+2]);
               image.setPixel(QPoint(j,i),qRgb(red,green,blue));
           }
       }
//LOG_INFO << "deal image1 "<<time.elapsed();
//    image.save("/Users/xjt/Downloads/build-qtDemo-Desktop_Qt_5_12_11_clang_64bit-Debug/recog.jpg");
    #endif
       setImage(image);
  //  LOG_INFO << "deal image "<<time.elapsed();

}

void ShowImage::setIndexImage(int indexMess){
    indexImage=indexMess;
}


void ShowImage::update(){
    setImage(pubImage);
    timer->stop();
}

void ShowImage::setListArray(QList<QByteArray> dataArr){
    myArrayList=dataArr;
}

void ShowImage::getFromServerImgBin(){
    int width = 1280;
    int height = 720;
    //不是在窗口绘图不需要重写绘图事件

    QByteArray arr;
    //1 创建绘图设备 指定透明背景且只能保存为原来格式才能透明 --例如本来png改成jpg就不是透明
       QImage image(width,height,QImage::Format_RGB32);   //比QPixmap多一个参数 参三格式设置为透明
       foreach (const QByteArray &b, myArrayList) {
          arr=b;
       }
       //QImage可以修改图片
       //对图片的2500个像素进行修改

       for(int i=0;i<height;i++)
       {
           for(int j=0;j<width;j++)
           {
               //设置像素
               int index=(i*width+j)*3;
               short blue=getUint(arr[index]);
               short green=getUint(arr[index+1]);
               short red=getUint(arr[index+2]);
               image.setPixel(QPoint(j,i),qRgb(red,green,blue));
           }
       }
       pubImage=image;
       setImage(image);
    image.save("/Users/xjt/Downloads/build-qtDemo-Desktop_Qt_5_12_11_clang_64bit-Debug/recog1.jpg");

}
