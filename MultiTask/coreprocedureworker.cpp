#include "coreprocedureworker.h"
#include <QThread>
#include <opencv2/opencv.hpp>
#include <QFloat16>
#include <NumCpp.hpp>
#include <QList>
#include <QDebug>
#include <QThread>
#include <QJsonArray>
#include <QFile>
#include <QJsonDocument>
#include <QImage>
#include <QPixmap>
#include <QtMath>
#include <QDateTime>
#include <QDir>
#include <QProcess>
#include <thread>

#include <open3d/Open3D.h>
#include <Eigen/Dense>

#define PI 3.1415926
#define ROBOTX_THREHOLD_HEAD 0.6//机械臂床头超限x
#define ROBOTX_THREHOLD_TAIL 0.7

CoreProcedureWorker::CoreProcedureWorker(QVector<QSharedPointer<CameraControl>> camobjs, QVector<QSharedPointer<RobotControl> > robots,
                                         QSharedPointer<BedControl> bedctrl, QObject *parent) : QObject(parent)
{
    m_camobjs = camobjs;
    m_robots = robots;
    m_bedCtrl = bedctrl;
    m_using3D = false;
    m_bedPos = 0;
    m_isTreating = false;
    m_stopTreatment = false;
    m_iniRedPtFlag = false;
    m_contact_bias = 0;

    //生成机械臂姿态对照表
    QVector<double> yAxis = {-10, -0.62, -0.55, -0.45, 10};
    QVector<double> xAxis = {-10, -0.5, -0.1, 0.3, 0.6, 10};
    QVector<QVector<double>> xyzs;
    for(int i = 1; i < yAxis.size(); i++)
        for(int j = 1; j < xAxis.size(); j++)
        {
            pose_book tmp;
            tmp.low_x = xAxis[j - 1];
            tmp.high_x = xAxis[j];
            tmp.low_y = yAxis[i - 1];
            tmp.high_y = yAxis[i];
            m_poseBook.append(tmp);
        }
    QVector<double> rxs_l = {30, 15, 0, -15};
    QVector<double> rys_l = {-150, -165, 180, 165, 150};
    QVector<double> rxs_r = {150, 165, 180, -165};
    QVector<double> rys_r = {-30, -15, 0, 15, 30};
    int index = 0;
    for(int i = 0; i < rxs_l.size(); i++)
        for(int j = 0; j < rys_l.size(); j++)
        {
            m_poseBook[index].pose_left.append(rxs_l[i] * PI / 180);
            m_poseBook[index].pose_left.append(rys_l[j] * PI / 180);
            m_poseBook[index].pose_left.append(0);
            m_poseBook[index].pose_right.append(rxs_r[i] * PI / 180);
            m_poseBook[index].pose_right.append(rys_r[j] * PI / 180);
            m_poseBook[index].pose_right.append(0);
            index++;
        }
    for(int i = 0; i < m_poseBook.size(); i++)
    {
        m_poseBook[i].pose_left = RobotControl::eulerAngleToRotationVec(m_poseBook[i].pose_left);
        m_poseBook[i].pose_right = RobotControl::eulerAngleToRotationVec(m_poseBook[i].pose_right);
    }

    //AI接口初始化
    auto t1 = QDateTime::currentMSecsSinceEpoch();
    m_predictObj = new KPPredictFactory();
    LOG_INFO << "AI接口初始化：" << m_predictObj->m_abstract_kp->init(3, {{"bodypose", "true"}, {"handpose", "false"}});
    auto t2 = QDateTime::currentMSecsSinceEpoch();
    LOG_INFO << "AI接口初始化耗时" << t2 - t1;
}

void CoreProcedureWorker::goCalibrate(QJsonObject args)
{
    QString rbName = args.value("robot_direction").toString();
    int rbIndex = (rbName == "左侧机器人") ? 0 : 1;
    QString camName = args.value("cam").toString();
    QChar camch = 'l';
    if(camName == "左侧摄像头")
        camch = 'l';
    else if(camName == "右侧摄像头")
        camch = 'r';
    else if(camName == "顶部摄像头")
        camch = 'm';
    else
        camch = 'h';
    emit blockUI("正在标定机械臂和相机，请稍候...", 0);
    //先把对侧机械臂移动到拍照位置
    for(int i = 0; i < m_robots.size(); i++)
        m_robots[i]->freeDrive(false);
    if(rbIndex == 0)
    {
        if(!m_robots[1]->isInPhotoPos())
        {
            m_robots[1]->safePos();
            m_robots[1]->photoPos();
        }
    }
    else
    {
        if(!m_robots[0]->isInPhotoPos())
        {
            m_robots[0]->safePos();
            m_robots[0]->photoPos();
        }
    }
    m_robots[rbIndex]->safePos();
    //机械臂移至标定初始位置
    m_robots[rbIndex]->calibratePos(camch);
    int camIndex = g_cameraOrder.indexOf(camch);
    LOG_INFO << "robot index: " << rbIndex << " cam index: " << camIndex;
    //设置标定板中心为tcp
    m_robots[rbIndex]->setTCP(QVector<double>({-0.0597, 0, 0.04775, 0, 0, 0}));//如加坤维外置传感器 传感器高度43.9mm
//    m_robots[rbIndex]->setTCP(QVector<double>({-0.0597, 0, 0.09175, 0, 0, 0}));//如加坤维外置传感器 传感器高度43.9mm
    //m_robots[rbIndex]->setTCP(QVector<double>({-0.0597, 0, 0.09475, 0, 0, 0})); 标定板和法轮盘间加外置传感器
    m_robots[rbIndex]->setPayLoad(1.06, QVector<double>({0, 0, 0}));
    QThread::msleep(500);//等待机械臂更新位姿
    //走3x3x3网格
    auto inipose = Global::fromArray(m_robots[rbIndex]->getStatus().toolVectorActual, 6);
    QList<double> toolpts;
    QList<double> observepts;
    int errcnt = 0;
    for(int i = -1; i < 2; i++)
        for(int j = -1; j < 2; j++)
            for(int k = -1; k < 2; k++)
            {
                auto toolposition = inipose;
                toolposition[0] += i * 0.05;
                toolposition[1] += j * 0.05;
                toolposition[2] += k * 0.05;

                m_robots[rbIndex]->moveToPos('l', 1, toolposition, 0.05, 0.05);
                m_robots[rbIndex]->waitReplyFromTCPClient({"movel"});

                //cv2 checkboard
                auto imdat = m_camobjs[camIndex]->getBGRData();
                auto depthdat = m_camobjs[camIndex]->getDepthData();
                if(imdat == nullptr || depthdat == nullptr)
                {
                    LOG_EXCEPTION_CAM << "cali error, get camera image failed!" << (imdat == nullptr) << (depthdat == nullptr);
                    emit blockUI(QString("标定失败，无法获取相机图像数据"), 5000);
                    return;
                }
                cv::Mat imgdata_BGR(IMAGE_HEIGHT, IMAGE_WIDTH, CV_8UC3, imdat.data());
                cv::Mat imgdata_GRAY;
                cv::cvtColor(imgdata_BGR, imgdata_GRAY, cv::COLOR_BGR2GRAY);
                cv::Mat corners;
                if(cv::findChessboardCorners(imgdata_GRAY, cv::Size(3,3), corners, cv::CALIB_CB_ADAPTIVE_THRESH))
                {
                    cv::cornerSubPix(imgdata_GRAY, corners, cv::Size(3,3), cv::Size(-1,-1),
                                     cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 30, 0.001));
                    float centerX = corners.row(4).col(0).at<float>(0);
                    float centerY = corners.row(4).col(0).at<float>(1);
                    auto d = CameraControl::getDepthValue(centerX, centerY, depthdat);
                    if(d == 0)
                    {
                        LOG_EXCEPTION_CAM << "cali error, get depth failed!";
                        emit blockUI(QString("标定失败，获取深度值错误"), 5000);
                        return;
                    }
                    double x, y;
                    m_camobjs[camIndex]->cam2dToCam3d(centerX, centerY, d, x, y);
                    toolpts << toolposition[0] * 1000 << toolposition[1] * 1000 << toolposition[2] * 1000;
                    observepts << x << y << d;
                    cv::drawChessboardCorners(imgdata_BGR, cv::Size(1,1), corners.row(4), true);
                }
                else
                {
                    LOG_INFO << QString("cam %1 chessboard not found").arg(camIndex);
                    errcnt++;
                    if(errcnt >= 3)
                    {
                        LOG_EXCEPTION_CAM << "cali error, find chessboard failed!";
                        emit blockUI(QString("标定失败，识别标定板中心失败次数过多"), 5000);
                        return;
                    }
                }
            }
    LOG_INFO << "camera axis:";
    for(int i = 0; i < observepts.size(); i+=3)
        LOG_INFO << observepts[i] << " " << observepts[i + 1] << " " << observepts[i + 2];
    LOG_INFO << "robot axis:";
    for(int i = 0; i < toolpts.size(); i+=3)
        LOG_INFO << toolpts[i] << " " << toolpts[i + 1] << " " << toolpts[i + 2];

    int ptCnt = observepts.size() / 3;
    nc::NdArray<double> measured_pts = nc::asarray(toolpts.toStdList());
    nc::NdArray<double> observed_pts = nc::asarray(observepts.toStdList());
    measured_pts.reshape(ptCnt, 3);
    observed_pts.reshape(ptCnt, 3);
    auto centroid_A = nc::mean(observed_pts, nc::Axis::ROW);
    auto centroid_B = nc::mean(measured_pts, nc::Axis::ROW);
    auto AA = observed_pts - nc::tile(centroid_A, ptCnt, 1);
    auto BB = measured_pts - nc::tile(centroid_B, ptCnt, 1);
    auto H = nc::dot(nc::transpose(AA), BB);
    nc::NdArray<double> U;
    nc::NdArray<double> S;
    nc::NdArray<double> Vt;
    nc::linalg::svd(H, U, S, Vt);
    auto V = nc::transpose(Vt);
    auto R = nc::dot(V, nc::transpose(U));
    if(nc::linalg::det(R) < 0)
    {
        LOG_INFO << "Reflection detected";
        auto rows = V.shape().rows;
        auto cols = V.shape().cols;
        for(unsigned int i = 0; i < rows; i++)
            V[i * cols + 2] *= -1;
        R = nc::dot(V, nc::transpose(U));
    }
    auto t = nc::dot(-R, nc::transpose(centroid_A)) + nc::transpose(centroid_B);
    t.reshape(3, 1);
    auto camera_pose = nc::concatenate({nc::concatenate({R, t}, nc::Axis::COL), nc::NdArray<double>({{0, 0, 0, 1}})}, nc::Axis::ROW);
    auto registered_pts = nc::dot(R, nc::transpose(observed_pts)) + nc::tile(t, 1, ptCnt);
    auto pre_error = nc::transpose(registered_pts) - measured_pts;
    auto pre_error1 = nc::multiply(pre_error, pre_error);
    auto pre_error2 = nc::sum(pre_error1, nc::Axis::COL);
    auto error = nc::sum(nc::sqrt(pre_error2));
    auto rmse = error.at(0,0) / ptCnt;
    LOG_INFO << "max err: " << nc::sqrt(pre_error2).max()[0];
    LOG_INFO << "min err: " << nc::sqrt(pre_error2).min()[0];
    LOG_INFO << "RMSE: " << rmse;
    m_camobjs[camIndex]->updateExtrinsics(camera_pose, rbIndex);
    nc::tofile(camera_pose, QString("extrinsics/extrinsic_%1_to_robot_%2.txt").arg(g_cameraSerialNums[camIndex]).arg(rbIndex).toStdString(), ' ');
    //重置机械臂tcp负载
    m_robots[rbIndex]->robotStop(true);
    QThread::msleep(500);
    m_robots[rbIndex]->robotPlay(true);
    QThread::msleep(500);
    LOG_INFO << QString("camera %1 done").arg(camIndex);
    emit blockUI(QString("标定完成，RMSE: ") + QString::number(rmse), 2000);
}

void CoreProcedureWorker::goCalibrateSensor(int rbindex)
{
    QString calixy = (rbindex == 0 ? "cali_xy_l" : "cali_xy_r");
    QString caliz = (rbindex == 0 ? "cali_z_l" : "cali_z_r");
    QString caliover = (rbindex == 0 ? "cali_over_l" : "cali_over_r");
    emit blockUI("正在校准外置传感器，请稍候...", 0);
    auto socket = new QTcpSocket;
    socket->connectToHost("127.0.0.1", 8680);
    if(!socket->waitForConnected(3000))
    {
        emit blockUI("校准失败，传感器喂数进程连接失败", 3000);
        return;
    }
    m_robots[rbindex]->safePos();
    QThread::sleep(10);
    socket->write(calixy.toLocal8Bit());
    socket->flush();
    //位置2
    QVector<double> pos;
    if(rbindex == 0)
        pos = {1.571, -1.571, 1.571, -1.571, 0, 0};
    else
        pos = {-1.571, -1.571, -1.571, -1.571, 0, 3.141};
    m_robots[rbindex]->moveToPos('j', 2, pos, 0.5, 0.5);
    if(m_robots[rbindex]->waitReplyFromTCPClient({"movej"}) != 0)
    {
        emit blockUI("校准失败，机械臂移动错误", 3000);
        return;
    }
    QThread::sleep(10);
    socket->write(caliz.toLocal8Bit());
    socket->flush();
    //位置3
    if(rbindex == 0)
        pos = {1.571, -1.571, 1.571, -1.571, 1.571, 0};
    else
        pos = {-1.571,-1.571,-1.571,1.571,1.571,3.141};
    m_robots[rbindex]->moveToPos('j', 2, pos, 0.5, 0.5);
    if(m_robots[rbindex]->waitReplyFromTCPClient({"movej"}) != 0)
    {
        emit blockUI("校准失败，机械臂移动错误", 3000);
        return;
    }
    QThread::sleep(10);
    socket->write(calixy.toLocal8Bit());
    socket->flush();
    //位置4
    if(rbindex == 0)
        pos = {1.571,-1.571,1.571,-3.14,-1.571,0};
    else
        pos = {-1.571,-1.571,-1.571,0,1.571,3.141};
    m_robots[rbindex]->moveToPos('j', 2, pos, 0.5, 0.5);
    if(m_robots[rbindex]->waitReplyFromTCPClient({"movej"}) != 0)
    {
        emit blockUI("校准失败，机械臂移动错误", 3000);
        return;
    }
    QThread::sleep(10);
    socket->write(caliz.toLocal8Bit());
    socket->flush();

    QThread::sleep(5);
    socket->write(caliover.toLocal8Bit());
    socket->flush();

    socket->disconnectFromHost();
    QThread::sleep(1);
    m_robots[rbindex]->safePos();
    m_robots[rbindex]->photoPos();
    emit blockUI("校准完成", 1000);
}

