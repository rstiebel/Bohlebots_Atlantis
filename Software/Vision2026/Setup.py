
import numpy as np
import cv2
from picamera2 import Picamera2
import argparse
import serial
import yaml

########################################################################
#	Constants
########################################################################
CAM_ID_FRONT = 0
CAM_ID_BACK = 1
CAM_FPS = 60.0
IMG_SIZE = (384, 384) # is used for ai and postprocessing
IMG_SIZE_CAPTURE =  (1260, 972) # (622, 480)# (h,w) -> this size is only used for getting the frame for better quality
SCALER_CROP = (324, 0, 1944, 1500)
CLASS_NAMES                    = ("Ball", "Blue", "Field", "Wall", "Yellow")
MASK_DOWNSCALE_FACTORS_CLASSES = (  4   ,   4    , 16    ,   4   ,   4     , 4) # last one for objects
CLASS_IDS_TO_IGNORE_BOT = [(0, False), (1, False), (2, False), (3, False), (4, False), (-1, True)] # -1 for objects
DOWNSCALE_DILATION_KERNELS = {f: np.ones((f, f), np.uint8) for f in set(MASK_DOWNSCALE_FACTORS_CLASSES)}
CLASS_IDS = {name: class_id for class_id, name in enumerate(list(CLASS_NAMES) + ["Object"])}
MAX_NUM_BALLS = 1
MAX_NUM_YELLOW = 1
MAX_NUM_BLUE = 1
MAX_NUM_OBJECTS = 3
MIN_CONFIDENCE_TO_CONSIDER = 0.5
DIST_FACTOR = (100.0 / 8.22)
COLORS_BGR = { "Ball": [0, 0, 255], "Field": [0, 128, 0], "Wall": [64, 0, 64], "Yellow": [0, 255, 224], "Blue": [255, 0, 0], "Object": [224, 0, 224] }
__ARGS = None


with open("CalibrationPoint.yaml") as f:
	calibrationPointConfig = yaml.safe_load(f)

