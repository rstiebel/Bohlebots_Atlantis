from Setup import *
camF, camB = initCams()
camIDForCalibration = 0

points = []

def getFrame(cam):
    img = cam.capture_array('main')
    img = cv2.resize(img, IMG_SIZE, interpolation=cv2.INTER_NEAREST)
    return img


def click_event(event, x, y, flags, params):
    if event == cv2.EVENT_LBUTTONDOWN:
        print(x, y)
        points.append([x, y])

if __name__ == "__main__":
    while True:
        if camIDForCalibration == CAM_ID_FRONT:
            frame = getFrame(camF)
        elif camIDForCalibration == CAM_ID_BACK:
            frame = getFrame(camB)
        else:
            print("Chose 0 for Front cam or 1 for Back cam")

        cv2.imshow(f"Frame {camIDForCalibration}", frame)
        cv2.setMouseCallback(f"Frame {camIDForCalibration}", click_event)
        key = cv2.waitKey(1) & 0xFF
        if key == ord('q'):
            break

np_points = np.array(points, dtype=np.float32)
print(points)
cv2.destroyAllWindows()