void CoreProcedureWorker::goToChessBoard(int rbindex, QString camindex)
{
    m_bedCtrl->moveTo(0);
    for(int i = 0; i < m_robots.size(); i++)
        m_robots[i]->freeDrive(false);
    if(!m_robots[0]->isInPhotoPos())
        m_robots[0]->safePos();
    if(!m_robots[1]->isInPhotoPos())
        m_robots[1]->safePos();
    Global::parallel<void, RobotControl, bool*>(&RobotControl::photoPos, m_robots[0], m_robots[1], nullptr);

    int camid = 0;
    QChar camch = 'l';
    if(camindex == "左侧摄像头")
        camch = 'l';
    else if(camindex == "右侧摄像头")
        camch = 'r';
    else if(camindex == "顶部摄像头")
        camch = 'm';
    else
        camch = 'h';
    camid = g_cameraOrder.indexOf(camch);
    auto imdat = m_camobjs[camid]->getBGRData();
    auto depthdat = m_camobjs[camid]->getDepthData();
    if(imdat == nullptr || depthdat == nullptr)
    {
        LOG_EXCEPTION_CAM << "get camera image failed!" << (imdat == nullptr) << (depthdat == nullptr);
        return;
    }
    cv::Mat imgdata_BGR(IMAGE_HEIGHT, IMAGE_WIDTH, CV_8UC3, imdat.data());
    cv::Mat imgdata_GRAY;
    cv::cvtColor(imgdata_BGR, imgdata_GRAY, cv::COLOR_BGR2GRAY);
    cv::Mat corners;
    if(cv::findChessboardCorners(imgdata_GRAY, cv::Size(3,3), corners, cv::CALIB_CB_ADAPTIVE_THRESH))
    {
        cv::cornerSubPix(imgdata_GRAY, corners, cv::Size(3,3), cv::Size(-1,-1),
                         cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 30, 0.001));
        cv::drawChessboardCorners(imgdata_BGR, cv::Size(1,1), corners.row(4), true);
        float centerX = corners.row(4).col(0).at<float>(0);
        float centerY = corners.row(4).col(0).at<float>(1);
        auto d = CameraControl::getDepthValue(centerX, centerY, depthdat);
        if(d == 0)
        {
            LOG_EXCEPTION_CAM << "cali error, get depth failed!";
            return;
        }
        double x, y;
        m_camobjs[camid]->cam2dToCam3d(centerX, centerY, d, x, y);
        double chessboard_x = x;
        double chessboard_y = y;
        double chessboard_z = d;
        LOG_INFO << "cam go to " << chessboard_x << " " << chessboard_y << " " << chessboard_z;
        m_camobjs[camid]->cam3dToRobot3d(rbindex, chessboard_x, chessboard_y, chessboard_z);
        LOG_INFO << "rb want go to " << chessboard_x << " " << chessboard_y << " " << chessboard_z;

        if(qAbs(chessboard_x) > 0.6)
            emit warn("警告", "标定板中心位置超过机械臂臂展范围，请调整后重试");
        else
        {
            auto pose = getPoseByXY(rbindex, rbindex == 0 ? chessboard_x : -chessboard_x, chessboard_y);
            QVector<double> tmp;
            tmp.append(chessboard_x);
            tmp.append(chessboard_y);
            tmp.append(chessboard_z + 0.05);
            tmp.append(pose[0]);
            tmp.append(pose[1]);
            tmp.append(pose[2]);
            m_robots[rbindex]->safePos();
            m_robots[rbindex]->moveToPos('l', 1, tmp, 0.1, 0.1);
            m_robots[rbindex]->waitReplyFromTCPClient({"movel"});
            LOG_INFO << "rb go to " << chessboard_x << " " << chessboard_y << " " << chessboard_z;
            m_robots[rbindex]->contact({0, 0, -0.02, 0, 0, 0}, 0.1, 1);
            m_robots[rbindex]->waitReplyFromTCPClient(QStringList({"tool_contact"}));
            auto curpos = Global::fromArray(m_robots[rbindex]->getStatus().toolVectorActual, 6);
            emit warn("提示", QString("Z方向偏差为 ") + QString::number(qAbs((chessboard_z - curpos[2]) * 1000.0), 'g', 3) + " mm,\n请手动测量机械臂工具中心到标定板中心的X和Y偏差");
        }
    }
    else
    {
        LOG_INFO << "find corner failed";
        emit warn("警告", "识别标定板中心失败，请确认标定板在相机视野内");
    }
}

void CoreProcedureWorker::upAndDown(int rbindex, bool up, QString step)
{
    auto pose = Global::fromArray(m_robots[rbindex]->getStatus().toolVectorActual, 6);
    QVector<double> tmp;
    tmp.append(pose[0]);
    tmp.append(pose[1]);
    tmp.append(pose[2]);
    if(rbindex == 0)
    {
        tmp.append(0.005);
        tmp.append(-3.134);
        tmp.append(0.003);
    }
    else
    {
        tmp.append(3.141);
        tmp.append(-0.002);
        tmp.append(-0.013);
    }
    if(up)
        tmp[2] += step.toDouble();
    else
        tmp[2] -= step.toDouble();
    m_robots[rbindex]->moveToPos('l', 1, tmp, 0.1, 0.1);
    m_robots[rbindex]->waitReplyFromTCPClient({"movel"});
}

int CoreProcedureWorker::predictIn2D(QStringList acunames, QVector<QSharedPointer<quint8>> rgbs, int mode, QMap<QString, IdentifyPointInfo> &acupoints, std::map<string, std::vector<double> > &boxes)
{
    //输入2d图像
    std::map<std::string, cv::Mat> input_imgs;
    for(int i = 0; i < rgbs.size(); i++)
    {
        auto key = QString(g_cameraOrder[i]).toStdString();
        cv::Mat mat(IMAGE_HEIGHT, IMAGE_WIDTH, CV_8UC3, rgbs[i].data());
        input_imgs[key] = mat.clone();
    }
    //输入经络点
    std::vector<std::string> input_acunames;
    for(int i = 0; i < acunames.size(); i++)
    {
        auto tmp = acunames[i];
        if(tmp.contains("du") || tmp.contains("ren"))
            input_acunames.push_back(tmp.replace("L-", "R-").toStdString());
        else
            input_acunames.push_back(tmp.toStdString());
    }
    LOG_INFO << "输入经络点：" << acunames;
    //ai接口返回值 pix_x pix_y cam_index
    std::vector<double> kppoints;
    //当前调理床位置
    double bed_bias = 0;
    auto [bsta, suc] = m_bedCtrl->getBedStatus();
    if(suc) bed_bias = bsta.pos;
    //ai识别
    auto t1 = QDateTime::currentMSecsSinceEpoch();
    auto predict_result = m_predictObj->m_abstract_kp->Predict(input_imgs, input_acunames, mode, kppoints, boxes, bed_bias);
    auto t2 = QDateTime::currentMSecsSinceEpoch();
    LOG_INFO << "AI识别耗时" << t2 - t1 << "模式" << mode;
    //处理识别结果
    if(predict_result != "识别正常")
    {
        LOG_EXCEPTION_AI << "identify fail:" << QString::fromStdString(predict_result);
        if(predict_result == "c10 error judge updown" || predict_result == "c10 error judge hand updown")
            return -2;
        else
            return -1;
    }
    else if(kppoints.size() != acunames.size() * 3)
    {
        LOG_EXCEPTION_AI << "identify fail size error" << kppoints.size();
        return -1;
    }
    else
    {
        LOG_INFO << QString::fromStdString(predict_result);
        LOG_INFO << "kppoints:" << QVector<double>::fromStdVector(kppoints);
        LOG_INFO << "box:" << "m" << (boxes.count("m") > 0 ? QVector<double>::fromStdVector(boxes["m"]) : QVector<double>({}))
                           << "h" << (boxes.count("h") > 0 ? QVector<double>::fromStdVector(boxes["h"]) : QVector<double>({}))
                           << "l" << (boxes.count("l") > 0 ? QVector<double>::fromStdVector(boxes["l"]) : QVector<double>({}))
                           << "r" << (boxes.count("r") > 0 ? QVector<double>::fromStdVector(boxes["r"]) : QVector<double>({}));
        acupoints.clear();
        QString aiorderofcam = "mrlh";
        for(int i = 0; i < acunames.size(); i++)
        {
            IdentifyPointInfo tmpinfo;
            tmpinfo.acuname = acunames[i];
            tmpinfo.pix_x = kppoints[3*i];
            tmpinfo.pix_y = kppoints[3*i+1];
            tmpinfo.camid = g_cameraOrder.indexOf(aiorderofcam.at(kppoints[3*i+2]));
            acupoints.insert(acunames[i], tmpinfo);
        }
    }
    return 0;
}

int CoreProcedureWorker::calRobotXYZ(QVector<QSharedPointer<quint8>> depths, QMap<QString, IdentifyPointInfo> &acupoints)
{
    //获取所有点的相机深度
    for(int i = 0; i < acupoints.keys().size(); i++)
    {
        auto key = acupoints.keys().at(i);
        auto camid = acupoints[key].camid;
        auto pix_x = acupoints[key].pix_x;
        auto pix_y = acupoints[key].pix_y;
        auto d = CameraControl::getDepthValue(pix_x, pix_y, depths[camid]);
        if(d == 0)
        {
            d = CameraControl::getEstimateDepthValue(pix_x, pix_y, depths[camid]);
            LOG_INFO << "fix depth" << acupoints[key].acuname << d;
        }
        if(d == 0)
        {
            LOG_EXCEPTION_CAM << "lost depth:" << acupoints[key].acuname;
            return -1;
        }
        acupoints[key].cam_z = d;
    }
    //处理估算较差的点，如果对侧有深度取对侧深度
    for(int i = 0; i < acupoints.keys().size(); i++)
    {
        auto key = acupoints.keys().at(i);
        auto tmp = key;
        auto otherkey = (tmp.contains("L-") ? tmp.replace("L-", "R-") : tmp.replace("R-", "L-"));
        if(acupoints[key].cam_z <= 0 && acupoints[otherkey].cam_z > 0
                && acupoints[key].camid == acupoints[otherkey].camid)
            acupoints[key].cam_z = acupoints[otherkey].cam_z;
    }
    //当前调理床位置
    double bed_bias = 0;
    auto [bsta, suc] = m_bedCtrl->getBedStatus();
    if(suc) bed_bias = bsta.pos;
    //计算所有点的机械臂坐标
    for(int i = 0; i < acupoints.keys().size(); i++)
    {
        auto key = acupoints.keys().at(i);
        auto d = qAbs(acupoints[key].cam_z);
        auto pix_x = acupoints[key].pix_x;
        auto pix_y = acupoints[key].pix_y;
        double x, y;
        m_camobjs[acupoints[key].camid]->cam2dToCam3d(pix_x, pix_y, d, x, y);
        acupoints[key].cam_x = x;
        acupoints[key].cam_y = y;
        acupoints[key].cam_z = d;
        calculatepts(acupoints[key], bed_bias);
    }
    //部分穴位点特殊处理
    //任督脉右侧镜像左侧机械臂坐标
    for(int i = 0; i < acupoints.keys().size(); i++)
    {
        auto key = acupoints.keys().at(i);
        auto tmp = key;
        auto otherkey = (tmp.contains("L-") ? tmp.replace("L-", "R-") : tmp.replace("R-", "L-"));
        if(key.startsWith("L-du") || key.startsWith("R-ren"))
        {
            acupoints[key].robot_x = -acupoints[otherkey].robot_x;
            acupoints[key].robot_y = -1.5 - acupoints[otherkey].robot_y;
            acupoints[key].robot_z = acupoints[otherkey].robot_z;
        }
    }
    //位置重叠的点往左右错开
//    QStringList tmpplist = {"L-wei-4", "L-wei-6", "L-wei-11", "L-gan-20", "L-gan-28"};
//    for(int i = 0; i < acupoints.keys().size(); i++)
//    {
//        auto key = acupoints.keys().at(i);
//        auto tmp = key;
//        auto otherkey = (tmp.contains("L-") ? tmp.replace("L-", "R-") : tmp.replace("R-", "L-"));
//        if(tmpplist.contains(acupoints[key].acuname))
//        {
//            double y_ref = -1.47;
//            auto mv = (y_ref - acupoints[key].robot_y - acupoints[otherkey].robot_y) / 2;
//            if(mv >= 0)
//            {
//                acupoints[key].robot_y += mv + 0.007;
//                acupoints[otherkey].robot_y += mv + 0.007;
//            }
//        }
//    }
    return 0;
}

void CoreProcedureWorker::calIniBedPos(QMap<QString, QVector<QStringList>> &curelist, QMap<QString, IdentifyPointInfo> &acupoints)
{
    for(int i = 0; i < curelist.keys().size(); i++)
    {
        auto jingluo = curelist.keys().at(i);
        double max_x = -10;
        double min_x = 10;
        for(int j = 0; j < 2; j++)
            for(int k = 0; k < curelist[jingluo][j].size(); k++)
            {
                auto acuname = curelist[jingluo][j][k];
                double x = (j == 0 ? acupoints[acuname].robot_x : -acupoints[acuname].robot_x);
                if(x > max_x)
                    max_x = x;
                if(x < min_x)
                    min_x = x;
            }
        if(max_x - min_x >= ROBOTX_THREHOLD_HEAD + ROBOTX_THREHOLD_TAIL)//经络点区间超过机械臂臂展
        {
            if(acupoints[curelist[jingluo][0][0]].robot_x > 0)//床初始位置平齐床尾最远的点
                m_iniBedPos[jingluo] = ROBOTX_THREHOLD_TAIL - max_x - 0.01;
            else//床初始位置平齐床头最远的点
                m_iniBedPos[jingluo] = -ROBOTX_THREHOLD_HEAD - min_x + 0.01;
        }
        else//经络点区间不超过机械臂臂展，床初始位置居中
        {
            m_iniBedPos[jingluo] = (ROBOTX_THREHOLD_TAIL - ROBOTX_THREHOLD_HEAD) / 2 - (max_x + min_x) / 2;
        }
        if(m_iniBedPos[jingluo] > m_bedCtrl->m_move_threhold_tail)
            m_iniBedPos[jingluo] = m_bedCtrl->m_move_threhold_tail;
        if(m_iniBedPos[jingluo] < -m_bedCtrl->m_move_threhold_head)
            m_iniBedPos[jingluo] = -m_bedCtrl->m_move_threhold_head;
    }
    if(g_cureVersion == 0 && m_iniBedPos.contains("pangguang1"))
            m_iniBedPos["pangguang1"] = m_bedCtrl->m_move_threhold_tail;
}

void CoreProcedureWorker::insertPixels(QMap<QString, QVector<QStringList>> &curelist, QMap<QString, IdentifyPointInfo> &acupoints)
{
    for(int i = 0; i < curelist.keys().size(); i++)
    {
        auto jingluo = curelist.keys().at(i);
        int index = 0;
        while(index < curelist[jingluo][0].size() - 1)
        {
            if(acupoints[curelist[jingluo][0][index]].camid == acupoints[curelist[jingluo][0][index+1]].camid &&
                    acupoints[curelist[jingluo][1][index]].camid == acupoints[curelist[jingluo][1][index+1]].camid)//只在同相机的2点间插点
            {
                int insertnum = 0;
                if(g_CureInsertsConfig->contains(curelist[jingluo][0][index]))
                    insertnum = g_CureInsertsConfig->value(curelist[jingluo][0][index]).toInt();
                if(insertnum > 0)
                {
                    for(int j = 0; j < 2; j++)
                    {
                        auto acuname = curelist[jingluo][j][index];
                        auto acuname1 = curelist[jingluo][j][index+1];
                        int camid = acupoints[acuname].camid;
                        double pix_x1 = acupoints[acuname].pix_x;
                        double pix_y1 = acupoints[acuname].pix_y;
                        double pix_x2 = acupoints[acuname1].pix_x;
                        double pix_y2 = acupoints[acuname1].pix_y;
                        double xstep = (pix_x2 - pix_x1) / (insertnum + 1);
                        double ystep = (pix_y2 - pix_y1) / (insertnum + 1);
                        for(int i = 1; i <= insertnum; i++)
                        {
                            IdentifyPointInfo tmpinfo;
                            tmpinfo.pix_x = pix_x1 + xstep * i;
                            tmpinfo.pix_y = pix_y1 + ystep * i;
                            tmpinfo.camid = camid;
                            tmpinfo.acuname = acuname + "-" + QString::number(i);
                            acupoints.insert(tmpinfo.acuname, tmpinfo);
                            curelist[jingluo][j].insert(index + i, tmpinfo.acuname);
                        }
                    }
                    index += insertnum + 1;
                }
                else
                    index++;
            }
            else
                index++;
        }
    }
}

