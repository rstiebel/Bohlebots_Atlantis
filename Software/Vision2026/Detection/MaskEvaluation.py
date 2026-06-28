from Setup import *



########################################################################
#	Evaluation Helper Functions
########################################################################

def getHeightOfObject(classID):
	if classID == CLASS_IDS["Ball"]:
		return BALL_SIZE
	if classID == CLASS_IDS["Yellow"]:
		return GOAL_SIZE
	if classID == CLASS_IDS["Blue"]:
		return GOAL_SIZE
	return OBJECT_SIZE


def closest_nonzero_pixel(mask, origin):
    #
    # In: mask: binary mask (2D array, non-zero = foreground), origin: (x, y) tuple
    # Out: (x, y) of closest non-zero pixel
    #
    ox, oy = origin
    #
    # Get coordinates of all non-zero pixels: returns (row, col) = (y, x)
    ys, xs = np.nonzero(mask)
    #
    # Compute squared distance (avoid sqrt for speed, doesn't change argmin)
    dist_sq = (xs - ox) ** 2 + (ys - oy) ** 2
    idx = np.argmin(dist_sq)
    return (int(xs[idx]), int(ys[idx]))


def maskXYToPolarCoords(x, y, classID, camID):
	#
	# In: (x, y) coords as pixels (x left to right, y top to bottom)
	# Out: (angle, dist)
	#
	#
	# Convert downscaled coords to full image size
	f = MASK_DOWNSCALE_FACTORS_CLASSES[classID] if classID < len(MASK_DOWNSCALE_FACTORS_CLASSES) else max(MASK_DOWNSCALE_FACTORS_CLASSES)
	x *= float(f)
	y *= float(f)
	#
	# Fix distortion from non-square scalar crop
	#     img: 0 to 384
	#     sensor: 0 to scalar crop height
	#     want: 0 to full sensor height
	w_sensor_crop, h_sensor_crop = SCALER_CROP[-2:]
	y *= h_sensor_crop / w_sensor_crop
	#
	# Use camera matrix to get undistorted coordinates
	obj_point = np.array([[[x, y]]], dtype=np.float64)
	cam_matrix = CAMERA_MATRIX_FRONT if camID == CAM_ID_FRONT else CAMERA_MATRIX_BACK
	obj_undistorted_norm = cv2.omnidir.undistortPoints(obj_point, cam_matrix, DISTORTION_COEFFICIENTS, XI, R=CAM_TILT_MATRIX).squeeze() # [[[x,y]]] -> [x,y] CAM_TILT_MATRIX

	if abs(obj_undistorted_norm[0]) > 1e3 or abs(obj_undistorted_norm[1]) > 1e3:
		# print("Warning: undistortion failed, point outside valid area")
		return (128, 255)
	#
	# Compute where object would be from top-view
	obj_ground = -obj_undistorted_norm
	#
	# Compute polar coords
	dist = (obj_ground @ obj_ground)**0.5
	dist *= DIST_FACTOR
	angle_rad = np.arctan2(obj_ground[0], obj_ground[1])
	angle_norm = angle_rad / np.pi
	#
	# Convert into Arduino format
	dist_arduino = min(int(round(dist)), 250)
	angle_arduino = int(-angle_norm * 128.0 + 0.5)  # round() would round mathematically and not like we want
	return angle_arduino, dist_arduino


def blobPosition(mask, camID, maxBlobs, classID):
	#
	# In: mask, camID, maxBlobs
	# Out: [(angle1, dist1), (angle2, dist2), ...] up to maxBlobs or dynamic with all detections if maxBlobs < 0
	#		angleX in [0 ; 255] -> 0 = -180 degrees, 128 = 0 degrees, 255 = +180 degrees
	#		distX in [0 ; 200] if determined, 255 otherwise
	#
	if maxBlobs == 0:
		return []
	if not np.any(mask):
		return [(128, 255)] * maxBlobs if maxBlobs > 0 else []
	#
	# Find centroid for each connected component
	num_labels, labels, _, blob_centroids = cv2.connectedComponentsWithStats(mask, 4, cv2.CV_16U)
	# labels is an array same shape as mask, where each pixel has the label
	# (integer) of the component it belongs to. 0 = background.
	blob_centroids = np.round(blob_centroids)
	#
	# Ignore background
	blob_centroids = blob_centroids[1:]
	#
	# Ignore if there are too many components
	if maxBlobs > 0:
		blob_centroids = blob_centroids[:maxBlobs]
	#
	# Take closest object point to origin instead of blob center, because the distances would be messed up.
	# We do not know how tall a robot is in real life but the point where it stands on the ground is independent of that.
	if classID < 0 or classID >= len(CLASS_NAMES):
		blob_centroids = []
		for label in range(1, num_labels):  # skip 0 (background)
			component_mask = np.where(labels == label, 255, 0).astype(np.uint8)
			closest = closest_nonzero_pixel(component_mask, ORIGIN_OBJECT_MASK)
			blob_centroids.append(closest)
	#
	# Calculate coordinates
	h, w = mask.shape
	centroids_polar = [maskXYToPolarCoords(x, y, classID, camID) for (x,y) in blob_centroids]
	for i in range(len(centroids_polar)):
		a, d = centroids_polar[i]
		if camID == CAM_ID_BACK:
			#
			# We get a as from [-127; 128] for (-180°;180°]
			# If this is the back camera we need to rotate by 180°
			#
			a = (((a + 128) + 128) % 255) - 127
		a += 127
		centroids_polar[i] = (a,d)
	#
	# If a specific number is expected, append with code for not seen
	if maxBlobs > 0 and len(centroids_polar) < maxBlobs:
		centroids_polar.extend([(128, 255)] * (maxBlobs - len(centroids_polar)))

	return centroids_polar



