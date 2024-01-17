shmem说明：
标志位(1 byte) + RGB数据(1280 * 720 * 3 byte) + 关键点数量(uint32) + 像素坐标(2 * uint16 * 1280 byte) + 相机color_3d坐标(3 * float * 1280 byte) 

标志位：
0, 相机进程返回rgb数据
1, 除0外返回xyz数据
2, 上位机读取rgb
3, 上位机读取关键点xyz
4, 保存当前帧点云数据到本地