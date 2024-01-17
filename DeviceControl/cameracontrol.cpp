#include "cameracontrol.h"
#include <QFile>
#include <QDebug>
#include <QDir>
#include <QJsonArray>
#include <QFileDialog>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <QThread>
#include <QApplication>
#include <QDateTime>

using namespace std;

CameraControl::CameraControl(QString serialnum, int index, QObject *parent) : QObject(parent)
{
    m_serialnum = serialnum;
    m_index = index;
    m_frameData = nullptr;
    m_shmempt = nullptr;
    m_curePosture = "";
    m_framecnt = 0;
    m_discon_cnt = -1;

    QString exFileName1 = QString("extrinsics/extrinsic_%1_to_robot_%2.txt").arg(m_serialnum).arg(0);
    QString exFileName2 = QString("extrinsics/extrinsic_%1_to_robot_%2.txt").arg(m_serialnum).arg(1);
    if(QFile::exists(exFileName1))
    {
        m_extrinsics0 = nc::fromfile<double>(exFileName1.toStdString(), ' ');
        m_extrinsics0.reshape(4, 4);
    }
    if(QFile::exists(exFileName2))
    {
        m_extrinsics1 = nc::fromfile<double>(exFileName2.toStdString(), ' ');
        m_extrinsics1.reshape(4, 4);
    }

    m_encryptor = new SYEncryptor;
    m_encryptor->setPubKey("pubkey.pem");
}

CameraControl::~CameraControl()
{

}

void CameraControl::inishm()
{
    //标志位 + RGB + 深度
    int shm_size = 1 + IMAGE_WIDTH * IMAGE_HEIGHT * 5;
    if(m_shmempt == nullptr)
    {
        auto shkey = ftok(QString(QString("./camprocess/tmp") + QString::number(m_index + 1)).toStdString().data(), 1001 + m_index);
        int shmid = shmget(shkey, shm_size, 0666|IPC_CREAT);
        if (shmid == -1)
        {
            LOG_EXCEPTION_CAM << "shmget failed";
            qApp->exit(-1);
        }
        auto shmem = shmat(shmid, NULL, 0);
        if (shmem == (void *)-1)
        {
            LOG_EXCEPTION_CAM << "shmat failed";
            qApp->exit(-1);
        }
        m_shmempt = (unsigned char *)shmem;
    }
    memset(m_shmempt, 0, shm_size);
    *m_shmempt = 1;
}

void CameraControl::getImageFromshm()
{
    if(*m_shmempt == 2)
    {
        if(m_discon_cnt == -1)
            emit switchDevSta(m_index+2);
        m_discon_cnt = 0;
        //提取图像数据
        m_mutex.lock();
        if(m_frameData == nullptr)
            m_frameData = (quint8*)malloc(IMAGE_WIDTH * IMAGE_HEIGHT * 5);
        memcpy(m_frameData, m_shmempt + 1, IMAGE_WIDTH * IMAGE_HEIGHT * 5);
        *m_shmempt = 1;
        auto raw = cv::Mat(IMAGE_HEIGHT, IMAGE_WIDTH, CV_8UC3, m_frameData);
        auto bgr_mat = raw.clone();
        m_mutex.unlock();
        //调理期间保存图片
        auto posture = getCurePosture();
        if(!g_usingImg && posture != "" && m_framecnt == 15 * 5)//5s一存
        {
            QString path = g_saveImgPath + QString("saveimgs/CureRecords/") + QDateTime::currentDateTime().toString("yyyyMMdd") + "/" + posture + "/color/";
            QString filename = path + g_cameraOrder[m_index] + "_" + posture + "_" + QDateTime::currentDateTime().toString("yyyyMMddHHmmsszzz") + ".jpg";
            QDir dir;
            if(!dir.mkpath(path))
                LOG_EXCEPTION_CAM << m_index << "create path error" << path;
            if(g_runArgs["EnableEncryption"] == "true")
            {
                std::vector<uchar> buffer;
                cv::imencode(".jpg", bgr_mat, buffer);
                std::vector<uchar> buffer_encrypted;
                m_encryptor->encrypt(buffer, buffer_encrypted);
                QFile file(filename);
                file.open(QFile::WriteOnly);
                file.write((char*)buffer_encrypted.data(), buffer_encrypted.size());
                file.close();
            }
            else
            {
                if(!cv::imwrite(filename.toStdString(), bgr_mat))
                    LOG_EXCEPTION_CAM << m_index << "save cure img error";
            }
        }
        if(m_framecnt >= 15 * 5)
            m_framecnt = 0;
        else
            m_framecnt++;
        //更新至UI视频流
        QImage img(bgr_mat.data, IMAGE_WIDTH, IMAGE_HEIGHT, QImage::Format_RGB888);
        auto imgcp = img.rgbSwapped().convertToFormat(QImage::Format_ARGB32);//videooutput控件支持该格式
        if(m_index == g_cameraOrder.indexOf('r'))
            imgcp = imgcp.mirrored(true);
        QSharedPointer<QImage> pimg(new QImage(imgcp));
        emit frameArrived(pimg);
//        emit frameArrived_s(pimg);
    }
    else if(*m_shmempt == 3)
    {
        if(m_discon_cnt == -1)
            emit switchDevSta(m_index+2);
        m_discon_cnt = 0;
        memcpy(&m_intrinsics_fx, m_shmempt + 1, sizeof(float));
        memcpy(&m_intrinsics_fy, m_shmempt + 1 + sizeof(float), sizeof(float));
        memcpy(&m_intrinsics_cx, m_shmempt + 1 + sizeof(float) * 2, sizeof(float));
        memcpy(&m_intrinsics_cy, m_shmempt + 1 + sizeof(float) * 3, sizeof(float));
        LOG_INFO << m_index << " intrins: " <<  m_intrinsics_fx << " " <<  m_intrinsics_fy << " " <<  m_intrinsics_cx << " " <<  m_intrinsics_cy;
        *m_shmempt = 1;
    }
    else
    {
        if(m_discon_cnt != -1)
        {
            m_discon_cnt++;
            if(m_discon_cnt >= 20)
            {
                m_discon_cnt = -1;
                if(!g_testEMC)
                {
                    emit switchDevSta(m_index+2);
                    restartCamProcess();
                }
            }
        }
    }
}

