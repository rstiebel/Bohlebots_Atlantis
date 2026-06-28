from Setup import *



########################################################################
#	Helper Functions
########################################################################
def downscaleMaxPooling(img, factor):
    kernel = DOWNSCALE_DILATION_KERNELS[factor]
    dilated = cv2.dilate(img, kernel)
    return np.ascontiguousarray(dilated[::factor, ::factor])


def fillWallMask(mask):
    #
    # Idea:
    #     Find the single connected block of black pixels that crosses the middle axis.
    #     If multiple such blocks exist, keep the bottom-most one (highest y).
    #     Set everything else to white.
    #     Before connected components: draw in white the convex hull of all white points
    #     and the most bottom row regardless of color.
    h, w = mask.shape
    # Collect all white points and all points of the bottom row
    white_pts = np.column_stack(np.where(mask == 255))[:, ::-1]  # (x, y)
    bottom_pts = np.column_stack(([0, w - 1], [h - 1, h - 1])).T  # left and right of bottom row
    pts = np.vstack([white_pts, bottom_pts]).astype(np.float32)
    hull = cv2.convexHull(pts)
    cv2.polylines(mask, [hull.astype(np.int32)], isClosed=True, color=255, thickness=1)
    # Find crossing labels by sampling the middle column, bottom-to-top, take first unique hit
    black_mask = (mask == 0).astype(np.uint8)
    num_labels, labels = cv2.connectedComponents(black_mask, connectivity=4)
    mid_col = labels[:, w // 2]
    crossing_labels = mid_col[mid_col != 0]  # Exclude background
    if len(crossing_labels) == 0:
        mask[:, :] = 255
        return mask
    best_label = crossing_labels[-1]  # Last = bottom-most row that crosses mid
    mask[labels != best_label] = 255
    return mask


def fixSeparatedGoal(goalMasks):
	if len(goalMasks) == 0:
		return None
	if len(goalMasks) == 1:
		return goalMasks[0]

	fixedMask = np.zeros(goalMasks[0].shape, dtype=np.uint8)
	allPoints = []
	for mask in goalMasks:
		ys, xs = np.where(mask == 255)
		points = np.vstack((xs, ys)).T
		allPoints.append(points)
	allPoints = np.vstack(allPoints)
	hull = cv2.convexHull(allPoints)
	cv2.fillConvexPoly(fixedMask, hull, 255)
	return fixedMask


def filterOnlyAllowed(mask, allowedAreaMask, camID, enabled):
	entry = BOT_POLYGONS.get(camID)
	if entry is None:
		return mask

	points = entry[enabled] if isinstance(entry, dict) else entry

	h, w = mask.shape[:2]
	scale = np.array([
		w / IMG_SIZE[0],
		h / IMG_SIZE[1],
	], dtype=np.float32)
	pts = (points * scale).astype(np.int32)
	cv2.fillPoly(mask, [pts.reshape(-1, 1, 2)], 0)
	
	mask &= cv2.resize(allowedAreaMask, (w, h), interpolation=cv2.INTER_NEAREST)

	return mask


########################################################################
#	Preperation
########################################################################
def prepareMasks(masks, classIDs, confidences, camID):
	#
	# In: result with list of any detections
	# Out: list of 1 mask per class + 1 additional mask for objects
	#
	def zeroMask(classID):
		f = MASK_DOWNSCALE_FACTORS_CLASSES[classID] if classID < len(MASK_DOWNSCALE_FACTORS_CLASSES) else max(MASK_DOWNSCALE_FACTORS_CLASSES)
		return np.zeros((IMG_SIZE[0] // f, IMG_SIZE[1] // f), dtype=np.uint8)

	finalMasks = [zeroMask(i) for i in range(len(CLASS_NAMES))]

	if len(masks) < 1 or len(classIDs) < 1 or len(confidences) < 1:
		return finalMasks

	yellowMasks = []
	blueMasks = []
	rawForObject = np.zeros((IMG_SIZE[0], IMG_SIZE[1]), dtype=np.uint8)

	#
	# Sort masks into classes
	#
	for mask, classID, confidence in zip(masks, classIDs, confidences):
		if confidence < MIN_CONFIDENCE_TO_CONSIDER:
			continue
		maskImg = mask * 255
		
		rawForObject |= maskImg
		#
		# Scale down with max pooling, not lose any detection
		f = MASK_DOWNSCALE_FACTORS_CLASSES[classID]
		maskImg = downscaleMaxPooling(maskImg, f)
		#
		# Combine all instances of the same class
		finalMasks[classID] |= maskImg
		#
		# Store seperated goal masks for potential reconstruction
		#
		if CLASS_NAMES[classID] == "Yellow":
			yellowMasks.append(maskImg)
		if CLASS_NAMES[classID] == "Blue":
			blueMasks.append(maskImg)
	
	#
	# Fill wall to the top
	#
	class_id = CLASS_IDS["Wall"]
	finalMasks[class_id] = fillWallMask(finalMasks[class_id])
	allowedAreaMask = np.bitwise_not(finalMasks[class_id])
	
	#
	# Construct object mask with size of smallest other mask
	#
	rawForObject = downscaleMaxPooling(rawForObject, MASK_DOWNSCALE_FACTORS_CLASSES[class_id])
	rawForObject = cv2.bitwise_not(rawForObject)
	finalMasks.append(rawForObject)
	
	#
	# Ignore own bot in all masks
	#
	for class_id, enabled in CLASS_IDS_TO_IGNORE_BOT:
		finalMasks[class_id] = filterOnlyAllowed(finalMasks[class_id], allowedAreaMask, camID, enabled)
		
	#
	# Fix maybe separated goals
	#
	if len(yellowMasks) > 1:
		finalMasks[CLASS_IDS["Yellow"]] = fixSeparatedGoal(yellowMasks)
	if len(blueMasks) > 1:
		finalMasks[CLASS_IDS["Blue"]] = fixSeparatedGoal(blueMasks)
	return finalMasks
