"""
功能：用其他视角（head）的深度值补偿主视角（middle）缺失的深度值。
Args:
    深度缺失的图像坐标值（x,y)
    intrinsic (list): 相机的内参矩阵
    extrinsics (list): 相机的外参矩阵

Returns:
    补偿的深度值 z
"""

import numpy as np
import time
import sys


# 将点云数据从相机坐标系转换到基座坐标系。
def pc2basematrix(pcd, extrinsics):
    one = np.ones(len(pcd))
    matric1 = np.array(extrinsics).reshape(4, 4)
    pcd_homo = np.insert(np.array(pcd), 3, one, axis=1)
    pc_base = (np.dot(matric1, pcd_homo.T)).T
    return pc_base


# 将点云数据转换到中间相机坐标系。
def pc2inv_middlematrix(pcd, extrinsics):
    matrix_m = np.array(extrinsics).reshape(4, 4)  # 顶部视角外参矩阵
    matrix_m_inv = np.linalg.inv(matrix_m)
    camera_3d_2T = (np.dot(matrix_m_inv, pcd.T)).T
    pc_middle = camera_3d_2T[:, :3]
    return pc_middle


# 根据相机内参将点云数据投影到图像平面上，得到深度图像
def pc2depth_2(pc, cx, cy, fx, fy):
    depth_reconstructed = np.zeros((720, 1280))

    x = ((pc[:, 0] * fx / pc[:, 2]) + cx).astype(int)
    y = ((pc[:, 1] * fy / pc[:, 2]) + cy).astype(int)

    valid_indices = np.where((x >= 0) & (x < 1280) & (y >= 0) & (y < 720))
    x_valid = x[valid_indices]
    y_valid = y[valid_indices]
    pc_valid = pc[valid_indices]

    depth_reconstructed[y_valid, x_valid] = np.maximum(depth_reconstructed[y_valid, x_valid], pc_valid[:, 2])

    return depth_reconstructed

# 深度图融合
def dep_fusion(reconstructed_imgs,sy,sx):
    middle = reconstructed_imgs[0]
    head = reconstructed_imgs[1]
    left = reconstructed_imgs[2]
    right = reconstructed_imgs[3]
    # 顶部为主视角，四视角深度图合并， 顺序：头部，左侧，右侧。·
    middle_max = np.max(middle)
    if middle[sy , sx] == 0:
        if head[sy , sx] != 0 and head[sy, sx] <= 1400:
            middle[sy , sx] = head[sy, sx]
        elif left[sy , sx] != 0 and left[sy, sx] <= 1400:
            middle[sy ,sx] = left[sy, sx]
        elif right[sy , sx] != 0 and right[sy, sx] <= 1400:
            middle[sy , sx] = right[sy, sx]
        else:
            area40 = middle[max(sy  -10, 150):min(sy +10,550),
                     max(sx -40, 750):min(sx+1,1050)]
            middle[sy , sx] = np.mean(area40[area40 != 0])

    # savepng_16("fusion_2in1_vertor.png", middle)
    return middle[sy , sx]


# 主函数：4视角深度融合
def fusion4in1(depthMap_head, depthMap_left, depthMap_right, depthMap_middle, intrinsics, extrinsics,sy,sx):
    reconstructed_imgs = [np.asarray(depthMap_middle).reshape(720, 1280)]
    for angle in ["head","left","right"]:
        intrinsic = intrinsics[angle]
        fx = intrinsic[0]
        fy = intrinsic[4]
        cx = intrinsic[6]
        cy = intrinsic[7]
        if angle == "head":
            pcd = np.asarray(depthMap_head).reshape(-1, 3)
        elif angle == "left":
            pcd = np.asarray(depthMap_left).reshape(-1, 3)
        elif angle == "right":
            pcd = np.asarray(depthMap_right).reshape(-1, 3)
        # 转至基座坐标系
        pcd_base = pc2basematrix(pcd, extrinsics[angle])
        # 转至中间相机坐标系
        pc_middle = pc2inv_middlematrix(pcd_base, extrinsics["middle"])
        # 点云转深度图
        depth_reconstructed = pc2depth_2(pc_middle, cx, cy, fx, fy)
        reconstructed_imgs.append(depth_reconstructed)
    img_fusion = dep_fusion(reconstructed_imgs,sy,sx)
    return img_fusion

def readdepth(file):
    data = np.fromfile(file, dtype=np.int16)
    aa = data[2::3]
    return aa.tolist()

def readpc(file):
    data = np.fromfile(file, dtype=np.int16)
    return data.tolist()

def l_filling(depth_left,sy, sx):
    depth_left1 = np.asarray(depth_left).reshape(720, 1280)
    if depth_left1[sy][sx] == 0:
        area40 = depth_left1[max(sy - 20, 250):min(sy +20, 500),
                         max(sx- 40, 750):min(sx + 1, 1000)]
        fillpoint =  np.mean(area40[area40 != 0])
    else:
        fillpoint = depth_left1[sy][sx]
    return fillpoint

def r_filling(depth_right,sy, sx):
    depth_right1 = np.asarray(depth_right).reshape(720, 1280)
    if depth_right1[sy][sx] == 0:
        area40 = depth_right1[max(sy - 20, 250):min(sy +20, 500),
                         max(sx- 40, 750):min(sx + 1, 1000)]
        fillpoint =  np.mean(area40[area40 != 0])
    else:
        fillpoint = depth_right1[sy][sx]
    return fillpoint

def h_filling(depth_head,sy, sx):
    depth_head1 = np.asarray(depth_head).reshape(720, 1280)
    if depth_head1[sy][sx] == 0:
        area40 = depth_head1[max(sy - 20, 250):min(sy +20, 500),
                         max(sx- 40, 750):min(sx + 1, 1000)]
        fillpoint =  np.mean(area40[area40 != 0])
    else:
        fillpoint = depth_head1[sy][sx]
    return fillpoint

if __name__ == '__main__':
    t1 = time.time()
    camorder = ["middle", "head", "left", "right"]
    pixs = sys.argv[1]
    files = []
    for i in range(4):
        files.append(sys.argv[i + 2])
    intrinsics = {}
    for i in range(4):
        intrinsics[camorder[i]] = np.asarray(list(map(float, sys.argv[i + 6].split(','))))
    extrinsics = {}
    for i in range(4):
        extrinsics[camorder[i]] = np.asarray(list(map(float, sys.argv[i + 10].split(','))))
    
    pix_list = pixs.split('#')
    for i in range(len(pix_list)):
        pix_info = pix_list[i].split(',')
        sx = int(pix_info[1])
        sy = int(pix_info[2])
        flag_cam = pix_info[0]
        if flag_cam == "m":
            img_fusion = fusion4in1(readpc(files[1]), readpc(files[2]), readpc(files[3]), readdepth(files[0]), intrinsics,
                                    extrinsics,int(sy), int(sx))
        elif flag_cam == "l":
            img_fusion = l_filling(readdepth(files[2]),int(sy), int(sx))
        elif flag_cam == "r":
            img_fusion = r_filling(readdepth(files[3]),int(sy), int(sx))
        elif flag_cam == "h":
            img_fusion = h_filling(readdepth(files[1]),int(sy), int(sx))
        print(img_fusion, end=',')
    
    # print(time.time() - t1)
