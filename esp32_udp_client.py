# 映像データを送信する

import socket
import time
import cv2
import os
import sys
import math
from PIL import Image

# udp設定
sendAddr = ('esp32側でIPアドレスを確認', 1234)  # ポート番号は1234
udp = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)


# 送信する映像データ作成
NUMPIXELS = 24  # LEDの数
Div = 100  # １周の分割数   # arduinoのDivと合わせる
Bright = 30  # 輝度
Led0Bright = 10  # 中心LEDの輝度 [%]
unit = 1  # 送信列数

gif_file_name = '好きなgif画像・mp4動画'



def polarConv(imgOrgin):
    # 画像データ読み込み
    # imgOrgin = cv2.imread(pic)

    # 画像サイズ取得
    # 参考：https://note.nkmk.me/python-opencv-pillow-image-size/
    h, w, _ = imgOrgin.shape

    # 画像縮小
    # 参考：https://www.tech-tech.xyz/opecv_resize.html
    imgRedu = cv2.resize(imgOrgin, (math.floor(
        (NUMPIXELS * 2 - 1)/h * w), NUMPIXELS * 2 - 1))

    # 縮小画像中心座標
    h2, w2, _ = imgRedu.shape
    wC = math.floor(w2 / 2)
    hC = math.floor(h2 / 2)

    # 極座標変換
    k = 0
    for l in range(0, 1):
        # while True: だとだめ
        for j in range(0, Div):
            data = '%02X' % j
            for i in range(0, hC+1):
                # 座標色取得
                # 参考：http://peaceandhilightandpython.hatenablog.com/entry/2016/01/03/151320
                rP = int(imgRedu[hC + math.ceil(i * math.cos(2*math.pi/Div*j)),
                                 wC - math.ceil(i * math.sin(2*math.pi/Div*j)), 2]
                         * ((100 - Led0Bright) / NUMPIXELS * i + Led0Bright) / 100 * Bright / 100)
                gP = int(imgRedu[hC + math.ceil(i * math.cos(2*math.pi/Div*j)),
                                 wC - math.ceil(i * math.sin(2*math.pi/Div*j)), 1]
                         * ((100 - Led0Bright) / NUMPIXELS * i + Led0Bright) / 100 * Bright / 100)
                bP = int(imgRedu[hC + math.ceil(i * math.cos(2*math.pi/Div*j)),
                                 wC - math.ceil(i * math.sin(2*math.pi/Div*j)), 0]
                         * ((100 - Led0Bright) / NUMPIXELS * i + Led0Bright) / 100 * Bright / 100)
                data += '%02X%02X%02X' % (rP, gP, bP)
            k += 1
            if k == unit:
                k = 0
                udp.sendto(data.encode('utf-8'), sendAddr)
                time.sleep(0.001) #sleepがないとパケットロスが激増する
            
                    

# GIF変換
while True:
    gif = cv2.VideoCapture(gif_file_name)

    while True:
        is_success, frame = gif.read()

        # ファイルが読み込めなくなったら終了
        if not is_success:
            break

        # 変換
        polarConv(frame)
        # time.sleep(0.01)
        if(count%12==0): #12枚ごとに表示する(表示速度調整)
            # 変換
            polarConv(frame)
        count += 1

# Ctrl + Cで終了
udp.close()
