#ifndef REALTIMEMONCLIENT_H
#define REALTIMEMONCLIENT_H

#include <QWebSocket>
#include "Comm/baseclient.h"

// ClientInterfaces_Realtime.pdf
// 3. RealTime 5.10
// tcp server port 30003
struct RealtimeRobotStatus {
  // page 1,15 items
  qint32 messageSize; // 0+4 //Total message length in bytes
  qreal time;         // 8 //Time elapsed since the controller was started
  qreal qTarget[6];   // 48 //Target joint positions
  qreal qdTarget[6];  // 48  //Target joint velocities
  qreal qddTarget[6]; // 48 //Target joint accelerations
  qreal iTarget[6];   // 48  //Target joint currents
  qreal mTarget[6];   // 48  //Target joint moments (torques)
  qreal qActual[6];   // 48  //Actual joint positions
  qreal qdActual[6];  // 48  //Actual joint velocities
  qreal iActual[6];   // 48  //Actual joint currents
  qreal iControl[6];  // 48  //Joint control currents
  qreal toolVectorActual[6]; // 48//Actual Cartesian coordinates of the tool:
                             // (x,y,z,rx,ry,rz), where rx, ry and rz
  // is a rotation vector representation of the tool orientation
  qreal tcpSpeedActual[6]; // 48  //Actual speed of the tool given in Cartesian
                           // coordinates
  qreal tcpForce[6];       // 48  //Generalised forces in the TCP
  qreal toolVectorTarget[6]; // 48  //Target Cartesian coordinates of the tool:
                             // (x,y,z,rx,ry,rz), where rx, ry and rz
  // is a rotation vector representation of the tool orientation
  // page 2,14 items
  qreal tcpSpeedTarget[6]; // 48  //Target speed of the tool given in Cartesian
                           // coordinates
  qreal digitalInputBits;  // 8  //Current state of the digital inputs. NOTE:
                           // these are bits encoded as
  // int64_t, e.g. a value of 5 corresponds to bit 0 and bit 2 set high
  qreal motorTemperatures[6]; // 48//Temperature of each joint in degrees
                              // celsius
  qreal controllerTimer;      // 8//Controller realtime thread execution time
  qreal testValue; // 8//A value used by Universal Robots software only
  qreal robotMode; // 8//Robot mode
  //     Description
  //        -1 ROBOT_MODE_NO_CONTROLLER
  //        0 ROBOT_MODE_DISCONNECTED
  //        1 ROBOT_MODE_CONFIRM_SAFETY
  //        2 ROBOT_MODE_BOOTING
  //        3 ROBOT_MODE_POWER_OFF
  //        4 ROBOT_MODE_POWER_ON
  //        5 ROBOT_MODE_IDLE
  //        6 ROBOT_MODE_BACKDRIVE
  //        7 ROBOT_MODE_RUNNING
  //        8 ROBOT_MODE_UPDATING_FIRMWARE
  qreal jointModes[6]; // 8//Joint control modes
  //     Description
  //        0 CONTROL_MODE_POSITION
  //        1 CONTROL_MODE_TEACH
  //        2 CONTROL_MODE_FORCE
  //        3 CONTROL_MODE_TORQUE

  qreal safetyModes; // 8//Safety mode
                     //     Description
                     //        SAFETY_MODE_UNDEFINED_SAFETY_MODE 11
                     //        SAFETY_MODE_VALIDATE_JOINT_ID 10
                     //        SAFETY_MODE_FAULT 9
                     //        SAFETY_MODE_VIOLATION 8
  //        SAFETY_MODE_ROBOT_EMERGENCY_STOP 7 (EA + EB + SBUS->Euromap67)
  //        Physical e-stop interface input activated
  //        SAFETY_MODE_SYSTEM_EMERGENCY_STOP 6 (EA + EB + SBUS->Screen)
  //        Physical e-stop interface input activated SAFETY_MODE_SAFEGUARD_STOP
  //        5 (SI0 + SI1 + SBUS) Physical s-stop interface input
  //        SAFETY_MODE_RECOVERY 4
  //        SAFETY_MODE_PROTECTIVE_STOP 3
  //        SAFETY_MODE_REDUCED 2
  //        SAFETY_MODE_NORMAL 1
  qreal testValue2[6]; // 48//A value used by Universal Robots software only
  qreal toolAccelerometerValues[3]; // 24 //Tool x,y and z accelerometer values
                                    // (software version 1.7)
  qreal testValue3[6]; // 48//A value used by Universal Robots software only
  qreal speedScaling;  // 8 //Speed scaling of the trajectory limiter
  qreal linearMomentumNorm; // 8//Norm of Cartesian linear momentum
  qreal testValue4;         // 8//A value used by Universal Robots software only
  // page 3 ,16 item
  qreal testValue5;       // 8//A value used by Universal Robots software only
  qreal vMain;            // 8 //Masterboard: Main voltage
  qreal vRobot;           // 8 //Masterboard: Robot voltage (48V)
  qreal iRobot;           // 8 //Masterboard: Robot current
  qreal vActual[6];       // 48//Actual joint voltages
  qreal digitalOutputs;   // 8//Digital outputs
  qreal programState;     // 8 //Program state
  qreal elbowPosition[3]; // 24 //Elbow position
  qreal elbowVelocity[3]; // 24 //Elbow velocity
  qreal safetyStatus;     // 8//Safety mode