QSharedPointer<quint8> CameraControl::getBGRData()
{
    QSharedPointer<quint8> ret = nullptr;
    if(g_usingImg)
    {
        QSharedPointer<QString> file(new QString(""));
        emit getOpenFile(QString(g_cameraOrder[m_index]), "./", "*.jpg", file);
        if(*file == "")
            return nullptr;
        cv::Mat mat;
        if(g_runArgs["EnableEncryption"] == "true")
        {
            QFile tmpfile(*file);
            tmpfile.open(QFile::ReadOnly);
            auto bytes = tmpfile.readAll();
            tmpfile.close();
            std::vector<uchar> tmp;
            tmp.insert(tmp.end(), (uchar*)bytes.data(), (uchar*)bytes.data() + bytes.size());
            SYDecryptor sy_decryptor;
            sy_decryptor.setKey("key.pem", "sy123456");
            std::vector<uchar> buffer_decrypted;
            sy_decryptor.decrypt(tmp, buffer_decrypted);
            mat = cv::imdecode(buffer_decrypted, cv::IMREAD_COLOR);
            if (mat.empty())
            {
                LOG_EXCEPTION_CAM << "Failed to decode image";
                return nullptr;
            }
        }
        else
        {
            mat = cv::imread(file->toStdString());
            if(mat.empty())
            {
                LOG_EXCEPTION_CAM << "jpg read empty";
                return nullptr;
            }
        }
        ret = QSharedPointer<quint8>((quint8*)malloc(IMAGE_WIDTH * IMAGE_HEIGHT * 3));
        memcpy(ret.data(), mat.data, IMAGE_WIDTH * IMAGE_HEIGHT * 3);
    }
    else
    {
        m_mutex.lock();
        if(m_frameData != nullptr)
        {
            ret = QSharedPointer<quint8>((quint8*)malloc(IMAGE_WIDTH * IMAGE_HEIGHT * 3));
            memcpy(ret.data(), m_frameData, IMAGE_WIDTH * IMAGE_HEIGHT * 3);
        }
        m_mutex.unlock();
        if(ret == nullptr)
            return nullptr;
    }
    return ret;
}

