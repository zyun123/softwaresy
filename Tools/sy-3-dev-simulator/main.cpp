#include <QApplication>
#include "threads.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    auto f = [](base_server *svobj){
        auto t = new QThread;
        svobj->moveToThread(t);
        t->start();
    };

    robot_dash dash1(9991);
    f(&dash1);
    robot_rt rt1(9992);
    f(&rt1);
    robot_dash dash2(9993);
    f(&dash2);
    robot_rt rt2(9994);
    f(&rt2);
    bed_server bsv(9995);
    f(&bsv);

    robot_urp rb1(8081);
    robot_urp rb2(8080);
    QThread t1;
    QThread t2;
    rb1.moveToThread(&t1);
    t1.start();
    rb2.moveToThread(&t2);
    t2.start();

    return a.exec();
}
