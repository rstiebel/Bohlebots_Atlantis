import cv2
from picamera2 import Picamera2

image_folder: str = "./Images/"


frame_num = 0
CAM_ID_FRONT = 0
CAM_ID_BACK = 1
CAM_FPS = 60.0
IMG_SIZE = (384, 384) # is used for ai and postprocessing
IMG_SIZE_CAPTURE =  (1944, 1944) # (622, 480)# (h,w) -> this size is only used for getting the frame for better quality
SCALER_CROP = (324, 0, 1944, 1944)

# da Ursprung oben links ist, müssen die y werte vor dem entzeren durch 1,296 geteilt werden (1944/1500)

def initCams():
	#
	# Cam0
	#
	cam = None
	try:
		cam = Picamera2(camera_num=CAM_ID_FRONT)
	except Exception as e:
		print(f"Failed to initialize camF:\n{e}")
	if cam is not None:
		config = cam.create_preview_configuration(main={"format": "RGB888", "size": IMG_SIZE_CAPTURE})
		cam.configure(config)
		cam.set_controls({"ScalerCrop": SCALER_CROP, "FrameRate": CAM_FPS})
		cam.start()
	return cam



cam = initCams()

while True:
	img = cam.capture_array('main')

	if img is not None:
		cv2.imshow("frame", img)
		key = cv2.waitKey(1) & 0xFF

	if key == ord('p'):
		cv2.imwrite(f"{image_folder}frame_{frame_num}.png", img)
		frame_num += 1
		print(f"Saved Image {frame_num}")
	elif key == ord('q'):
		break

cv2.destroyAllWindows()