bool CoreProcedureWorker::isRbPtValid(int rbindex, double x, double y, double z)
{
    auto tmpx = (rbindex == 0) ? x : -x;
    bool chk_x = (tmpx > -(m_bedCtrl->m_move_threhold_tail + ROBOTX_THREHOLD_HEAD) &&
                  tmpx < m_bedCtrl->m_move_threhold_head + ROBOTX_THREHOLD_TAIL);
    bool chk_y = (y > -0.85 && y < -0.27);
    bool chk_z = (z > -0.2 && z < 0.4);
    return chk_x && chk_y && chk_z;
}

void CoreProcedureWorker::saveIdentifyRawImgs(QVector<QSharedPointer<quint8>> rgbs, QVector<QSharedPointer<quint8>> depths, bool repredicting)
{
    QString zishi = "up_wai";
    if(m_curPosture == "1号姿势")
        zishi = "up_wai";
    else if(m_curPosture == "2号姿势")
        zishi = "up_nei";
    else if(m_curPosture == "3号姿势")
        zishi = "down_wai";
    else if(m_curPosture == "4号姿势")
        zishi = "down_nei";
    QString subdir;
    if(g_usingImg)
        subdir = "IdentifyRecords_UsingImg";
    else
        subdir = "IdentifyRecords";
    QString path = g_saveImgPath + QString("saveimgs/") + subdir + "/" + zishi + "/" + (repredicting?"dynamic/":"static/");
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMddHHmmsszzz");
    for(int i = 0; i < rgbs.size(); i++)
    {
        QString shead;
        if(g_cameraOrder[i] == 'l')
            shead = "left";
        else if(g_cameraOrder[i] == 'm')
            shead = "middle";
        else if(g_cameraOrder[i] == 'r')
            shead = "right";
        else
            shead = "head";
        CameraControl::savePic(path + "/color/", shead + "_" + zishi + "_" + timestamp + ".jpg", rgbs[i], CV_8UC3);
        CameraControl::savePic(path + "/depth/", shead + "_" + zishi + "_" + timestamp + ".png", depths[i], CV_16UC1);
        LOG_INFO << "保存color:" << path + "/color/" + shead + "_" + zishi + "_" + timestamp + ".jpg";
        LOG_INFO << "保存depth:" << path + "/depth/" + shead + "_" + zishi + "_" + timestamp + ".png";
    }
}

QMap<QChar, cv::Mat> CoreProcedureWorker::drawIdentifyResult(QMap<QString, QVector<QStringList>> &curelist, QMap<QString, IdentifyPointInfo> &acupoints, std::map<std::string, std::vector<double>> &boxes, QVector<QSharedPointer<quint8>> rgbs, bool coverup)
{
    QMap<QChar, cv::Mat> drawImgs;
    std::vector<cv::Scalar> palette = {{0,128,0}, {230,230,0}, {0,0,0}, {0,102,0}, {0,51,0}, {102,178,0}, {51,153,0}, {0,153,153},
                                       {0,102,102}, {0,51,51}, {153,0,153}, {102,0,102}, {51,255,51}, {0,255,0}, {0,0,255}};
    for(int i = 0; i < rgbs.size(); i++)
    {
        cv::Mat mat(IMAGE_HEIGHT, IMAGE_WIDTH, CV_8UC3, rgbs[i].data());
        drawImgs.insert(g_cameraOrder.at(i), mat.clone());
        auto key = QString(g_cameraOrder[i]).toStdString();
        if(boxes.count(key) != 0)
            cv::rectangle(drawImgs[g_cameraOrder[i]], cv::Point2f(boxes[key][0], boxes[key][1]), cv::Point2f(boxes[key][2], boxes[key][3]), cv::Scalar({0, 150, 0}), 2);
    }
    if(curelist.empty() == false)
    {
        for(int i = 0; i < curelist.keys().size(); i++)
        {
            auto color = palette[i];
            auto key = curelist.keys().at(i);
            for(int k = 0; k < 2; k++)
            {
                int lastpt = -1;
                for(int j = 0; j < curelist[key][k].size(); j++)
                {
                    auto ptinfo = acupoints[curelist[key][k][j]];
                    if(coverup)
                    {
                        bool skip = false;
                        auto jsonarr = g_JingLuoCoverConfig->value("coverup").toArray();
                        for(int index = 0; index < jsonarr.size(); index++)
                        {
                            if(ptinfo.acuname == jsonarr[index].toString())
                            {
                                skip = true;
                                break;
                            }
                        }
                        if(skip)
                            continue;
                    }
                    cv::circle(drawImgs[g_cameraOrder.at(ptinfo.camid)], cv::Point2f(ptinfo.pix_x, ptinfo.pix_y), 2, {0, 0, 255}, -1);
                    if(lastpt >= 0 && acupoints[curelist[key][k][lastpt]].camid == ptinfo.camid)
                    {
                        auto lastptinfo = acupoints[curelist[key][k][lastpt]];
                        cv::line(drawImgs[g_cameraOrder.at(ptinfo.camid)], cv::Point2f(ptinfo.pix_x, ptinfo.pix_y),
                                cv::Point2f(lastptinfo.pix_x, lastptinfo.pix_y), color, 1);
                    }
                    lastpt = j;
                }
            }
        }
    }
    else
    {
        for(int i = 0; i < acupoints.keys().size(); i++)
        {
            auto key = acupoints.keys().at(i);
            cv::circle(drawImgs[g_cameraOrder.at(acupoints[key].camid)], cv::Point2f(acupoints[key].pix_x, acupoints[key].pix_y), 2, {0, 0, 255}, -1);
        }
    }
    return drawImgs;
}