def blobPositionsForClass(classID, maxBlobs, masks, camID):
	#
	# In: classID of what should be evaluated (= index of class name in CLASS_NAMES)
	#     as well as list of masks in sync with CLASS_NAMES + object mask appended and the camID
	# Out: [(angle1, dist1), (angle2, dist2), ...] up to maxBlobs or dynamic with all detections if maxBlobs < 0
	#
	if maxBlobs == 0:
		return []
	if classID < 0:
		#WARN(f"classID {classID} is invalid")
		return [(128, 255)] * maxBlobs if maxBlobs > 0 else []

	positions = blobPosition(masks[classID], camID, maxBlobs, classID)

	if maxBlobs < 0:   # dynamic length -> return all detected positions
		return [(a,d) for (a,d) in positions if d < 255]

	return positions



########################################################################
#	Evaluation
########################################################################
def evaluateMasks(masks, camID):
	#
	# Ball
	ballPositions = blobPositionsForClass(CLASS_IDS["Ball"], MAX_NUM_BALLS, masks, camID)
	#
	# Yellow
	yellowPositions = blobPositionsForClass(CLASS_IDS["Yellow"], MAX_NUM_YELLOW, masks, camID)
	#
	# Blue
	bluePositions = blobPositionsForClass(CLASS_IDS["Blue"], MAX_NUM_BLUE, masks, camID)
	#
	# Objects
	objectPositions = []
	for i in range(len(CLASS_NAMES), len(masks)):
		objectPositions += blobPositionsForClass(i, -1, masks, camID)
	#
	# Compose final tuple
	positions = [None for _ in CLASS_IDS]
	positions[CLASS_IDS["Ball"]] = ballPositions
	positions[CLASS_IDS["Yellow"]] = yellowPositions
	positions[CLASS_IDS["Blue"]] = bluePositions
	positions[CLASS_IDS["Object"]] = objectPositions
	return positions



########################################################################
#	Merging Helper Functions
########################################################################

last_positions = [None for _ in CLASS_IDS]

def mergePositionsHelper(classID, positionsFront, positionsBack, maxPositions):
	global last_positions
	
	if maxPositions < 0:   # dynamic length -> return all detected positions
		return [(a,d) for (a,d) in positionsFront if d < 255] + [(a,d) for (a,d) in positionsBack if d < 255]
		
	if maxPositions == 1:
		validPositions = positionsFront + positionsBack
		validPositions = [(a, d) for a,d in validPositions if d < 255]
		if len(validPositions) == 0:
			last_positions[classID] = None
			return [(128, 255)]
		valid_angles, valid_dists = zip(*validPositions)
		new_a = sum(valid_angles) / len(valid_angles)
		new_d = sum(valid_dists) / len(valid_dists)
		last_p = last_positions[classID]
		last_a = last_p[0] if last_p is not None else None
		last_d = last_p[1] if last_p is not None else None

		d = (0.9 * new_d) + (0.1 * last_d) if last_d is not None else new_d

		if last_a is not None and abs(new_a - last_a) > 127:
			if new_a < last_a:
				new_a += 255
			else:
				last_a += 255
		a = (0.9 * new_a) + (0.1 * last_a) if last_a is not None else new_a
		a %= 255
		last_positions[classID] = (a, d)
		return [(int(round(a)), int(round(d)))]
	else:
		validPositions = [(a,d) for (a,d) in positionsFront if d < 255] + \
						 [(a,d) for (a,d) in positionsBack if d < 255]
		# Nach Distanz sortieren
		validPositions.sort(key=lambda x: x[1])
		# Auf maxPositions begrenzen
		positionsMerged = validPositions[:maxPositions]
		# Falls weniger Positionen als maxPositions, mit Fallback auffüllen
		while len(positionsMerged) < maxPositions:
			positionsMerged.append((128, 255))  # default fallback
		return positionsMerged




########################################################################
#	Merging
########################################################################
def mergePositions(positionsFront, positionsBack):
	#
	# Ball
	class_id = CLASS_IDS["Ball"]
	ballPositionsMerged = mergePositionsHelper(class_id, positionsFront[class_id], positionsBack[class_id], MAX_NUM_BALLS)
	#
	# Yellow
	class_id = CLASS_IDS["Yellow"]
	yellowPositionsMerged = mergePositionsHelper(class_id, positionsFront[class_id], positionsBack[class_id], MAX_NUM_YELLOW)
	#
	# Blue
	class_id = CLASS_IDS["Blue"]
	bluePositionsMerged = mergePositionsHelper(class_id, positionsFront[class_id], positionsBack[class_id], MAX_NUM_BLUE)
	#
	# Objects
	class_id = CLASS_IDS["Object"]
	positionsObjectFront = positionsFront[class_id]
	positionsObjectBack  = positionsBack [class_id]
	l = max(len(positionsObjectFront), len(positionsObjectBack))
	objectPositionsMerged = mergePositionsHelper(-1, positionsObjectFront, positionsObjectBack, -1)
	#
	# Compose final list
	positionsMerged = [None for _ in CLASS_IDS]
	positionsMerged[CLASS_IDS["Ball"]] = ballPositionsMerged
	positionsMerged[CLASS_IDS["Yellow"]] = yellowPositionsMerged
	positionsMerged[CLASS_IDS["Blue"]] = bluePositionsMerged
	positionsMerged[CLASS_IDS["Object"]] = objectPositionsMerged
	return positionsMerged