QSharedPointer<quint8> CameraControl::getDepthData()
{
    QSharedPointer<quint8> ret = nullptr;
    if(g_usingImg)
    {
        QSharedPointer<QString> file(new QString(""));
        emit getOpenFile(QString(g_cameraOrder[m_index]), "./", "*.png", file);
        if(*file == "")
            return nullptr;
        cv::Mat mat;
        if(g_runArgs["EnableEncryption"] == "true")
        {
            QFile tmpfile(*file);
            tmpfile.open(QFile::ReadOnly);
            auto bytes = tmpfile.readAll();
            tmpfile.close();
            std::vector<uchar> tmp;
            tmp.insert(tmp.end(), (uchar*)bytes.data(), (uchar*)bytes.data() + bytes.size());
            SYDecryptor sy_decryptor;
            sy_decryptor.setKey("key.pem", "sy123456");
            std::vector<uchar> buffer_decrypted;
            sy_decryptor.decrypt(tmp, buffer_decrypted);
            mat = cv::imdecode(buffer_decrypted, cv::IMREAD_ANYDEPTH);
            if (mat.empty())
            {
                LOG_EXCEPTION_CAM << "Failed to decode image";
                return nullptr;
            }
        }
        else
        {
            mat = cv::imread(file->toStdString(), CV_16UC1);
            if(mat.empty())
            {
                LOG_EXCEPTION_CAM << "png read empty";
                return nullptr;
            }
        }
        ret = QSharedPointer<quint8>((quint8*)malloc(IMAGE_WIDTH * IMAGE_HEIGHT * 2));
        memcpy(ret.data(), mat.data, IMAGE_WIDTH * IMAGE_HEIGHT * 2);
    }
    else
    {
        m_mutex.lock();
        if(m_frameData != nullptr)
        {
            ret = QSharedPointer<quint8>((quint8*)malloc(IMAGE_WIDTH * IMAGE_HEIGHT * 2));
            memcpy(ret.data(), m_frameData + IMAGE_WIDTH * IMAGE_HEIGHT * 3, IMAGE_WIDTH * IMAGE_HEIGHT * 2);
        }
        m_mutex.unlock();
        if(ret == nullptr)
            return nullptr;
    }
    return ret;
}

void CameraControl::cam2dToCam3d(double pix_x, double pix_y, double d, double &x, double &y)
{
    x = (pix_x - m_intrinsics_cx) / m_intrinsics_fx * d;
    y = (pix_y - m_intrinsics_cy) / m_intrinsics_fy * d;
}

void CameraControl::cam3dToRobot3d(int rbIndex, double &x, double &y, double &z)
{
    auto matrix = getExtrinsics(rbIndex);
    nc::NdArray<double> one = {1};
    auto camera_3d = nc::asarray<double>({x, y, z});
    camera_3d = nc::concatenate({camera_3d, one}, nc::Axis::COL);
    camera_3d.reshape(4, 1);
    auto target_3d = nc::dot<double>(matrix, camera_3d);
    x = target_3d[0] / 1000;
    y = target_3d[1] / 1000;
    z = target_3d[2] / 1000;
}

void CameraControl::caliFix(int rbIndex, double &x, double &y, double &z)
{
    //自动矫正参数
    if(g_CaliConfig != nullptr)
    {
        auto caliargs = g_CaliConfig->value(QString(g_cameraOrder[m_index])).toObject();
        if((rbIndex == 0 && x > 0) || (rbIndex == 1 && x < 0))
        {
            if(caliargs.contains("tail"))
            {
                double s = caliargs.value("tail").toDouble();
                x *= s;
            }
        }
        else
        {
            if(caliargs.contains("head"))
            {
                double s = caliargs.value("head").toDouble();
                x *= s;
            }
        }
    }
    //人工矫正参数
    if(g_CaliConfig_Manual != nullptr)
    {
        auto cfg_arr = g_CaliConfig_Manual->value(rbIndex == 0 ? "left" : "right").toArray();
        for(int i = 0; i < cfg_arr.size(); i++)
        {
            auto cfg_obj = cfg_arr[i].toObject();
            double xlow = cfg_obj["x_low"].toDouble();
            double xhigh = cfg_obj["x_high"].toDouble();
            auto cam_str = cfg_obj["cam"].toString();
            if(x >= xlow && x <= xhigh && g_cameraOrder[m_index] == cam_str)
            {
                double bias_x1 = cfg_obj["bias_x"].toArray().at(0).toDouble();
                double bias_x2 = cfg_obj["bias_x"].toArray().at(1).toDouble();
                double bias_y1 = cfg_obj["bias_y"].toArray().at(0).toDouble();
                double bias_y2 = cfg_obj["bias_y"].toArray().at(1).toDouble();
                double bias_z1 = cfg_obj["bias_z"].toArray().at(0).toDouble();
                double bias_z2 = cfg_obj["bias_z"].toArray().at(1).toDouble();
                x += bias_x1 + (bias_x2 - bias_x1) * (x - xlow) / (xhigh - xlow);
                y += bias_y1 + (bias_y2 - bias_y1) * (x - xlow) / (xhigh - xlow);
                z += bias_z1 + (bias_z2 - bias_z1) * (x - xlow) / (xhigh - xlow);
                break;
            }
        }
    }
}