  //        SAFETY_STATUS_SYSTEM_THREE_POSITION_ENABLING_STOP 13
  //        SAFETY_STATUS_AUTOMATIC_MODE_SAFEGUARD_STOP 12
  //        SAFETY_STATUS_UNDEFINED_SAFETY_MODE 11
  //        SAFETY_STATUS_VALIDATE_JOINT_ID 10
  //        SAFETY_STATUS_FAULT 9
  //        SAFETY_STATUS_VIOLATION 8
  //        SAFETY_STATUS_ROBOT_EMERGENCY_STOP 7 (EA + EB + SBUS->Euromap67)
  //        Physical e- stop interface input activated
  //        SAFETY_STATUS_SYSTEM_EMERGENCY_STOP 6 (EA + EB + SBUS->Screen)
  //        Physical e-stop interface input activated
  //        SAFETY_STATUS_SAFEGUARD_STOP 5 (SI0 + SI1 + SBUS) Physical s-stop
  //        interface input
  //        SAFETY_STATUS_RECOVERY 4
  //        SAFETY_STATUS_PROTECTIVE_STOP 3
  //        SAFETY_STATUS_REDUCED 2
  //        SAFETY_STATUS_NORMAL 1
  qreal testValue6;        // 8//A value used by Universal Robots software only
  qreal testValue7;        // 8//A value used by Universal Robots software only
  qreal testValue8;        // 8//A value used by Universal Robots software only
  qreal payloadMass;       // 8 //Payload Mass [kg]
  qreal payloadCoG[3];     // 24 //Payload Center of Gravity (x, y, z) [m]
  qreal payloadInertia[6]; // 48 //Payload Inertia (Ixx, Iyy, Izz, Ixy, Ixz,
                           // Iyz) [kg*m^2]
  //  TOTAL 152*8(double)+4(int)=1220 bytes
};

class RealtimeMonClient : public BaseClient {
    Q_OBJECT
public:
  explicit RealtimeMonClient(QString ip, QObject *parent = nullptr);

  void processMsgRcv() override;
  RealtimeRobotStatus getRobotStatus();

signals:
  void reduceForce();
  void recoverForce(QString);
  void enableFreeDrive(bool);

public slots:
  void connectToPlot();
  void setProgState(bool flag){m_progStateFlag = flag;}
  void setSoftProtectiveStop(bool flag){m_softProtectiveStopFlag = flag;}
  void setAutoArgs(QVector<double> p1, QVector<double> p2);

private:
  RealtimeRobotStatus m_rbInfo;
  QMutex m_statusMutex; //加锁机械臂状态信息
  qreal m_lastTS;//上一包数据的时间戳
  QAtomicInteger<bool> m_reduceFlag;//是否已发出减力信号
  QVector<QList<double>> m_tcplist;//历史tcp缓存队列
  QVector<QList<double>> m_spdlist;//历史速度缓存队列
  QWebSocket *m_plot_client;//将30003数据发给plot工具解析
  QAtomicInteger<bool> m_plot_connected;//是否已连接plot
  int m_updateCnt;//数据未更新计数，200ms未更新判断为故障的

  QVector<double> m_auto_startpt;
  QVector<double> m_auto_endpt;
  int m_recover_cnt;
  QVector<double> m_last_toolpos;
  QMutex m_auto_mutex;

  //实时数据反馈监测处理开关
  bool m_instantStopFlag;//机械臂io中止
  bool m_freeDriveFLag;//机械臂io自由驱动
  bool m_safetyModesFlag;//安全模式错误
  QAtomicInteger<bool> m_progStateFlag;//机械臂程序运行状态
  QAtomicInteger<bool> m_softProtectiveStopFlag;//力控软保护

  QVector<double> getAvgTcp(double x, double y, double z);//获取最近的平均tcp值
  QVector<double> getAvgSpd(double x, double y, double z);//获取最近的平均spd值
};

#endif // REALTIMEMONCLIENT_H