void CoreProcedureWorker::identifyIn2D(QString posture, QString jlargs)
{
//    QThread::sleep(45);
    m_curPosture = posture;
    //重置UI显示
    QPixmap tmpImg(400, 300);
    tmpImg.fill(QColor(Qt::darkGray));
    tmpImg.save("cache/camera_identify_m.jpg");
    tmpImg.save("cache/camera_identify_l.jpg");
    tmpImg.save("cache/camera_identify_r.jpg");
    tmpImg.save("cache/camera_identify_h.jpg");
    //检测是否已标定相机
    for(int i = 0; i < g_cameraNum; i++)
    {
        if(!m_camobjs[i]->checkExtrinsics())
        {
            emit identifyFinished(-5);
            return;
        }
    }
    //机械臂初始化
    for(int i = 0; i < m_robots.size(); i++)
        m_robots[i]->freeDrive(false);
    if(!m_robots[0]->isInPhotoPos() || !m_robots[1]->isInPhotoPos())
    {
        bool result = false;
        bool result1 = false;
        m_robots[0]->safePos(&result);
        if(!result)
        {
            emit identifyFinished(-1);
            return;
        }
        m_robots[1]->safePos(&result);
        if(!result)
        {
            emit identifyFinished(-1);
            return;
        }
        std::thread t1(&RobotControl::photoPos, m_robots[0], &result);
        std::thread t2(&RobotControl::photoPos, m_robots[1], &result1);
        t1.join();
        t2.join();
        if(!result || !result1)
        {
            emit identifyFinished(-1);
            return;
        }
    }
    //调理床初始化
    m_bedCtrl->moveTo(g_testIniBedPos, 1);
    QThread::sleep(1);
    //获取经络名
    QStringList jingluoList = jlargs.split(",");
    jingluoList.removeAt(jingluoList.size() - 1);
    //生成经络点列表
    auto curelist = genAcunameMap(jingluoList);
    QStringList acunames;
    for(int i = 0; i < jingluoList.size(); i++)
    {
        acunames.append(curelist[jingluoList[i]][0]);//左臂
        acunames.append(curelist[jingluoList[i]][1]);//右臂
    }
    LOG_INFO << "开始识别：" << posture << jlargs;
    //获取相机数据
    QVector<QSharedPointer<quint8>> rgbs;
    QVector<QSharedPointer<quint8>> depths;
    for(int i = 0; i < g_cameraNum; i++)
    {
        rgbs.append(m_camobjs[i]->getBGRData());
        depths.append(m_camobjs[i]->getDepthData());
    }
    if(rgbs.size() != g_cameraNum || depths.size() != g_cameraNum)
    {
        emit infromresetCams();
        LOG_EXCEPTION_AI << "identify fail:" << "cam stream error";
        emit identifyFinished(-1);
        return;
    }
    auto start_t = QDateTime::currentMSecsSinceEpoch();
    //保存识别原图
    saveIdentifyRawImgs(rgbs, depths);
    //调用ai接口获取2d坐标
    QMap<QString, IdentifyPointInfo> acupoints;
    std::map<string, std::vector<double> > boxes;
    auto result = predictIn2D(acunames, rgbs, 0, acupoints, boxes);
    if(result != 0)
    {
        if(result == -2)
        {
            if(posture == "1号姿势" || posture == "2号姿势")
                emit identifyFinished(-2);
            else
                emit identifyFinished(-3);
        }
        else
            emit identifyFinished(-1);
        return;
    }
    //保存并显示识别结果图片
    auto svimgs = drawIdentifyResult(curelist, acupoints, boxes, rgbs, false);
    QString tmpPth = QString("saveimgs/IdentifyResults/") + QDateTime::currentDateTime().toString("yyyyMM") + "/";
    QDir dir(tmpPth);
    if(!dir.exists())
    {
        QProcess cmd;
        cmd.start("mkdir", {"-p", tmpPth});
        cmd.waitForFinished();
    }
    QString tmpts = QDateTime::currentDateTime().toString("yyyyMMddHHmmsszzz");
    for(int i = 0; i < svimgs.keys().size(); i++)
    {
        auto key = svimgs.keys().at(i);
        auto fname = tmpPth + key + "_" + tmpts + ".jpg";
        if(g_runArgs["EnableEncryption"] == "true")
            CameraControl::saveEncodeMat(fname, svimgs[key]);
        else
            cv::imwrite(fname.toStdString(), svimgs[key]);
        LOG_INFO << "保存识别结果：" << fname;
    }
    auto showimgs = drawIdentifyResult(curelist, acupoints, boxes, rgbs, true);
    for(int i = 0; i < showimgs.keys().size(); i++)
    {
        auto key = showimgs.keys().at(i);
        cv::imwrite(QString(QString("cache/camera_identify_") + key + ".jpg").toStdString(), showimgs[key]);
    }
    //特殊像素坐标处理
    if(curelist.contains("gan1"))//膝盖边缘点下移
    {
        acupoints["L-gan-5"].pix_y -= 4;
        acupoints["R-gan-5"].pix_y += 4;
        acupoints["L-gan-6"].pix_y -= 4;
        acupoints["R-gan-6"].pix_y += 4;
    }
    if(curelist.contains("gan2"))//嘴、眼下的点下移
    {
        acupoints["L-gan-20"].pix_y += 7;
        acupoints["R-gan-20"].pix_y -= 7;
        acupoints["L-gan-28"].pix_y += 7;
        acupoints["R-gan-28"].pix_y -= 7;

        acupoints["L-gan-20"].pix_x -= 15;
        acupoints["R-gan-20"].pix_x -= 15;
        acupoints["L-gan-21"].pix_x -= 15;
        acupoints["R-gan-21"].pix_x -= 15;
        acupoints["L-gan-22"].pix_x -= 15;
        acupoints["R-gan-22"].pix_x -= 15;

        acupoints["L-gan-23"].pix_y += 15;
        acupoints["L-gan-24"].pix_y += 15;

        acupoints["R-gan-23"].pix_y -= 15;
        acupoints["R-gan-24"].pix_y -= 15;

        acupoints["L-gan-25"].pix_x += 10;
        acupoints["R-gan-25"].pix_x += 10;
    }
    if(curelist.contains("wei1"))//嘴、眼下的点下移
    {
        acupoints["L-wei-3"].pix_x -= 15;
        acupoints["R-wei-3"].pix_x -= 15;
        acupoints["L-wei-5"].pix_x -= 15;
        acupoints["R-wei-5"].pix_x -= 15;
        acupoints["L-wei-6"].pix_x -= 15;
        acupoints["R-wei-6"].pix_x -= 15;

        acupoints["L-wei-6"].pix_y += 7;
        acupoints["R-wei-6"].pix_y -= 7;
        acupoints["L-wei-11"].pix_y += 7;
        acupoints["R-wei-11"].pix_y -= 7;
        acupoints["L-wei-11"].pix_x -= 10;
        acupoints["R-wei-11"].pix_x -= 10;

        acupoints["L-wei-10"].pix_x -= 10;
        acupoints["R-wei-10"].pix_x -= 10;
    }
    if(curelist.contains("dan1"))//拉开左右视角点
    {
        acupoints["L-dan-18"].pix_y = acupoints["L-dan-17"].pix_y;
        acupoints["R-dan-18"].pix_y = acupoints["R-dan-17"].pix_y;
    }
    if(g_cureVersion == 0 && curelist.contains("sanjiao"))//耳廓边的点外移
    {
        acupoints["R-sanjiao-13"].pix_y -= 10;
        acupoints["L-sanjiao-13"].pix_y += 10;
        acupoints["R-sanjiao-14"].pix_y -= 10;
        acupoints["L-sanjiao-14"].pix_y += 10;
        acupoints["R-sanjiao-15"].pix_y -= 10;
        acupoints["L-sanjiao-15"].pix_y += 10;

        acupoints["R-sanjiao-16"].pix_x += 10;
        acupoints["L-sanjiao-16"].pix_x += 10;

        acupoints["R-sanjiao-17"].pix_y += 10;
        acupoints["L-sanjiao-17"].pix_y -= 10;
        acupoints["R-sanjiao-21"].pix_y += 10;
        acupoints["L-sanjiao-21"].pix_y -= 10;
        acupoints["R-sanjiao-22"].pix_y += 10;
        acupoints["L-sanjiao-22"].pix_y -= 10;
        acupoints["R-sanjiao-23"].pix_y += 10;
        acupoints["L-sanjiao-23"].pix_y -= 10;
        acupoints["R-sanjiao-24"].pix_y += 10;
        acupoints["L-sanjiao-24"].pix_y -= 10;
        acupoints["R-sanjiao-25"].pix_y += 10;
        acupoints["L-sanjiao-25"].pix_y -= 10;
    }
    if(curelist.contains("shen"))
    {
        acupoints["R-shen-13"].pix_y += 4;
        acupoints["L-shen-13"].pix_y -= 4;
    }
    if(g_cureVersion == 0 && curelist.contains("pangguang2"))
    {
        acupoints["R-pangguang-27"].pix_y += 10;
        acupoints["L-pangguang-27"].pix_y -= 10;
        acupoints["R-pangguang-28"].pix_x += 15;
        acupoints["L-pangguang-28"].pix_x += 15;
    }
    //按像素坐标插值
    insertPixels(curelist, acupoints);
    //配置经络点信息机械臂序号
    for(int i = 0; i < curelist.keys().size(); i++)
    {
        auto jingluo = curelist.keys().at(i);
        for(int j = 0; j < 2; j++)
            for(int k = 0; k < curelist[jingluo][j].size(); k++)
                acupoints[curelist[jingluo][j][k]].rbindex = j;
    }
    //计算机械臂坐标
    result = calRobotXYZ(depths, acupoints);
    if(result != 0)
    {
        emit identifyFinished(-1);
        return;
    }
    //特殊经络处理
    QStringList tmplist = {"ren", "du"};
    for(int i = 0; i < tmplist.size(); i++)
    {
        if(curelist.contains(tmplist[i]))
        {
            curelist[tmplist[i]][0].removeLast();
            curelist[tmplist[i]][1].removeFirst();
        }
    }
    if(curelist.contains("gan2"))
    {
        acupoints[curelist["gan2"][0][7]].robot_x = acupoints[curelist["gan2"][0][6]].robot_x;
        acupoints[curelist["gan2"][1][7]].robot_x = acupoints[curelist["gan2"][1][6]].robot_x;
    }
    //根据配置文件对机械臂坐标进行修正
    for(int i = 0; i < acupoints.keys().size(); i++)
    {
        auto key = acupoints.keys().at(i);
        if(g_CurePtsBiasConfig->contains(acupoints[key].acuname))
        {
            auto jsonarr = g_CurePtsBiasConfig->value(acupoints[key].acuname).toArray();
            acupoints[key].robot_x += jsonarr.at(0).toDouble();
            acupoints[key].robot_y += jsonarr.at(1).toDouble();
            acupoints[key].robot_z += jsonarr.at(2).toDouble();
        }
    }
    //校验机械臂坐标是否超限
    for(int i = 0; i < acupoints.keys().size(); i++)
    {
        auto key = acupoints.keys().at(i);
        if(!isRbPtValid(acupoints[key].rbindex, acupoints[key].robot_x, acupoints[key].robot_y, acupoints[key].robot_z))
        {
            LOG_EXCEPTION_AI << "identify fail:" << "robot position error" <<
                                acupoints[key].acuname << acupoints[key].pix_x << acupoints[key].pix_y << acupoints[key].cam_z
                                << acupoints[key].robot_x << acupoints[key].robot_y << acupoints[key].robot_z;
            emit identifyFinished(-4);
            return;
        }
    }
    //规划调理床初始位置
    calIniBedPos(curelist, acupoints);
    //建立open3d点云并预估法向
    auto st = QDateTime::currentMSecsSinceEpoch();
    std::vector<std::shared_ptr<open3d::geometry::PointCloud>> pcds;
    std::vector<std::shared_ptr<open3d::geometry::KDTreeFlann>> kdtrees;
    std::vector<std::shared_ptr<open3d::geometry::PointCloud>> downpcds;
    std::vector<std::shared_ptr<open3d::geometry::KDTreeFlann>> downkdtrees;
    int oxy[4][4] = {{0}};
    for(int i = 0; i < g_cameraOrder.size(); i++)
    {
        if(boxes[QString(g_cameraOrder[i]).toStdString()].size() == 0)
        {
            pcds.push_back(nullptr);
            kdtrees.push_back(nullptr);
            downpcds.push_back(nullptr);
            downkdtrees.push_back(nullptr);
            continue;
        }
        int x1 = boxes[QString(g_cameraOrder[i]).toStdString()][0];
        int y1 = boxes[QString(g_cameraOrder[i]).toStdString()][1];
        int x2 = boxes[QString(g_cameraOrder[i]).toStdString()][2];
        int y2 = boxes[QString(g_cameraOrder[i]).toStdString()][3];
        if(g_cameraOrder[i] == 'r')
        {
            x1 = boxes[QString(g_cameraOrder[i]).toStdString()][2];
            y1 = boxes[QString(g_cameraOrder[i]).toStdString()][1];
            x2 = boxes[QString(g_cameraOrder[i]).toStdString()][0];
            y2 = boxes[QString(g_cameraOrder[i]).toStdString()][3];
        }
        oxy[i][0] = x1;
        oxy[i][1] = y1;
        oxy[i][2] = x2;
        oxy[i][3] = y2;
        int pix_wid = x2 - x1;
        int pix_height = y2 - y1;
        std::vector<double> vec;
        for(int c = y1; c < y2; c++)
        {
            std::vector<double> tmpvec;
            tmpvec.assign((uint16_t*)(depths[i].data()) + c * 1280 + x1, (uint16_t*)(depths[i].data()) + c * 1280 + x2);
            vec.insert(vec.end(), tmpvec.begin(), tmpvec.end());
        }
        auto depth_array = nc::asarray<double>(vec, false);

        auto axis_x = nc::linspace<double>(x1, x2, pix_wid);
        auto axis_y = nc::linspace<double>(y1, y2, pix_height);
        auto xypair = nc::meshgrid<double>(axis_x, axis_y);

        auto X = xypair.first;
        auto Y = xypair.second;
        X.reshape({(uint32_t)(pix_wid*pix_height), 1});
        Y.reshape({(uint32_t)(pix_wid*pix_height), 1});
        depth_array.reshape({(uint32_t)(pix_wid*pix_height), 1});

        double fx = m_camobjs[i]->m_intrinsics_fx;
        double fy = m_camobjs[i]->m_intrinsics_fy;
        double cx = m_camobjs[i]->m_intrinsics_cx;
        double cy = m_camobjs[i]->m_intrinsics_cy;
        auto c1 = (X - cx) / fx * depth_array;
        auto c2 = (Y - cy) / fy * depth_array;
        auto c3 = depth_array;

        auto pcd = make_shared<open3d::geometry::PointCloud>();
        for(int j = 0; j < pix_wid*pix_height; j++)
            pcd->points_.push_back(Eigen::Vector3d({c1[j], c2[j], c3[j]}));

        Eigen::Matrix4d ex = Eigen::Map<Eigen::Matrix4d>(m_camobjs[i]->getExtrinsics(0).data(), 4, 4);
        pcd->Transform(ex.transpose());

        auto downpcd = pcd->VoxelDownSample(10);
        downpcd->EstimateNormals(open3d::geometry::KDTreeSearchParamHybrid(200, 100));

        //纠正反向向量
        if(g_cameraOrder[i] == 'm')
        {
            for(size_t j = 0; j < downpcd->normals_.size(); j++)
                if(downpcd->normals_[j][2] < 0)
                    downpcd->normals_[j] = -downpcd->normals_[j];
        }
        else if(g_cameraOrder[i] == 'l')
        {
            for(size_t j = 0; j < downpcd->normals_.size(); j++)
                if(downpcd->normals_[j][1] + downpcd->normals_[j][2] < 0)
                    downpcd->normals_[j] = -downpcd->normals_[j];
        }
        else if(g_cameraOrder[i] == 'r')
        {
            for(size_t j = 0; j < downpcd->normals_.size(); j++)
                if(downpcd->normals_[j][2] - downpcd->normals_[j][1] < 0)
                    downpcd->normals_[j] = -downpcd->normals_[j];
        }
        else
        {
            for(size_t j = 0; j < downpcd->normals_.size(); j++)
                if(downpcd->normals_[j][2] - downpcd->normals_[j][0] < 0)
                    downpcd->normals_[j] = -downpcd->normals_[j];
        }

        auto tree = make_shared<open3d::geometry::KDTreeFlann>(*pcd);
        auto downtree = make_shared<open3d::geometry::KDTreeFlann>(*downpcd);
        kdtrees.push_back(tree);
        downkdtrees.push_back(downtree);
        downpcds.push_back(downpcd);
        pcds.push_back(pcd);
    }
    LOG_INFO << "open3d pcd build cost:" << QDateTime::currentMSecsSinceEpoch() - st;
    //规划力控法向量
    auto getxyzFromPcds = [=](int camid, int pix_x, int pix_y)->Eigen::Vector3d
    {
        auto pcd_width = oxy[camid][2] - oxy[camid][0];
        return pcds[camid]->points_[(pix_y - oxy[camid][1]) * pcd_width + pix_x - oxy[camid][0]];
    };
    st = QDateTime::currentMSecsSinceEpoch();
    vector<int> indices;
    vector<double> nouse;
    vector<vector<vector<size_t>>> pcdindexs;
    for(int i = 0; i < curelist.keys().size(); i++)
    {
        auto jingluo = curelist.keys().at(i);
        pcdindexs.push_back({{}, {}, {}, {}});
        for(int k = 0; k < curelist[jingluo][0].size() - 1; k++)
        {
            if(g_CureForceOverrideConfig->contains(curelist[jingluo][0][k]) ||
                    g_CureForceOverrideConfig->contains(curelist[jingluo][1][k]))//已被预设力控的点跳过
                continue;
            QVector<IdentifyPointInfo> p1;
            QVector<IdentifyPointInfo> p2;
            p1.append(acupoints[curelist[jingluo][0][k]]);
            p1.append(acupoints[curelist[jingluo][1][k]]);
            p2.append(acupoints[curelist[jingluo][0][k+1]]);
            p2.append(acupoints[curelist[jingluo][1][k+1]]);
            double src_pix_x[2];
            double src_pix_y[2];
            double xstep[2];
            double ystep[2];
            int max_step = 1280;
            int ccamid[2];
            std::vector<int> insert_steps;
            //生成经络点法向量；计算插值点步长
            for(int j = 0; j < 2; j++)
            {
                //搜索p1法向量
                auto pxyz_p1 = getxyzFromPcds(p1[j].camid, p1[j].pix_x, p1[j].pix_y);
                downkdtrees[p1[j].camid]->SearchKNN<Eigen::Vector3d>(pxyz_p1, 1, indices, nouse);
                auto curDir = downpcds[p1[j].camid]->normals_[indices[0]];
                acupoints[curelist[jingluo][j][k]].force_x = (j == 0 ? -curDir[0] : curDir[0]);
                acupoints[curelist[jingluo][j][k]].force_y = (j == 0 ? -curDir[1] : curDir[1]);
                acupoints[curelist[jingluo][j][k]].force_z = -curDir[2];
                pcdindexs[i][p1[j].camid].push_back(indices[0]);
                //判断后续插值
                if(jingluo == "ren" || jingluo == "du")
                    continue;
                src_pix_x[j] = p1[j].pix_x;
                src_pix_y[j] = p1[j].pix_y;
                auto tar_pix_x = p2[j].pix_x;
                auto tar_pix_y = p2[j].pix_y;
                ccamid[j] = p1[j].camid;
                if(p1[j].camid != p2[j].camid)
                {
                    if(!(p1[j].camid == g_cameraOrder.indexOf('m') || p2[j].camid == g_cameraOrder.indexOf('m')))
                        ccamid[j] = p1[j].camid;
                    else
                        ccamid[j] = g_cameraOrder.indexOf('m');
                    if(p1[j].camid == ccamid[j])
                    {
                        auto pxyz = getxyzFromPcds(p2[j].camid, tar_pix_x, tar_pix_y);
                        kdtrees[ccamid[j]]->SearchKNN<Eigen::Vector3d>(pxyz, 1, indices, nouse);
                        auto wid = oxy[ccamid[j]][2] - oxy[ccamid[j]][0];
                        tar_pix_y = (int)(indices[0] / wid) + oxy[ccamid[j]][1];
                        tar_pix_x = indices[0] % wid + oxy[ccamid[j]][0];
                    }
                    else
                    {
                        auto pxyz = getxyzFromPcds(p1[j].camid, src_pix_x[j], src_pix_y[j]);
                        kdtrees[ccamid[j]]->SearchKNN<Eigen::Vector3d>(pxyz, 1, indices, nouse);
                        auto wid = oxy[ccamid[j]][2] - oxy[ccamid[j]][0];
                        src_pix_y[j] = (int)(indices[0] / wid) + oxy[ccamid[j]][1];
                        src_pix_x[j] = indices[0] % wid + oxy[ccamid[j]][0];
                    }
                }
                //计算迭代步长
                int num = qSqrt(qPow(src_pix_x[j] - tar_pix_x, 2) + qPow(src_pix_y[j] - tar_pix_y, 2));
                if(max_step > num - 1)
                    max_step = num - 1;
                xstep[j] = (tar_pix_x - src_pix_x[j]) / num;
                ystep[j] = (tar_pix_y - src_pix_y[j]) / num;
                //迭代判断路径上是否需根据法向变化插值
                double pix_x_iter = src_pix_x[j] + xstep[j];
                double pix_y_iter = src_pix_y[j] + ystep[j];
                for(int iter_step = 1; iter_step < num; iter_step++)
                {
                    auto pxyz = getxyzFromPcds(ccamid[j], pix_x_iter, pix_y_iter);
                    downkdtrees[ccamid[j]]->SearchKNN<Eigen::Vector3d>(pxyz, 1, indices, nouse);
                    auto pDir = downpcds[ccamid[j]]->normals_[indices[0]];
                    double cos = curDir.dot(pDir) / (curDir.norm() * pDir.norm());
                    auto deg = qAcos(cos) * 180 / PI;
//                    pcdindexs[i][ccamid].push_back(indices[0]);
                    if(qAbs(deg) > 15)
                    {
                        curDir = pDir;
                        insert_steps.push_back(iter_step);
                    }
                    pix_x_iter += xstep[j];
                    pix_y_iter += ystep[j];
                }
            }
            //插入插值点
            std::sort(insert_steps.begin(), insert_steps.end());
            for(int j = 0; j < 2; j++)
            {
                int index = 1;
                for(int m = 0; m < insert_steps.size(); m++)
                {
                    if(insert_steps[m] > max_step)
                        break;
                    if(m > 0 && insert_steps[m] == insert_steps[m-1])
                        continue;
                    double pix_x_iter = src_pix_x[j] + xstep[j] * insert_steps[m];
                    double pix_y_iter = src_pix_y[j] + ystep[j] * insert_steps[m];
                    auto pxyz = getxyzFromPcds(ccamid[j], pix_x_iter, pix_y_iter);
                    downkdtrees[ccamid[j]]->SearchKNN<Eigen::Vector3d>(pxyz, 1, indices, nouse);
                    auto pDir = downpcds[ccamid[j]]->normals_[indices[0]];
                    IdentifyPointInfo info;
                    info.acuname = jingluo + "-" + p1[j].acuname + "-v-" + QString::number(index);
                    info.pix_x = pix_x_iter;
                    info.pix_y = pix_y_iter;
                    info.camid = ccamid[j];
                    info.rbindex = p1[j].rbindex;
                    info.force_x = (j == 0 ? -pDir[0] : pDir[0]);
                    info.force_y = (j == 0 ? -pDir[1] : pDir[1]);
                    info.force_z = -pDir[2];
                    info.cam_z = CameraControl::getDepthValue(pix_x_iter, pix_y_iter, depths[ccamid[j]]);
                    m_camobjs[ccamid[j]]->cam2dToCam3d(pix_x_iter, pix_y_iter, info.cam_z, info.cam_x, info.cam_y);
                    QVector<double> cords = {info.cam_x, info.cam_y, info.cam_z};
                    m_camobjs[ccamid[j]]->cam3dToRobot3d(p1[j].rbindex, cords[0], cords[1], cords[2]);
                    m_camobjs[ccamid[j]]->caliFix(p1[j].rbindex, cords[0], cords[1], cords[2]);
                    cords = caliCordsByBed(info.rbindex, -g_testIniBedPos, cords);
                    info.robot_x = cords[0];
                    info.robot_y = cords[1];
                    info.robot_z = cords[2];
                    if(isRbPtValid(info.rbindex, info.robot_x, info.robot_y, info.robot_z))
                    {
                        curelist[jingluo][j].insert(k + index, info.acuname);
                        acupoints.insert(info.acuname, info);
                        pcdindexs[i][ccamid[j]].push_back(indices[0]);
                        index++;
                        LOG_INFO << "open3d insert point:" << info.acuname << info.pix_x << info.pix_y << info.cam_z
                                 << info.robot_x << info.robot_y << info.robot_z;
                    }
                    else
                        LOG_EXCEPTION_CAM << "open3d insert point depth corrupted:" << info.acuname << info.pix_x << info.pix_y << info.cam_z
                                             << info.robot_x << info.robot_y << info.robot_z;
                }
                if(j == 1)
                    k += index - 1;
            }
        }
    }
    LOG_INFO << "cal force dir cost:" << QDateTime::currentMSecsSinceEpoch() - st;

//    LOG_INFO << acupoints.keys();

//    for(int i = 0; i < curelist.keys().size(); i++)
//    {
//        auto jingluo = curelist.keys().at(i);
//        for(int j = 0; j < pcdindexs[i].size(); j++)
//        {
//            if(pcdindexs[i][j].size() > 0)
//            {
//                auto subpcd = downpcds[j]->SelectByIndex(pcdindexs[i][j]);
//                open3d::io::WritePointCloud(QString(jingluo + "_" + QString::number(j) + ".pcd").toStdString(), *subpcd);
//            }
//        }
//    }
//    open3d::io::WritePointCloud("0.pcd", *(pcds[0]));
//    open3d::io::WritePointCloud("1.pcd", *(pcds[1]));
//    open3d::io::WritePointCloud("2.pcd", *(pcds[2]));
//    open3d::io::WritePointCloud("3.pcd", *(pcds[3]));
//    open3d::io::WritePointCloud("d0.pcd", *(downpcds[0]));
//    open3d::io::WritePointCloud("d1.pcd", *(downpcds[1]));
//    open3d::io::WritePointCloud("d2.pcd", *(downpcds[2]));
//    open3d::io::WritePointCloud("d3.pcd", *(downpcds[3]));

    LOG_INFO << "total identify time cost:" << QDateTime::currentMSecsSinceEpoch() - start_t;
    //保存识别结果
    m_cureList = curelist;
    m_acupoints = acupoints;
    emit identifyFinished(0);

//    LOG_INFO << acupoints;
}