void CameraControl::updateExtrinsics(nc::NdArray<double> ex, int rbindex)
{
    m_extrinsics_mutex.lock();
    if(rbindex == 0)
        m_extrinsics0 = ex;
    else
        m_extrinsics1 = ex;
    m_extrinsics_mutex.unlock();
}

bool CameraControl::checkExtrinsics()
{
    QString exFileName1 = QString("extrinsics/extrinsic_%1_to_robot_%2.txt").arg(m_serialnum).arg(0);
    QString exFileName2 = QString("extrinsics/extrinsic_%1_to_robot_%2.txt").arg(m_serialnum).arg(1);
    m_extrinsics_mutex.lock();
    auto empty = (m_extrinsics0.isempty() || m_extrinsics1.isempty());
    m_extrinsics_mutex.unlock();
    return QFile::exists(exFileName1) && QFile::exists(exFileName2) && !empty;
}

nc::NdArray<double> CameraControl::getExtrinsics(int rbindex)
{
    nc::NdArray<double> tmp;
    m_extrinsics_mutex.lock();
    if(rbindex == 0)
        tmp = m_extrinsics0;
    else
        tmp = m_extrinsics1;
    m_extrinsics_mutex.unlock();
    return tmp;
}

double CameraControl::getDepthValue(int pix_x, int pix_y, QSharedPointer<quint8> pdat)
{
    auto dat = pdat.data();
    auto d = ((uint16_t*)dat)[pix_y * IMAGE_WIDTH + pix_x];
    if(d == 0)
    {
        for(int i = pix_x - 2; i <= pix_x + 2; i++)
            for(int j = pix_y - 2; j <= pix_y + 2; j++)
            {
                if(i < 0 || i > IMAGE_WIDTH - 1 || j < 0 || j > IMAGE_HEIGHT - 1)
                    continue;
                d = ((uint16_t*)dat)[j * IMAGE_WIDTH + i];
                if(d != 0)
                    return d;
            }
    }
    return d;
}

double CameraControl::getEstimateDepthValue(int pix_x, int pix_y, QSharedPointer<quint8> pdat)
{
    auto dat = pdat.data();
    double d = 49999;
    //取100*100范围内最小深度
    for(int i = pix_x - 50; i <= pix_x + 49; i++)
        for(int j = pix_y - 50; j <= pix_y + 49; j++)
        {
            if(i < 0 || i > IMAGE_WIDTH - 1 || j < 0 || j > IMAGE_HEIGHT - 1)
                continue;
            auto tmpd = ((uint16_t*)dat)[j * IMAGE_WIDTH + i];
            if(tmpd != 0 && tmpd < d)
                d = tmpd;
        }
    if(d > 40000)
        d = 0;
    return d;
}

void CameraControl::savePic(QString path, QString filename, QSharedPointer<quint8> pdat, int type)
{
    //创建文件夹
    QDir dir(path);
    if(!dir.exists())
    {
        QProcess cmd;
        cmd.start("mkdir", {"-p", path});
        cmd.waitForFinished();
    }
    //保存
    auto file = path + filename;
    auto mat = cv::Mat(IMAGE_HEIGHT, IMAGE_WIDTH, type, pdat.data());
    if(g_runArgs["EnableEncryption"] == "true")
        saveEncodeMat(file, mat);
    else
        cv::imwrite(file.toStdString(), mat);
}

void CameraControl::saveEncodeMat(QString file, cv::Mat &mat)
{
    std::vector<uchar> buffer;
    if(file.endsWith(".jpg"))
        cv::imencode(".jpg", mat, buffer);
    else
        cv::imencode(".png", mat, buffer);
    SYEncryptor sy_encryptor;
    sy_encryptor.setPubKey("pubkey.pem");
    std::vector<uchar> buffer_encrypted;
    sy_encryptor.encrypt(buffer, buffer_encrypted);
    QFile f(file);
    f.open(QFile::WriteOnly);
    f.write((char*)buffer_encrypted.data(), buffer_encrypted.size());
    f.close();
}

bool CameraControl::checkBedCollision()
{
    bool ret = false;
    m_mutex.lock();
    for(int i = 120; i < 128; i++)
    {
        for(int j = 20; j < 60; j++)
        {
            auto pix_x = i * 10;
            auto pix_y = j * 10;
            auto tmpptr = (uint16_t*)(m_frameData + IMAGE_HEIGHT * IMAGE_WIDTH * 3);
            auto d = tmpptr[pix_y * IMAGE_WIDTH + pix_x];
            if(d < 800)
            {
                ret = true;
                break;
            }
        }
        if(ret)
            break;
    }
    m_mutex.unlock();
    return ret;
}
