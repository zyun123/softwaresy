#ifndef SHOWIMAGE_H
#define SHOWIMAGE_H

#include "ImageProvider.h"
class ShowImage: public QObject

{
    Q_OBJECT
   public:
       explicit ShowImage(QObject *parent = 0);
       ImageProvider *m_pImgProvider;

   public slots:
       void setImage(QImage image);
       void initData(int index);
       short getUint(char xx);
       void getFromServerImgBin();
       void update();
       void setListArray(QList<QByteArray> dataArr);
       void setIndexImage(int indexMess);

   signals:
       void callQmlRefeshImg(int index);
       void timeout();
private:
       QImage pubImage;
       QTimer *timer;
       QList<QByteArray> myArrayList;
       int indexImage;

};

#endif // SHOWIMAGE_H