bool CoreProcedureWorker::repredictIn2D(QStringList acunames)
{
    LOG_INFO << "开始二次识别";
    //获取相机数据
    QVector<QSharedPointer<quint8>> rgbs;
    QVector<QSharedPointer<quint8>> depths;
    for(int i = 0; i < g_cameraNum; i++)
    {
        rgbs.append(m_camobjs[i]->getBGRData());
        depths.append(m_camobjs[i]->getDepthData());
    }
    if(rgbs.size() != g_cameraNum || depths.size() != g_cameraNum)
    {
        emit infromresetCams();
        LOG_EXCEPTION_AI << "repredictIn2D fail:" << "cam stream error";
        emit identifyFinished(-1);
        return false;
    }
    //保存识别原图
    saveIdentifyRawImgs(rgbs, depths, true);
    //调用ai接口获取2d坐标
    QMap<QString, IdentifyPointInfo> acupoints;
    std::map<string, std::vector<double> > boxes;
    auto result = predictIn2D(acunames, rgbs, 0, acupoints, boxes);
    if(result != 0)
    {
        LOG_EXCEPTION_AI << "repredictIn2D fail";
        return false;
    }
    //保存二次识别结果
    QMap<QString, QVector<QStringList>> curelist;
    auto svimgs = drawIdentifyResult(curelist, acupoints, boxes, rgbs, false);
    QString tmpPth = QString("saveimgs/IdentifyResults/") + QDateTime::currentDateTime().toString("yyyyMM") + "/";
    QDir dir(tmpPth);
    if(!dir.exists())
    {
        QProcess cmd;
        cmd.start("mkdir", {"-p", tmpPth});
        cmd.waitForFinished();
    }
    QString tmpts = QDateTime::currentDateTime().toString("yyyyMMddHHmmsszzz");
    for(int i = 0; i < svimgs.keys().size(); i++)
    {
        auto key = svimgs.keys().at(i);
        auto fname = tmpPth + key + "_" + tmpts + ".jpg";
        if(g_runArgs["EnableEncryption"] == "true")
            CameraControl::saveEncodeMat(fname, svimgs[key]);
        else
            cv::imwrite(fname.toStdString(), svimgs[key]);
        LOG_INFO << "保存二次识别结果：" << fname;
    }
    //计算机械臂坐标
    result = calRobotXYZ(depths, acupoints);
    if(result != 0)
    {
        LOG_EXCEPTION_AI << "repredictIn2D fail";
        return false;
    }
    //校验机械臂坐标是否违规
    for(int i = 0; i < acupoints.keys().size(); i++)
    {
        auto key = acupoints.keys().at(i);
        if(!isRbPtValid(acupoints[key].rbindex, acupoints[key].robot_x, acupoints[key].robot_y, acupoints[key].robot_z))
        {
            LOG_EXCEPTION_AI << "repredictIn2D fail:" << "robot position error" <<
                                acupoints[key].acuname << acupoints[key].pix_x << acupoints[key].pix_y << acupoints[key].cam_z
                                << acupoints[key].robot_x << acupoints[key].robot_y << acupoints[key].robot_z;
            return false;
        }
    }
    //打印机械臂坐标点
    QString outpts = "二次识别机械臂坐标 ";
    for(int i = 0; i < acupoints.keys().size(); i++)
    {
        auto key = acupoints.keys().at(i);
        outpts += acupoints[key].acuname + ":\t";
        outpts += QString("[");
        outpts += QString::number(acupoints[key].robot_x, 'g', 3) + ", ";
        outpts += QString::number(acupoints[key].robot_y, 'g', 3) + ", ";
        outpts += QString::number(acupoints[key].robot_z, 'g', 3) + "]\n";
    }
    LOG_INFO << outpts;
    //识别结果覆盖首次识别
    for(int i = 0; i < acupoints.keys().size(); i++)
    {
        auto key = acupoints.keys().at(i);
        if(m_acupoints.contains(key))
        {
            m_acupoints[key].robot_x = acupoints[key].robot_x;
            m_acupoints[key].robot_y = acupoints[key].robot_y;
            m_acupoints[key].robot_z = acupoints[key].robot_z;
        }
    }
    return true;
}

//TODO 3D接口待后续完善
void CoreProcedureWorker::identifyIn3D(QString posture, QString jlargs)
{

}

void CoreProcedureWorker::identifyJingLuo(QString posture, QString jlargs)
{
    if(m_using3D)
        identifyIn3D(posture, jlargs);
    else
        identifyIn2D(posture, jlargs);
}

void CoreProcedureWorker::calculatepts(IdentifyPointInfo &pt, double bed_bias)
{
    double x = pt.cam_x;
    double y = pt.cam_y;
    double z = pt.cam_z;
    m_camobjs[pt.camid]->cam3dToRobot3d(pt.rbindex, x, y, z);
    m_camobjs[pt.camid]->caliFix(pt.rbindex, x, y, z);
    pt.robot_x = x;
    pt.robot_y = y;
    if(g_usingImg)
        pt.robot_z = 0;
    else
        pt.robot_z = z;
    //剔除床的偏移量，机械臂位置都按床在0算 如果是图片识别需把床初始位置设置为跟图片当时采集位置一致!
    auto cords = caliCordsByBed(pt.rbindex, -bed_bias, QVector<double>({pt.robot_x, pt.robot_y, pt.robot_z}));
    pt.robot_x = cords[0];
    pt.robot_y = cords[1];
    pt.robot_z = cords[2];
}