CALIBRATION_POINT_PERFECT = (IMG_SIZE[0] // 2, (IMG_SIZE[1] * 11) // 12)
CALIBRATION_POINT_CURRENT_FRONT = tuple(calibrationPointConfig["calibration_point_front"])
CALIBRATION_POINT_CURRENT_BACK  = tuple(calibrationPointConfig["calibration_point_back"])


CALIBRATION_OFFSET_FRONT = (CALIBRATION_POINT_CURRENT_FRONT[0] - CALIBRATION_POINT_PERFECT[0], CALIBRATION_POINT_CURRENT_FRONT[1] - CALIBRATION_POINT_PERFECT[1])
CALIBRATION_OFFSET_BACK  = (CALIBRATION_POINT_CURRENT_BACK [0] - CALIBRATION_POINT_PERFECT[0], CALIBRATION_POINT_CURRENT_BACK [1] - CALIBRATION_POINT_PERFECT[1])

ORIGIN_OBJECT_MASK = (int((CALIBRATION_POINT_PERFECT[0] - 1) / MASK_DOWNSCALE_FACTORS_CLASSES[-1]), int(CALIBRATION_POINT_PERFECT[1] / MASK_DOWNSCALE_FACTORS_CLASSES[-1]))
#
# Camera
#
_fx = IMG_SIZE[0] / 2.0
_fy = IMG_SIZE[1] / 2.0
_cx = IMG_SIZE[0] / 2.0
_cy = IMG_SIZE[1] / 2.0
CAMERA_MATRIX_PERFECT = np.array([[ _fx, 0.0, _cx ],
                                  [ 0.0, _fy, _cy ],
                                  [ 0.0, 0.0, 1.0 ]])
CAMERA_MATRIX_FRONT = np.array([[ _fx, 0.0, _cx + CALIBRATION_OFFSET_FRONT[0] ],
                                [ 0.0, _fy, _cy + CALIBRATION_OFFSET_FRONT[1] ],
                                [ 0.0, 0.0,                               1.0 ]])
CAMERA_MATRIX_BACK = np.array([[ _fx, 0.0, _cx + CALIBRATION_OFFSET_BACK[0] ],
                               [ 0.0, _fy, _cy + CALIBRATION_OFFSET_BACK[1] ],
                               [ 0.0, 0.0,                              1.0 ]])


XI = np.array([0.74895006])

DISTORTION_COEFFICIENTS = np.array([[-0.22190681, 0.02699571, 0.00252793, 0.00116042]])

CAMERA_TILT_DEG = 38 #38 # deg
CAMERA_TILT_RAD = np.radians(CAMERA_TILT_DEG) # rad
CAMERA_HEIGHT_CM = 15 # cm

CAM_TILT_COS, CAM_TILT_SIN = np.cos(CAMERA_TILT_RAD), np.sin(CAMERA_TILT_RAD)
CAM_TILT_MATRIX = np.array([[1,            0,             0],
                            [0, CAM_TILT_COS, -CAM_TILT_SIN],
                            [0, CAM_TILT_SIN,  CAM_TILT_COS]])

GOAL_SIZE = 10 # cm
BALL_SIZE = 4.2 # cm
OBJECT_SIZE = 18 # cm

ser = serial.Serial("/dev/ttyAMA0", baudrate=115200, timeout=1)


POLYGON_CENTER = [IMG_SIZE[0] // 2, IMG_SIZE[1]]
# für gegner würde man vorne mehr wegnehmen wegen dribbler (FRONT_POLYGON_ENABLED) -> da wir aktuell aber vorne alles abdecken können gibt es keinen Unterschied
FRONT_POLYGON_ENABLED = np.array([[-102, 0], [-100, -20], [-90, -20], [-85, -45], [-62, -73], [-67, -80], [-47, -97], [48, -97], [68, -80], [63, -73], [86, -45], [91, -20], [101, -20], [103, 0]], dtype=np.float32)
FRONT_POLYGON_ENABLED += POLYGON_CENTER

FRONT_POLYGON_DISABLED = np.array([[-102, 0], [-100, -20], [-90, -20], [-85, -45], [-62, -73], [-67, -80], [-47, -92], [48, -92], [68, -80], [63, -73], [86, -45], [91, -20], [101, -20], [103, 0]], dtype=np.float32)
FRONT_POLYGON_DISABLED += POLYGON_CENTER

BACK_POLYGON = np.array([[-107, 0], [-97, -53], [-82, -52], [-65, -75], [-45, -100], [-20, -112], [21, -112], [46, -100], [66, -75], [83, -52], [98, -53], [108, 0]], dtype=np.float32)
BACK_POLYGON += POLYGON_CENTER

BOT_POLYGONS = {
    CAM_ID_FRONT: {
        True:  FRONT_POLYGON_ENABLED,
        False: FRONT_POLYGON_DISABLED,
    },

    CAM_ID_BACK: BACK_POLYGON
}

########################################################################
#	Logging
########################################################################
def PRINT(message):
	if __ARGS is not None and __ARGS.quiet:
		return
	print(f"\033[3m\033[97m{message}\033[0m") # Italic bright white message

def INFO(message):
	if __ARGS is not None and __ARGS.quiet:
		return
	print(f"\033[1;34m[INFO]\033[0m \033[3;94m{message}\033[0m")   # Bold blue label, italic bright blue message

def SUCCESS(message):
	if __ARGS is not None and __ARGS.quiet:
		return
	print(f"\033[1;32m[SUCCESS]\033[0m \033[3;92m{message}\033[0m")   # Bold green label, italic bright green message

def WARN(message):
	if __ARGS is not None and __ARGS.quiet:
		return
	print(f"\033[1;33m[WARNING]\033[0m \033[3;93m{message}\033[0m")   # Bold yellow label, italic bright yellow message

def ERROR(message):
	if __ARGS is not None and __ARGS.quiet:
		return
	print(f"\033[1;31m[ERROR]\033[0m \033[3;91m{message}\033[0m")   # Bold red label, italic bright red message



########################################################################
#	CLI Arguments
########################################################################
def initARGS():
	parser = argparse.ArgumentParser(description="Atlantis Vision System of 2025/2026", epilog="Order of execution:\n  (1) Visualization\n  (2) Benchmarking\n  (3) Main", formatter_class=argparse.RawTextHelpFormatter)
	parser.add_argument('--quiet', '-q', action='store_true', default=False, help='Quiet mode (no prints)')
	parser.add_argument('--visualize', '-v', action='store_true', default=False, help='Enter a visualization in which the final masks are overlayed to the original images')
	parser.add_argument('--calibrate', '-c', action='store_true', default=False, help='Show camera feed with calibration overlay')
	parser.add_argument('--benchmark-runs', type=int, default=0, help='Number of benchmark runs to average over (default 0)')
	parser.add_argument('--no-main', action='store_true', default=False, help='Disable main execution')

	global __ARGS
	__ARGS = parser.parse_args()

def getARGS():
	return __ARGS



########################################################################
#	Camera Initialization
########################################################################
def initCams():
	#
	# Cam0
	#
	camF = None
	try:
		camF = Picamera2(camera_num=CAM_ID_FRONT)
	except Exception as e:
		ERROR(f"Failed to initialize camF:\n{e}")
	if camF is not None:
		config = camF.create_preview_configuration(main={"format": "RGB888", "size": IMG_SIZE_CAPTURE})
		camF.configure(config)
		camF.set_controls({"ScalerCrop": SCALER_CROP, "FrameRate": CAM_FPS})
		camF.start()
	#
	# Cam1
	#
	camB = None
	try:
		camB = Picamera2(camera_num=CAM_ID_BACK)
	except Exception as e:
		ERROR(f"Failed to initialize camB:\n{e}")
	if camB is not None:
		config = camB.create_preview_configuration(main={"format": "RGB888", "size": IMG_SIZE_CAPTURE})
		camB.configure(config)
		camB.set_controls({"ScalerCrop": SCALER_CROP, "FrameRate": CAM_FPS})
		camB.start()
	#
	# Distortion
	#
	return camF, camB


########################################################################
#	Utility functions
########################################################################
def serializeAIResults(res):
	if res is None:
		return np.array([], dtype=np.uint8), np.array([], dtype=np.uint8), np.array([], dtype=np.uint8)

	masks = []
	classIDs = []
	confidences = []
	for det in res:
		x_min = det["mask"]["x_min"]
		y_min = det["mask"]["y_min"]
		data = det["mask"]["data"].astype(np.uint8)
		h, w = data.shape
		mask = np.zeros(IMG_SIZE, dtype=np.uint8)
		mask[y_min:y_min+h, x_min:x_min+w] = data
		masks.append(mask)
		classIDs.append(det["category_id"])
		confidences.append(det["score"])
	return masks, classIDs, confidences



def sendToESP(positions):
	# SUCCESS(f"positions: {positions}")
	# SUCCESS(f"ball: {positions[CLASS_IDS['Ball']]}")
	# SUCCESS(f"yellow: {positions[CLASS_IDS['Yellow']]}")
	# SUCCESS(f"blue: {positions[CLASS_IDS['Blue']]}")
	# SUCCESS(f"object: {positions[CLASS_IDS['Object']]}")
	data = bytearray([255, 255])

	balls = positions[CLASS_IDS["Ball"]]
	yellow = positions[CLASS_IDS["Yellow"]]
	blue = positions[CLASS_IDS["Blue"]]
	objects = positions[CLASS_IDS["Object"]]

	numOfObjects = len(objects)
	if numOfObjects > MAX_NUM_OBJECTS:
		# ERROR("Number of objects is too high")
		numOfObjects = MAX_NUM_OBJECTS

	data.append(numOfObjects)

	for category in (balls, yellow, blue):
		for angle, dist in category:
			data.append(angle & 0xFF)
			data.append(dist & 0xFF)

	for angle, dist in objects[:numOfObjects]:
		data.append(angle & 0xFF)
		data.append(dist & 0xFF)

	ser.write(data)