void CoreProcedureWorker::startCure(QString meridian)
{
    auto parallel = [=](void (RobotControl::*func)(bool*), bool *r1, bool *r2){
        std::thread t1(func, m_robots[0], r1);
        std::thread t2(func, m_robots[1], r2);
        t1.join();
        t2.join();
    };
    if(m_stopTreatment)
        return;
    //禁用自由驱动
    m_robots[0]->blockFreeDrive(true);
    m_robots[1]->blockFreeDrive(true);
    m_isTreating = true;
    //开始调理期间图片保存
    if(g_runArgs["SaveCureImg"] == "true")
    {
        QMap<QString, QString> postureList = {{"1号姿势", "up_wai"}, {"2号姿势", "up_nei"}, {"3号姿势", "down_wai"}, {"4号姿势", "down_nei"}};
        for(int i = 0; i < m_camobjs.size(); i++)
            m_camobjs[i]->setCurePosture(postureList[m_curPosture]);
    }
    //获取识别结果
    auto curePts = m_cureList[meridian];
    //打印所有调理坐标点
    QString outpts = "";
    for(int i = 0; i < curePts.size(); i++)
    {
        if(i == 0)
            outpts += "left:\n";
        else
            outpts += "right:\n";
        for(int j = 0; j < curePts[i].size(); j++)
        {
            outpts += curePts[i][j] + ":\t";
            outpts += QString("[");
            outpts += QString::number(m_acupoints[curePts[i][j]].robot_x, 'g', 3) + ", ";
            outpts += QString::number(m_acupoints[curePts[i][j]].robot_y, 'g', 3) + ", ";
            outpts += QString::number(m_acupoints[curePts[i][j]].robot_z, 'g', 3) + "]\t[";
            outpts += QString::number(m_acupoints[curePts[i][j]].cam_x, 'f', 1) + ", ";
            outpts += QString::number(m_acupoints[curePts[i][j]].cam_y, 'f', 1) + ", ";
            outpts += QString::number(m_acupoints[curePts[i][j]].cam_z, 'f', 1);
            outpts += "]\n";
        }
    }
    LOG_INFO << outpts;
    //初始化机械臂位置
    for(int i = 0; i < m_robots.size(); i++)
        m_robots[i]->freeDrive(false);
    bool result = false;
    bool result1 = false;
    if(m_robots[0]->isInPhotoPos() && m_robots[1]->isInPhotoPos())
    {
        parallel(&RobotControl::safePos, &result, &result1);
        if(!result || !result1)
        {
            errquit("safe pos error", "移动到安全位置错误, 请尝试重新调理");
            return;
        }
    }
    else
    {
        m_robots[0]->safePos(&result);
        if(!result)
        {
            errquit("safe pos error", "移动到安全位置错误, 请尝试重新调理");
            return;
        }
        m_robots[1]->safePos(&result);
        if(!result)
        {
            errquit("safe pos error", "移动到安全位置错误, 请尝试重新调理");
            return;
        }
    }
    for(int i = 0; i < m_robots.size(); i++)
        m_robots[i]->vibrateOff();
    QThread::msleep(1000);//等待完全静止
    //力控清零
    for(int i = 0; i < m_robots.size(); i++)
        m_robots[i]->clearforce();
    QThread::msleep(1000);//力控清零等待1s
    //开启力控软保护
    for(int i = 0; i < m_robots.size(); i++)
        m_robots[i]->setRTSoftProtectiveStopFlag(true);
    //判断是否有中止调理再等待2s
    if(m_stopTreatment)
    {
        normalquit();
        return;
    }
    //初始化床位置
    if(m_iniBedPos.contains(meridian))
        m_bedPos = m_iniBedPos[meridian];
    else
    {
        errquit(QString("ini bed pos error: ") + meridian, "床初始位置计算错误");
        return;
    }
    if(!m_bedCtrl->moveTo(m_bedPos, 1))
    {
        errquit(QString("ini bed pos failed: ") + QString::number(m_bedPos), "初始化床位置失败，请尝试重新识别调理");
        return;
    }
    //默认力控
    msg_force_startforce force_l;
    force_l.force_value[2] = 3;
    msg_force_startforce force_r;
    force_r.force_value[2] = 3;
    //机械臂姿态 旋转向量
    QVector<double> pose_l;
    QVector<double> pose_r;
    auto last_pt_l = caliCordsByBed(0, m_bedPos, QVector<double>({m_acupoints[curePts[0][0]].robot_x, m_acupoints[curePts[0][0]].robot_y,
                                                     m_acupoints[curePts[0][0]].robot_z}));
    auto last_pt_r = caliCordsByBed(1, m_bedPos, QVector<double>({m_acupoints[curePts[1][0]].robot_x, m_acupoints[curePts[1][0]].robot_y,
                                                     m_acupoints[curePts[1][0]].robot_z}));
    //设置阻尼增益
    m_robots[0]->setGain(DEFAULT_CURE_GAIN);
    m_robots[0]->setDamp(DEFAULT_CURE_DAMP);
    m_robots[1]->setGain(DEFAULT_CURE_GAIN);
    m_robots[1]->setDamp(DEFAULT_CURE_DAMP);
    //移动至第一个点
    QVector<double> contact_dir_l = {0, 0, -0.05, 0, 0, 0};
    QVector<double> contact_dir_r = {0, 0, -0.05, 0, 0, 0};
    auto tmp_cfgptr = (g_cureVersion == 0 ? g_CureContactConfig : g_CureContactConfig_Body);
    if(tmp_cfgptr->value("forward").toArray().contains(meridian))
    {
        m_acupoints[curePts[0][0]].robot_x -= 0.08;
        m_acupoints[curePts[1][0]].robot_x += 0.08;
        contact_dir_l = {0.05, 0, 0, 0, 0, 0};
        contact_dir_r = {-0.05, 0, 0, 0, 0, 0};
    }
    else if(tmp_cfgptr->value("backward").toArray().contains(meridian))
    {
        m_acupoints[curePts[0][0]].robot_x += 0.08;
        m_acupoints[curePts[1][0]].robot_x -= 0.08;
        contact_dir_l = {-0.05, 0, 0, 0, 0, 0};
        contact_dir_r = {0.05, 0, 0, 0, 0, 0};
    }
    else if(tmp_cfgptr->value("inside").toArray().contains(meridian))
    {
        m_acupoints[curePts[0][0]].robot_y += 0.08;
        m_acupoints[curePts[1][0]].robot_y += 0.08;
        contact_dir_l = {0, -0.05, 0, 0, 0, 0};
        contact_dir_r = {0, -0.05, 0, 0, 0, 0};
    }
    else if(tmp_cfgptr->value("outside").toArray().contains(meridian))
    {
        m_acupoints[curePts[0][0]].robot_y -= 0.08;
        m_acupoints[curePts[1][0]].robot_y -= 0.08;
        contact_dir_l = {0, 0.05, 0, 0, 0, 0};
        contact_dir_r = {0, 0.05, 0, 0, 0, 0};
    }
    else
    {
        double lz = m_acupoints[curePts[0][0]].robot_z + 0.08;
        double rz = m_acupoints[curePts[1][0]].robot_z + 0.08;
        double mz = (lz > rz ? lz : rz);//下落方向左右臂对齐
        if(mz < 0.2)
            mz = 0.2;
        m_acupoints[curePts[0][0]].robot_z = mz;
        m_acupoints[curePts[1][0]].robot_z = mz;
    }
    auto fpl = caliCordsByBed(0, m_bedPos, QVector<double>({m_acupoints[curePts[0][0]].robot_x, m_acupoints[curePts[0][0]].robot_y,
                                                     m_acupoints[curePts[0][0]].robot_z}));
    auto fpr = caliCordsByBed(1, m_bedPos, QVector<double>({m_acupoints[curePts[1][0]].robot_x, m_acupoints[curePts[1][0]].robot_y,
                                                     m_acupoints[curePts[1][0]].robot_z}));
    if(g_CurePoseConfig->contains(curePts[0][0]))//按配置文件
    {
        auto arr = g_CurePoseConfig->value(curePts[0][0]).toArray();
        QVector<double> degs;
        for(int i = 0; i < arr.size(); i++)
            degs.append(arr[i].toDouble() * PI / 180);
        pose_l.append(RobotControl::eulerAngleToRotationVec(degs));
    }
    else
        pose_l = getPoseByXY(0, fpl[0], fpl[1]);//按坐标
    if(g_CurePoseConfig->contains(curePts[1][0]))//按配置文件
    {
        auto arr = g_CurePoseConfig->value(curePts[1][0]).toArray();
        QVector<double> degs;
        for(int i = 0; i < arr.size(); i++)
            degs.append(arr[i].toDouble() * PI / 180);
        pose_r.append(RobotControl::eulerAngleToRotationVec(degs));
    }
    else
        pose_r = getPoseByXY(1, fpl[0], fpl[1]);//按坐标
    if(pose_l.size() != 3 || pose_r.size() != 3)
    {
        errquit(QString("cure_pose config error"), "该经络机械臂姿态配置缺失，请联系运维处理");
        return;
    }
    fpl.append(pose_l);
    fpr.append(pose_r);
    //某些经络为防止碰撞 先额外移动一次
    if((g_cureVersion == 0 && meridian == "pangguang1") || meridian == "dan2" || meridian == "dan1" || meridian == "du" || meridian == "ren")
    {
        QVector<double> tmp_fpl = fpl;
        QVector<double> tmp_fpr = fpr;
        if(meridian == "dan1")
        {
            tmp_fpl[2] += 0.1;
            tmp_fpr[2] += 0.1;
        }
        else if(meridian == "du" || meridian == "ren")
        {
            tmp_fpl[1] += 0.1;
            tmp_fpr[1] += 0.1;
        }
        else if(meridian == "pangguang1")
        {
            tmp_fpl[1] += 0.1;
            tmp_fpr[1] += 0.1;
            tmp_fpl[2] += 0.1;
            tmp_fpr[2] += 0.1;
        }
        else if(meridian == "dan2")
        {
            tmp_fpl[2] += 0.15;
            tmp_fpr[2] += 0.15;
        }
        m_robots[0]->moveToPos('l', 1, tmp_fpl, 0.1, 0.1);
        m_robots[1]->moveToPos('l', 1, tmp_fpr, 0.1, 0.1);
        Global::parallel<int, RobotControl, QStringList, int>(&RobotControl::waitReplyFromTCPClient, m_robots[0], m_robots[1], QStringList({"movel"}), 15000);
    }
    m_robots[0]->moveToPos('l', 1, fpl, 0.1, 0.1);
    m_robots[1]->moveToPos('l', 1, fpr, 0.1, 0.1);
    Global::parallel<int, RobotControl, QStringList, int>(&RobotControl::waitReplyFromTCPClient, m_robots[0], m_robots[1], QStringList({"movel"}), 15000);
    if(m_stopTreatment)
    {
        normalquit();
        return;
    }
    //尝试接触人体
    auto t_contact = [=](int rbindex){
        if(rbindex == 0)
            m_robots[rbindex]->contact(contact_dir_l, 0.1, 1);
        else
            m_robots[rbindex]->contact(contact_dir_r, 0.1, 1);
        m_robots[rbindex]->waitReplyFromTCPClient(QStringList({"tool_contact"}), 15000);
    };
    bool arrive1 = false;
    bool arrive2 = false;
    auto startpose_l = Global::fromArray(m_robots[0]->getStatus().toolVectorActual, 6);
    auto startpose_r = Global::fromArray(m_robots[1]->getStatus().toolVectorActual, 6);
    while(!arrive1 || !arrive2)
    {
        if(!arrive1 && !arrive2)
        {
            std::thread tc1(t_contact, 0);
            std::thread tc2(t_contact, 1);
            tc1.join();
            tc2.join();
        }
        else if(!arrive1)
            t_contact(0);
        else
            t_contact(1);
        if(m_stopTreatment)
        {
            normalquit();
            return;
        }
        auto endpose_l = Global::fromArray(m_robots[0]->getStatus().toolVectorActual, 6);
        auto endpose_r = Global::fromArray(m_robots[1]->getStatus().toolVectorActual, 6);
        auto dis_l = qSqrt(qPow(endpose_l[0]-startpose_l[0], 2) + qPow(endpose_l[1]-startpose_l[1], 2) + qPow(endpose_l[2]-startpose_l[2], 2));
        auto dis_r = qSqrt(qPow(endpose_r[0]-startpose_r[0], 2) + qPow(endpose_r[1]-startpose_r[1], 2) + qPow(endpose_r[2]-startpose_r[2], 2));
        if(dis_l > 0.06)
            arrive1 = true;
        if(dis_r > 0.06)
            arrive2 = true;
    }
    LOG_INFO << "contact body, ready to go";
    //开始循环发点
    int pointIndex = 1;
    while(pointIndex < curePts[0].size())
    {
        if(m_stopTreatment)
        {
            normalquit();
            return;
        }
        LOG_INFO << "from" << curePts[0][pointIndex-1] << "to" << curePts[0][pointIndex];
        //机械臂在人体前后X方向动作阈值判断，是否需要动床
        auto curBedPos = m_bedPos;
        bool judgeInRange = true;
        if((m_acupoints[curePts[0][pointIndex]].robot_x + m_bedPos > ROBOTX_THREHOLD_TAIL && m_acupoints[curePts[1][pointIndex]].robot_x - m_bedPos > ROBOTX_THREHOLD_HEAD) ||
                (m_acupoints[curePts[0][pointIndex]].robot_x + m_bedPos < -ROBOTX_THREHOLD_HEAD && m_acupoints[curePts[1][pointIndex]].robot_x - m_bedPos < -ROBOTX_THREHOLD_TAIL))
        {
            //由于2机械臂坐标系x反向，不可能出现该情况
            errquit("impossible!", "错误，请联系运维人员处理");
            return;
        }
        else
        {
            if(m_acupoints[curePts[0][pointIndex]].robot_x + m_bedPos > ROBOTX_THREHOLD_TAIL || m_acupoints[curePts[1][pointIndex]].robot_x - m_bedPos < -ROBOTX_THREHOLD_TAIL)//床尾方向超限
            {
                judgeInRange = false;
                double range_pos1 = ROBOTX_THREHOLD_TAIL - m_acupoints[curePts[0][pointIndex]].robot_x;
                double range_pos2 = ROBOTX_THREHOLD_TAIL + m_acupoints[curePts[1][pointIndex]].robot_x;
                auto range_pos = range_pos1 < range_pos2 ? range_pos1 : range_pos2;
                if(range_pos < -m_bedCtrl->m_move_threhold_head)
                {
                    errquit(QString("adjust bed error: ") + QString::number(range_pos), "机械臂床联动错误，请联系运维人员处理");
                    return;
                }
                else
                {
                    m_bedPos = 0;
                    while(m_bedPos > range_pos)
                        m_bedPos -= 0.1;
                    if(m_bedPos < -m_bedCtrl->m_move_threhold_head)
                        m_bedPos = -m_bedCtrl->m_move_threhold_head;
                }
            }
            else if(m_acupoints[curePts[0][pointIndex]].robot_x + m_bedPos < -ROBOTX_THREHOLD_HEAD ||
                    m_acupoints[curePts[1][pointIndex]].robot_x - m_bedPos > ROBOTX_THREHOLD_HEAD)//床头方向超限
            {
                judgeInRange = false;
                double range_pos1 = -ROBOTX_THREHOLD_HEAD - m_acupoints[curePts[0][pointIndex]].robot_x;
                double range_pos2 = -ROBOTX_THREHOLD_HEAD + m_acupoints[curePts[1][pointIndex]].robot_x;
                auto range_pos = range_pos1 > range_pos2 ? range_pos1 : range_pos2;
                if(range_pos > m_bedCtrl->m_move_threhold_tail)
                {
                    errquit(QString("adjust bed error: ") + QString::number(range_pos), "机械臂床联动错误，请联系运维人员处理");
                    return;
                }
                else
                {
                    m_bedPos = 0;
                    while(m_bedPos < range_pos)
                        m_bedPos += 0.1;
                    if(m_bedPos > m_bedCtrl->m_move_threhold_tail)
                        m_bedPos = m_bedCtrl->m_move_threhold_tail;
                }
            }
        }
        //如果刚落到人体即判断超限，属于床初始位置配置不合理
        if(!judgeInRange && pointIndex == 1)
        {
            errquit(QString("ini bed pos not reasonable: ") + meridian, "经络初始床位置不合理，请联系运维人员处理");
            return;
        }
        if(!judgeInRange)//机械臂床联动
        {
            //获取机械臂当前位姿
            auto curpos_l = Global::fromArray(m_robots[0]->getStatus().toolVectorActual, 6);
            auto curpos_r = Global::fromArray(m_robots[1]->getStatus().toolVectorActual, 6);
            auto point_l = caliCordsByBed(0, m_bedPos - curBedPos, curpos_l.mid(0, 3));
            auto point_r = caliCordsByBed(1, m_bedPos - curBedPos, curpos_r.mid(0, 3));
            point_l.append(curpos_l.mid(3, 3));
            point_r.append(curpos_r.mid(3, 3));
            if(g_CurePoseConfig->contains(curePts[0][pointIndex-1]))//按配置文件
            {
                auto arr = g_CurePoseConfig->value(curePts[0][pointIndex-1]).toArray();
                QVector<double> degs;
                for(int i = 0; i < arr.size(); i++)
                    degs.append(arr[i].toDouble() * PI / 180);
                pose_l = RobotControl::eulerAngleToRotationVec(degs);
            }
            else
                pose_l = getPoseByXY(0, point_l[0], point_l[1]);//按坐标
            if(g_CurePoseConfig->contains(curePts[1][pointIndex-1]))//按配置文件
            {
                auto arr = g_CurePoseConfig->value(curePts[1][pointIndex-1]).toArray();
                QVector<double> degs;
                for(int i = 0; i < arr.size(); i++)
                    degs.append(arr[i].toDouble() * PI / 180);
                pose_r = RobotControl::eulerAngleToRotationVec(degs);
            }
            else
                pose_r = getPoseByXY(1, point_l[0], point_l[1]);//按坐标
            for(int i = 3; i < 6; i++)
            {
                point_l[i] = pose_l[i - 3];
                point_r[i] = pose_r[i - 3];
            }
            m_robots[0]->setforce(force_l);
            m_robots[1]->setforce(force_r);
            LOG_INFO << "robot move along with bed";
            m_robots[0]->moveToPos('l', 1, point_l, DEFAULT_CURE_ACC, DEFAULT_CURE_SPEED);
            m_robots[1]->moveToPos('l', 1, point_r, DEFAULT_CURE_ACC, DEFAULT_CURE_SPEED);
            last_pt_l = point_l;
            last_pt_r = point_r;
            std::thread t1(&RobotControl::waitReplyFromTCPClient, m_robots[0], QStringList({"movel"}), 45000);
            std::thread t2(&RobotControl::waitReplyFromTCPClient, m_robots[1], QStringList({"movel"}), 45000);
            std::thread t3(&BedControl::moveTo, m_bedCtrl, m_bedPos, 1, 45000);
            t1.join();
            t2.join();
            t3.join();
            LOG_INFO << "robot move along with bed finished";
            if(m_stopTreatment)
            {
                normalquit();
                return;
            }
        }
        else//移动至下个穴位点
        {
            auto t_move = [=](int rbindex, QVector<double> *last_pt, msg_force_startforce *force){
                //计算位姿 2穴位点和之间的插值点统一用一个姿态
                QVector<double> pose;
                if(g_CurePoseConfig->contains(curePts[rbindex][pointIndex]))//按配置文件
                {
                    auto arr = g_CurePoseConfig->value(curePts[rbindex][pointIndex]).toArray();
                    QVector<double> degs;
                    for(int i = 0; i < arr.size(); i++)
                        degs.append(arr[i].toDouble() * PI / 180);
                    pose = RobotControl::eulerAngleToRotationVec(degs);
                }
                else//按坐标
                {
                    if(rbindex == 0)
                        pose = getPoseByXY(rbindex, m_acupoints[curePts[rbindex][pointIndex]].robot_x,
                                                    m_acupoints[curePts[rbindex][pointIndex]].robot_y);
                    else
                        pose = getPoseByXY(rbindex, -m_acupoints[curePts[rbindex][pointIndex]].robot_x,
                                                     m_acupoints[curePts[rbindex][pointIndex]].robot_y);
                }
                //移动至各插值点
//                auto ptname = curePts[rbindex][pointIndex - 1];
//                int index = 1;
//                auto from_ptname = ptname;
//                while(index < 100)
//                {
//                    auto to_ptname = meridian + "-" + ptname + "-v-" + QString::number(index);
//                    if(m_acupoints.contains(to_ptname))
//                    {
//                        auto point = caliCordsByBed(rbindex, m_bedPos, QVector<double>({m_acupoints[to_ptname].robot_x,
//                                                                                        m_acupoints[to_ptname].robot_y,
//                                                                                        m_acupoints[to_ptname].robot_z}));
//                        point.append(pose);
//                        cureNextPoint(rbindex, m_acupoints[from_ptname], point,
//                                      *last_pt, *force, (pointIndex == 1 && index == 1));
//                        from_ptname = to_ptname;
//                    }
//                    else
//                        break;
//                    index++;
//                }
                //移动至目标点
                auto point = caliCordsByBed(rbindex, m_bedPos, QVector<double>({m_acupoints[curePts[rbindex][pointIndex]].robot_x,
                                                                                m_acupoints[curePts[rbindex][pointIndex]].robot_y,
                                                                                m_acupoints[curePts[rbindex][pointIndex]].robot_z}));
                point.append(pose);
                cureNextPoint(rbindex, m_acupoints[curePts[rbindex][pointIndex - 1]], point,
                              *last_pt, *force, (pointIndex == 1));
            };
            std::thread t1(t_move, 0, &last_pt_l, &force_l);
            std::thread t2(t_move, 1, &last_pt_r, &force_r);
            t1.join();
            t2.join();
            //判断是否需要中止调理
            if(m_stopTreatment)
            {
                normalquit();
                return;
            }
            if(m_robots[0]->getStatus().safetyModes != 1 || m_robots[1]->getStatus().safetyModes != 1)
            {
                errquit("move error", "机械臂状态错误");
                return;
            }
            //判断是否需要停留一段时间
            if(g_CurePtPauseConfig->contains(curePts[0][pointIndex]) && pointIndex < curePts[0].size() - 1)
            {
                //重置力控
                m_robots[0]->stopMoving(DEFAULT_CURE_ACC);
                m_robots[1]->stopMoving(DEFAULT_CURE_ACC);
                m_robots[0]->endforce();
                m_robots[1]->endforce();
                m_robots[0]->setforce(force_l);
                m_robots[1]->setforce(force_r);
                int pauseTime = g_CurePtPauseConfig->value(curePts[0][pointIndex]).toInt();
                QThread::msleep(pauseTime);
            }
            if(pointIndex == curePts[0].size() - 1)
            {
                //关震动
                for(int i = 0; i < m_robots.size(); i++)
                    m_robots[i]->vibrateOff();
                QThread::msleep(1200);
            }
            m_robots[0]->stopMoving(DEFAULT_CURE_ACC);
            m_robots[1]->stopMoving(DEFAULT_CURE_ACC);
            LOG_INFO << curePts[0][pointIndex] << "arrived";
            //二次识别
            if(g_RepredictPtsConfig->contains(curePts[0][pointIndex]) && !g_usingImg)
            {
                QStringList args;
                auto arr = g_RepredictPtsConfig->value(curePts[0][pointIndex]).toArray();
                for(int ii = 0; ii < arr.size(); ii++)
                    args.append(arr[ii].toString());
                repredictIn2D(args);
            }
            pointIndex++;
        }
        m_robots[0]->endforce();
        m_robots[1]->endforce();
    }
    //读取当前位置加上偏移量抬起
    auto curpos_l = Global::fromArray(m_robots[0]->getStatus().toolVectorActual, 6);
    auto curpos_r = Global::fromArray(m_robots[1]->getStatus().toolVectorActual, 6);
    curpos_l[1] += 0.08;
    curpos_r[1] += 0.08;
    curpos_l[2] += 0.08;
    curpos_r[2] += 0.08;
    m_robots[0]->moveToPos('l', 1, curpos_l, 0.1, 0.1);
    m_robots[1]->moveToPos('l', 1, curpos_r, 0.1, 0.1);
    Global::parallel<int, RobotControl, QStringList, int>(&RobotControl::waitReplyFromTCPClient, m_robots[0], m_robots[1], QStringList({"movel"}), 10000);
    double range = 0.3;
    if(qAbs(curpos_l[0] - 0.132) > range || qAbs(curpos_r[0] + 0.136) > range)
    {
        auto curJointPos_l = Global::fromArray(m_robots[0]->getStatus().qActual, 6);
        auto curJointPos_r = Global::fromArray(m_robots[1]->getStatus().qActual, 6);
        QVector<double> preSafePos_l = LEFT_SAFE_POS;
        QVector<double> preSafePos_r = RIGHT_SAFE_POS;
        if(curpos_l[0] - 0.132 > range || curpos_r[0] + 0.136 < -range)
        {
            curJointPos_l[4] += 1.571;
            curJointPos_r[4] -= 1.571;
            preSafePos_l[4] = 0;
            preSafePos_r[4] = 0;
        }
        else if(curpos_l[0] - 0.132 < -range || curpos_r[0] + 0.136 > range)
        {
            curJointPos_l[4] -= 1.571;
            curJointPos_r[4] += 1.571;
            preSafePos_l[4] = -3.14;
            preSafePos_r[4] = 3.14;
        }
        m_robots[0]->moveToPos('j', 2, curJointPos_l, 0.3, 0.3);
        m_robots[1]->moveToPos('j', 2, curJointPos_r, 0.3, 0.3);
        Global::parallel<int, RobotControl, QStringList, int>(&RobotControl::waitReplyFromTCPClient, m_robots[0], m_robots[1], QStringList({"movej"}), 10000);
        m_robots[0]->moveToPos('j', 2, preSafePos_l, 0.3, 0.3);
        m_robots[1]->moveToPos('j', 2, preSafePos_r, 0.3, 0.3);
        Global::parallel<int, RobotControl, QStringList, int>(&RobotControl::waitReplyFromTCPClient, m_robots[0], m_robots[1], QStringList({"movej"}), 10000);
    }
    parallel(&RobotControl::safePos, &result, &result1);
    LOG_INFO << "cure finish leave off";
    //关闭力控软保护
    for(int i = 0; i < m_robots.size(); i++)
        m_robots[i]->setRTSoftProtectiveStopFlag(false);
    //停止调理期间图片保存
    if(g_runArgs["SaveCureImg"] == "true")
    {
        for(int i = 0; i < m_camobjs.size(); i++)
            m_camobjs[i]->setCurePosture("");
    }
    //通知UI该经络调理结束
    if(!m_stopTreatment)
        emit cureFinished(true);
    m_isTreating = false;
    m_robots[0]->blockFreeDrive(false);
    m_robots[1]->blockFreeDrive(false);
}

void CoreProcedureWorker::errquit(QString log, QString uimsg)
{
    for(int i = 0; i < m_robots.size(); i++)
        m_robots[i]->vibrateOff();
    for(int i = 0; i < m_robots.size(); i++)
        m_robots[i]->stopMoving();
    for(int i = 0; i < m_robots.size(); i++)
        m_robots[i]->endforce();
    if(g_runArgs["SaveCureImg"] == "true")
    {
        for(int i = 0; i < m_camobjs.size(); i++)
            m_camobjs[i]->setCurePosture("");
    }
    LOG_INFO << log;
    emit warn(QString("调理中止").toLocal8Bit(), uimsg.toLocal8Bit());
    m_isTreating = false;
    m_robots[0]->blockFreeDrive(false);
    m_robots[1]->blockFreeDrive(false);
    emit cureFinished(false);
}

void CoreProcedureWorker::cureNextPoint(int rbindex, IdentifyPointInfo &from_pt, QVector<double> &to_pt,
                                        QVector<double> &last_point, msg_force_startforce &force, bool vibon)
{
    //计算力控方向
    bool force_covered = false;
    QVector<double> tmpdir;
    if(g_CureForceOverrideConfig->contains(from_pt.acuname))
    {
        force_covered = true;
        auto tmparr = g_CureForceOverrideConfig->value(from_pt.acuname).toArray();
        tmpdir = RobotControl::eulerAngleToRotationVec({tmparr[0].toDouble() * PI / 180,
                                                        tmparr[1].toDouble() * PI / 180,
                                                        tmparr[2].toDouble() * PI / 180});
    }
    if(!force_covered)
    {
        tmpdir = RobotControl::eulerAngleToRotationVec(RobotControl::vecToEulerAngle({0, 0, 1},
                                                            {from_pt.force_x, from_pt.force_y, from_pt.force_z}));
    }
    for(int i = 3; i < 6; i++)
        force.aixs_bias[i] = tmpdir[i - 3];
    //计算爬坡力
    msg_force_startforce climb_f;
    climb_f.force_value[2] = 7.5;
    climb_f.speed[2] = DEFAULT_CURE_SPEED;
    tmpdir = RobotControl::eulerAngleToRotationVec(RobotControl::vecToEulerAngle({0, 0, 1},
                                                               {to_pt[0] - last_point[0],
                                                                to_pt[1] - last_point[1],
                                                                to_pt[2] - last_point[2]}));
    for(int i = 3; i < 6; i++)
        climb_f.aixs_bias[i] = tmpdir[i - 3];
    //爬坡方向
    QVector<double> lift_dir = {-from_pt.force_x,
                                -from_pt.force_y,
                                -from_pt.force_z};
    //设置力控值大小
    double force_bias = 0;
    if(g_CureForceBiasConfig->contains(from_pt.acuname))
        force_bias = g_CureForceBiasConfig->value(from_pt.acuname).toDouble();
    force.force_value[2] = g_cureForceVal + force_bias;
    m_robots[rbindex]->setforce(force);
    //开启震动
    if(vibon)
        m_robots[rbindex]->vibrateOn();
    //发点
    if(!force_covered)
        m_robots[rbindex]->startAutoForce(last_point, to_pt, force, climb_f, lift_dir);
    m_robots[rbindex]->moveToPos('l', 1, to_pt, DEFAULT_CURE_ACC, DEFAULT_CURE_SPEED);
    m_robots[rbindex]->waitReplyFromTCPClient(QStringList({"movel"}), 30000);
    if(!force_covered)
        m_robots[rbindex]->endAutoForce();
    last_point = to_pt;
}

void CoreProcedureWorker::normalquit()
{
    if(g_runArgs["SaveCureImg"] == "true")
    {
        for(int i = 0; i < m_camobjs.size(); i++)
            m_camobjs[i]->setCurePosture("");
    }
    m_isTreating = false;
    m_robots[0]->blockFreeDrive(false);
    m_robots[1]->blockFreeDrive(false);
}

QVector<double> CoreProcedureWorker::calForceDir(QVector<double> p1, QVector<double> p2, QVector<double> ref_v, double deg)
{
    //转为mm
    for(int i = 0; i < p1.size(); i++)
        p1[i] *= 1000.0;
    for(int i = 0; i < p2.size(); i++)
        p2[i] *= 1000.0;
    auto p = {p2[0] - p1[0], p2[1] - p1[1], p2[2] - p1[2]};
    auto climb_fvec = RobotControl::eulerAngleToRotationVec(RobotControl::vecToEulerAngle({0, 0, 1}, p));//爬坡力方向
    //计算力方向向量 默认垂直路径且与参考方向夹角小于90度
    auto v1 = RobotControl::xVec(p, ref_v);//TODO 可能需判断向量是否平行?
    auto v2 = RobotControl::xVec(v1, p);
    if(RobotControl::dotVec(v2, ref_v) < 0)
    {
        for(int i = 0; i < v2.size(); i++)
            v2[i] = -v2[i];
    }
    auto fvec = v2;
    auto climb_mvec = {-fvec[0], -fvec[1], -fvec[2]};//爬坡移动方向
    if(deg != 0)//需倾斜deg角度
    {
        auto tmpv = RobotControl::rotateVecAlongAxis(v2, v1, deg);
        auto tmpv1 = RobotControl::rotateVecAlongAxis(v2, v1, -deg);
        if(RobotControl::dotVec(tmpv, p) > 0)
            fvec = tmpv;
        else
            fvec = tmpv1;
    }
    QVector<double> ret = RobotControl::eulerAngleToRotationVec(RobotControl::vecToEulerAngle({0, 0, 1}, fvec));//旋转向量
    ret.append(climb_fvec);
    ret.append(climb_mvec);
    LOG_INFO << "移动:" << QVector<double>(p) << "力:" << fvec;
    return ret;
}

QVector<double> CoreProcedureWorker::getPoseByXY(int rbindex, double x, double y)
{
    //根据左侧机械臂位置判断姿态，对侧机械臂姿态取对称即可
    for(int i = 0; i < m_poseBook.size(); i++)
    {
        if(x >= m_poseBook[i].low_x && x <= m_poseBook[i].high_x && y >= m_poseBook[i].low_y && y <= m_poseBook[i].high_y)
        {
            if(rbindex == 0)
                return m_poseBook[i].pose_left;
            else
                return m_poseBook[i].pose_right;
        }
    }
    return QVector<double>({});
}

QMap<QString, QVector<QStringList>> CoreProcedureWorker::genAcunameMap(QStringList jllist)
{
    QMap<QString, QVector<QStringList>> ret;
    QJsonArray ref;
    if(g_cureVersion == 0)
        ref = g_JingLuoConfig->value("pose_down").toArray();
    else
        ref = g_JingLuoConfig_Body->value("pose_down").toArray();
    for(int i = 0; i < jllist.size(); i++)
    {
        auto jlname = jllist[i];
        if(jlname[jlname.size() - 1].isDigit())
            jlname.chop(1);
        QVector<QStringList> tmpv;
        QStringList list1;
        QStringList list2;
        QJsonArray arr;
        if(g_cureVersion == 0)
            arr = g_JingLuoConfig->value(jllist[i]).toArray();
        else
            arr = g_JingLuoConfig_Body->value(jllist[i]).toArray();
        Q_ASSERT(arr.size() % 2 == 0);
        for(int i = 0; i < arr.size() / 2; i++)
        {
            int low = arr[2*i].toInt();
            int high = arr[2*i+1].toInt();
            Q_ASSERT(low <= high);
            for(int j = low; j <= high; j++)
            {
                list1.append(QString("L-") + jlname + "-" + QString::number(j));
                list2.append(QString("R-") + jlname + "-" + QString::number(j));
            }
        }
        if(ref.contains(jllist[i]))
        {
            tmpv.append(list2);
            tmpv.append(list1);
        }
        else
        {
            tmpv.append(list1);
            tmpv.append(list2);
        }
        ret.insert(jllist[i], tmpv);
    }
    return ret;
}

void CoreProcedureWorker::iniRedPoint()
{
    if(!m_iniRedPtFlag/* && !g_testEMC*/)
    {
        m_iniRedPtFlag = true;
        emit blockUI("初始化中，请稍候...", 0);
        m_cali_camid = 0;
        m_cali_rbindex = 0;
        m_cali_points.clear();
        if(!m_robots[0]->isInPhotoPos())
        {
            m_robots[0]->safePos();
            m_robots[0]->photoPos();
        }
        if(!m_robots[1]->isInPhotoPos())
        {
            m_robots[1]->safePos();
            m_robots[1]->photoPos();
        }
        if(g_testCali == 1)
        {
            if(m_bedCtrl->moveTo(g_testIniBedPos))
                emit blockUI("初始化完毕", 1000);
            else
                emit blockUI("床移动错误", 3000);
        }
        else
            emit blockUI("初始化完毕", 1000);
        m_iniRedPtFlag = false;
    }
}

void CoreProcedureWorker::recordRedPoint(int camid, double pix_x, double pix_y, int rbindex, double preserve)
{
    m_cali_camid = camid;
    m_cali_rbindex = rbindex;
    m_contact_bias = preserve;
    int redptx = pix_x;
    int redpty = pix_y;
    if(camid == 3)
    {
        redptx = 1280 - redptx;
        redpty = 720 - redpty;
    }
    if(redptx < 0 || redptx > 1280 || redpty < 0 || redpty > 720)
        return;
    auto depth = m_camobjs[camid]->getDepthData();
    auto z = m_camobjs[camid]->getDepthValue(redptx, redpty, depth);
    double x, y;
    m_camobjs[camid]->cam2dToCam3d(redptx, redpty, z, x, y);
    m_camobjs[camid]->cam3dToRobot3d(rbindex, x, y, z);
    m_camobjs[camid]->caliFix(rbindex, x, y, z);
    m_cali_points.append(QVector<double>{x, y, z});
}

void CoreProcedureWorker::goToRedPoint(int mod)
{
    if(m_cali_points.size() == 0)
        return;
    emit blockUI("机械臂移动中，请稍等...", 0);
    m_isTreating = true;
    m_robots[m_cali_rbindex]->setDamp(DEFAULT_CURE_DAMP);
    m_robots[m_cali_rbindex]->setGain(DEFAULT_CURE_GAIN);
    m_robots[m_cali_rbindex]->safePos();
    for(int i = 0; i < m_cali_points.size(); i++)
    {
        auto x = m_cali_points[i][0];
        auto y = m_cali_points[i][1];
        auto z = m_cali_points[i][2];
        //调理床移动
        if(g_testCali == 1)
        {
            auto bedpos = g_testIniBedPos;
            if(mod == 6)
                bedpos = 0.5;
            else if(qAbs(x) > 0.6)
            {
                auto chkx = (m_cali_rbindex == 0 ? x : -x);
                if(qAbs(chkx - g_testIniBedPos) > 1)
                {
                    emit blockUI("超出标定验证范围", 3000);
                    return;
                }
                if(m_cali_rbindex == 0)
                {
                    if(x > 0.6)
                        bedpos = 0.6 - x + g_testIniBedPos;
                    else
                        bedpos = -0.6 - x + g_testIniBedPos;
                }
                else
                {
                    if(x > 0.6)
                        bedpos = x - 0.6 + g_testIniBedPos;
                    else
                        bedpos = x + 0.6 + g_testIniBedPos;
                }
            }
            m_bedCtrl->moveTo(bedpos);
            if(m_stopTreatment)
            {
                m_isTreating = false;
                return;
            }
            auto cords = caliCordsByBed(m_cali_rbindex, bedpos - g_testIniBedPos, QVector<double>({x, y, z}));
            x = cords[0];
            y = cords[1];
            z = cords[2];
        }
        else
        {
            if(qAbs(x) > 0.6)
            {
                emit blockUI("超出标定验证范围", 3000);
                return;
            }
        }
        //移动
        QVector<double> xyz;
        QVector<double> pose;
        QVector<double> contact_spd;
        auto getRad = [=](double a, double b, double c)->QVector<double>{
            return QVector<double>({a * 3.1415926 / 180, b * 3.1415926 / 180, c * 3.1415926 / 180});
        };
        if(mod == 1)
        {
            xyz = {x, y, z + m_contact_bias};
            if(qAbs(x) > 0.3)
            {
                if(m_cali_rbindex == 0)
                {
                    if(x > 0.3) pose = RobotControl::eulerAngleToRotationVec(getRad(0, 150, 0));
                    else pose = RobotControl::eulerAngleToRotationVec(getRad(0, -150, 0));
                }
                else
                {
                    if(x < -0.3) pose = RobotControl::eulerAngleToRotationVec(getRad(180, 30, 0));
                    else pose = RobotControl::eulerAngleToRotationVec(getRad(180, -30, 0));
                }
            }
            else
            {
                if(m_cali_rbindex == 0) pose = RobotControl::eulerAngleToRotationVec(getRad(0, 180, 0));
                else pose = RobotControl::eulerAngleToRotationVec(getRad(180, 0, 0));
            }
            contact_spd = {0, 0, -0.02, 0, 0, 0};
        }
        else if(mod == 2)
        {
            xyz = {x, y + m_contact_bias, z};
            if(qAbs(x) > 0.3)
            {
                if(m_cali_rbindex == 0)
                {
                    if(x > 0.3) pose = RobotControl::eulerAngleToRotationVec(getRad(30, 150, 0));
                    else pose = RobotControl::eulerAngleToRotationVec(getRad(30, -150, 0));
                }
                else
                {
                    if(x < -0.3) pose = RobotControl::eulerAngleToRotationVec(getRad(150, 30, 0));
                    else pose = RobotControl::eulerAngleToRotationVec(getRad(150, -30, 0));
                }
            }
            else
            {
                if(m_cali_rbindex == 0) pose = RobotControl::eulerAngleToRotationVec(getRad(30, 180, 0));
                else pose = RobotControl::eulerAngleToRotationVec(getRad(150, 0, 0));
            }
            contact_spd = {0, -0.02, 0, 0, 0, 0};
        }
        else if(mod == 3)
        {
            xyz = {x, y + m_contact_bias, z + m_contact_bias};
            if(qAbs(x) > 0.3)
            {
                if(m_cali_rbindex == 0)
                {
                    if(x > 0.3) pose = RobotControl::eulerAngleToRotationVec(getRad(30, 150, 0));
                    else pose = RobotControl::eulerAngleToRotationVec(getRad(30, -150, 0));
                }
                else
                {
                    if(x < -0.3) pose = RobotControl::eulerAngleToRotationVec(getRad(150, 30, 0));
                    else pose = RobotControl::eulerAngleToRotationVec(getRad(150, -30, 0));
                }
            }
            else
            {
                if(m_cali_rbindex == 0) pose = RobotControl::eulerAngleToRotationVec(getRad(30, 180, 0));
                else pose = RobotControl::eulerAngleToRotationVec(getRad(150, 0, 0));
            }
            contact_spd = {0, -0.014, -0.014, 0, 0, 0};
        }
        else if(mod == 4)
        {
            xyz = {x, y - m_contact_bias, z + m_contact_bias};
            if(qAbs(x) > 0.3)
            {
                if(m_cali_rbindex == 0)
                {
                    if(x > 0.3) pose = RobotControl::eulerAngleToRotationVec(getRad(-15, 150, 0));
                    else pose = RobotControl::eulerAngleToRotationVec(getRad(-15, -150, 0));
                }
                else
                {
                    if(x < -0.3) pose = RobotControl::eulerAngleToRotationVec(getRad(-165, 30, 0));
                    else pose = RobotControl::eulerAngleToRotationVec(getRad(-165, -30, 0));
                }
            }
            else
            {
                if(m_cali_rbindex == 0) pose = RobotControl::eulerAngleToRotationVec(getRad(-15, 180, 0));
                else pose = RobotControl::eulerAngleToRotationVec(getRad(-165, 0, 0));
            }
            contact_spd = {0, 0.014, -0.014, 0, 0, 0};
        }
        else if(mod == 5)
        {
            xyz = {x, y - m_contact_bias, z};
            if(qAbs(x) > 0.3)
            {
                if(m_cali_rbindex == 0)
                {
                    if(x > 0.3) pose = RobotControl::eulerAngleToRotationVec(getRad(-15, 150, 0));
                    else pose = RobotControl::eulerAngleToRotationVec(getRad(-15, -150, 0));
                }
                else
                {
                    if(x < -0.3) pose = RobotControl::eulerAngleToRotationVec(getRad(-165, 30, 0));
                    else pose = RobotControl::eulerAngleToRotationVec(getRad(-165, -30, 0));
                }
            }
            else
            {
                if(m_cali_rbindex == 0) pose = RobotControl::eulerAngleToRotationVec(getRad(-15, 180, 0));
                else pose = RobotControl::eulerAngleToRotationVec(getRad(-165, 0, 0));
            }
            contact_spd = {0, 0.02, 0, 0, 0, 0};
        }
        else if(mod == 6)
        {
            if(m_cali_rbindex == 0)
                xyz = {x - m_contact_bias, y, z + m_contact_bias};
            else
                xyz = {x + m_contact_bias, y, z + m_contact_bias};
            if(m_cali_rbindex == 0)
                pose = RobotControl::eulerAngleToRotationVec(getRad(30, 150, 0));
            else
                pose = RobotControl::eulerAngleToRotationVec(getRad(150, 30, 0));
            if(m_cali_rbindex == 0)
                contact_spd = {0.02, 0, -0.02, 0, 0, 0};
            else
                contact_spd = {-0.02, 0, -0.02, 0, 0, 0};
        }
        else
        {
            xyz = {x, y, z};
            if(qAbs(x) > 0.3 && mod == 0)
            {
                if(m_cali_rbindex == 0)
                {
                    if(x > 0.3) pose = RobotControl::eulerAngleToRotationVec(getRad(15, 150, 0));
                    else pose = RobotControl::eulerAngleToRotationVec(getRad(15, -150, 0));
                }
                else
                {
                    if(x < -0.3) pose = RobotControl::eulerAngleToRotationVec(getRad(165, 30, 0));
                    else pose = RobotControl::eulerAngleToRotationVec(getRad(165, -30, 0));
                }
            }
            else
            {
                if(m_cali_rbindex == 0) pose = RobotControl::eulerAngleToRotationVec(getRad(0, 180, 0));
                else pose = RobotControl::eulerAngleToRotationVec(getRad(180, 0, 0));
            }
        }
        if(mod == 6)
        {
            auto tmpxyz = xyz;
            tmpxyz[1] = -0.493;
            m_robots[m_cali_rbindex]->moveToPos('l', 1, tmpxyz + pose, 0.1, 0.1);
            m_robots[m_cali_rbindex]->waitReplyFromTCPClient({"movel"});
            if(m_stopTreatment)
            {
                m_isTreating = false;
                return;
            }
        }
        m_robots[m_cali_rbindex]->moveToPos('l', 1, xyz + pose, 0.1, 0.1);
        m_robots[m_cali_rbindex]->waitReplyFromTCPClient({"movel"});
        if(m_stopTreatment)
        {
            m_isTreating = false;
            return;
        }
        if(mod != 0 && mod != 7)
        {
//            m_robots[m_cali_rbindex]->contact(contact_spd, 0.1, 1);
//            m_robots[m_cali_rbindex]->waitReplyFromTCPClient(QStringList({"tool_contact"}));
            auto tmpdir = RobotControl::eulerAngleToRotationVec(RobotControl::vecToEulerAngle({0, 0, 1}, contact_spd.mid(0,3)));
            msg_force_startforce force;
            force.force_value[2] = 2;
            for(int i = 0; i < 3; i++)
                force.aixs_bias[i+3] = tmpdir[i];
            m_robots[m_cali_rbindex]->clearforce();
            m_robots[m_cali_rbindex]->setforce(force);
            QThread::sleep(5);
            m_robots[m_cali_rbindex]->endforce();
        }
        if(m_stopTreatment)
        {
            m_isTreating = false;
            return;
        }
        QThread::sleep(5);
        if(mod == 6)
        {
            auto tmpxyz = xyz;
            tmpxyz[1] = -0.493;
            if(tmpxyz[2] < 0.1)
                tmpxyz[2] = 0.1;
            m_robots[m_cali_rbindex]->moveToPos('l', 1, tmpxyz + pose, 0.1, 0.1);
            m_robots[m_cali_rbindex]->waitReplyFromTCPClient({"movel"});
            if(m_stopTreatment)
            {
                m_isTreating = false;
                return;
            }
            auto curJointPos = Global::fromArray(m_robots[m_cali_rbindex]->getStatus().qActual, 6);
            QVector<double> preSafePos;
            if(m_cali_rbindex == 0)
                curJointPos[4] -= 1.571;
            else
                curJointPos[4] += 1.571;
            if(m_cali_rbindex == 0)
            {
                preSafePos = LEFT_SAFE_POS;
                preSafePos[4] = -3.14;
            }
            else
            {
                preSafePos = RIGHT_SAFE_POS;
                preSafePos[4] = 3.14;
            }
            m_robots[m_cali_rbindex]->moveToPos('j', 2, curJointPos, 0.3, 0.3);
            m_robots[m_cali_rbindex]->waitReplyFromTCPClient(QStringList({"movej"}));
            m_robots[m_cali_rbindex]->moveToPos('j', 2, preSafePos, 0.3, 0.3);
            m_robots[m_cali_rbindex]->waitReplyFromTCPClient(QStringList({"movej"}));
        }
        if(mod != 7 && i != m_cali_points.size() - 1)
            m_robots[m_cali_rbindex]->safePos();
    }
    m_isTreating = false;
    emit blockUI("定位结束", 1000);

    if(g_testEMC)
    {
        auto curpos = Global::fromArray(m_robots[m_cali_rbindex]->getStatus().toolVectorActual, 6);
        auto test_emc = [=](){
            while(g_testEMC)
            {
                m_robots[m_cali_rbindex]->safePos();
                m_robots[m_cali_rbindex]->moveToPos('l', 1, curpos, 0.1, 0.1);
                m_robots[m_cali_rbindex]->waitReplyFromTCPClient({"movel"});
                QThread::sleep(1);
            }
        };
        std::thread ttt(test_emc);
        ttt.detach();
    }
}

void CoreProcedureWorker::genCaliBias()
{
    emit blockUI("校准中，请稍候...", 0);
    waitForConfirm("提示", "请将标定板安装在左侧机械臂，然后点击确定");
    if(!m_robots[0]->isInPhotoPos())
    {
        m_robots[0]->safePos();
        m_robots[0]->photoPos();
    }
    if(!m_robots[1]->isInPhotoPos())
    {
        m_robots[1]->safePos();
        m_robots[1]->photoPos();
    }
    m_bedCtrl->moveTo(0);
    QJsonObject result;
    result.insert("h", QJsonObject());
    for(int i = 0; i < 3; i++)
    {
        if(i == 2)
            waitForConfirm("提示", "请将标定板安装在右侧机械臂，然后点击确定");
        int rbindex = 0;
        QVector<double> pos;
        char camch = 'm';
        if(i == 0)//middle
        {
            pos = {-0.6,-0.613,0.214,1.22,1.24,-1.21};
            camch = 'm';
            rbindex = 0;
        }
        else if( i == 1)//right
        {
            pos = {-0.6,-0.737,0.216,2.06,1.95,-0.331};
            camch = 'r';
            rbindex = 0;
        }
        else//left
        {
            pos = {-0.6,-0.705,0.195,1.96,1.76,-0.432};
            camch = 'l';
            rbindex = 1;
        }
        m_robots[rbindex]->safePos();
        m_robots[rbindex]->setTCP(QVector<double>({-0.0597, 0, 0.04775, 0, 0, 0}));
        vector<double> scale_head;
        vector<double> scale_tail;
        int repeat = 0;
        while(pos[0] < 0.61)
        {
            if(repeat == 0)
            {
                m_robots[rbindex]->moveToPos('l', 1, pos, 0.1, 0.1);
                m_robots[rbindex]->waitReplyFromTCPClient({"movel"});
            }
            QThread::sleep(1);
            auto imdat = m_camobjs[g_cameraOrder.indexOf(camch)]->getBGRData();
            auto depthdat = m_camobjs[g_cameraOrder.indexOf(camch)]->getDepthData();
            cv::Mat imgdata_BGR(IMAGE_HEIGHT, IMAGE_WIDTH, CV_8UC3, imdat.data());
            cv::Mat imgdata_GRAY;
            cv::cvtColor(imgdata_BGR, imgdata_GRAY, cv::COLOR_BGR2GRAY);
            cv::Mat corners;
            if(cv::findChessboardCorners(imgdata_GRAY, cv::Size(3,3), corners, cv::CALIB_CB_ADAPTIVE_THRESH))
            {
                cv::cornerSubPix(imgdata_GRAY, corners, cv::Size(3,3), cv::Size(-1,-1),
                                 cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 30, 0.001));
                float centerX = corners.row(4).col(0).at<float>(0);
                float centerY = corners.row(4).col(0).at<float>(1);
                auto d = CameraControl::getDepthValue(centerX, centerY, depthdat);
                double x, y;
                m_camobjs[g_cameraOrder.indexOf(camch)]->cam2dToCam3d(centerX, centerY, d, x, y);
                m_camobjs[g_cameraOrder.indexOf(camch)]->cam3dToRobot3d(rbindex, x, y, d);
                if((rbindex == 0 && x > 0) || (rbindex == 1 && x < 0))
                    scale_tail.push_back(pos[0] / x);
                else
                    scale_head.push_back(pos[0] / x);
            }
            else
                LOG_EXCEPTION_CAM << "find chessboard corners failed";
            if(repeat < 11)
                repeat++;
            else
            {
                repeat = 0;
                pos[0] += 1.2;
            }
        }
        auto head_s = getMidMean(scale_head);
        auto tail_s = getMidMean(scale_tail);
        result.insert(QString(camch), QJsonObject({{"head", head_s}, {"tail", tail_s}}));
        LOG_INFO << "scale result" << camch << "head:" << head_s << "tail:" << tail_s;
        m_robots[rbindex]->robotStop(true);
        QThread::msleep(500);
        m_robots[rbindex]->robotPlay(true);
        QThread::msleep(500);
        m_robots[rbindex]->safePos();
        m_robots[rbindex]->photoPos();
    }
    QJsonDocument document;
    document.setObject(result);
    QByteArray byte_array = document.toJson(QJsonDocument::Compact);
    QString json_str(byte_array);
    QFile file("config/cali_bias.json");
    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        emit blockUI("文件保存失败!", 3000);
    else
    {
        QTextStream in(&file);
        in << json_str;
        file.close();
        g_CaliConfig = QSharedPointer<QJsonObject>(new QJsonObject(result));
        emit blockUI("校准完成", 3000);
    }
    return;
}

void CoreProcedureWorker::waitForConfirm(QString title, QString msg)
{
    g_coreBlockFlag = true;
    emit warn_b(title, msg);
    while(g_coreBlockFlag)
        QThread::msleep(500);
}

QVector<double> CoreProcedureWorker::caliCordsByBed(int rbindex, double bed_dis, QVector<double> cords)
{
    if(g_CaliConfig_bed == nullptr)
    {
        double bias_x = (rbindex == 0 ? bed_dis : -bed_dis);
        QVector<double> ret = {cords[0] + bias_x, cords[1], cords[2]};
        return ret;
    }
    else
    {
        QString bot = (rbindex == 0 ? "left" : "right");
        auto obj = g_CaliConfig_bed->value(bot).toObject();
        auto bed_vec = obj.value("bed_vec").toDouble();
        auto rb_vec = obj.value("rb_vec").toArray();
        double bias_x = rb_vec[0].toDouble() * bed_dis / bed_vec;
        double bias_y = rb_vec[1].toDouble() * bed_dis / bed_vec;
        double bias_z = rb_vec[2].toDouble() * bed_dis / bed_vec;
        QVector<double> ret = {cords[0] + bias_x, cords[1] + bias_y, cords[2] + bias_z};
        return ret;
    }
}

double CoreProcedureWorker::getMidMean(vector<double> &vec)
{
    sort(vec.begin(), vec.end(), [=](double &a, double &b)->bool{return a < b;});
    double sum = 0;
    int cnt = 0;
    for(int i = vec.size() / 4; i < vec.size() * 3 / 4; i++)
    {
        sum += vec[i];
        cnt++;
    }
    return sum / cnt;
}
