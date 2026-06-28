from Setup import *
from Detection import *
from time import time, sleep



########################################################################
#	Benchmarking
########################################################################
def benchmarkPipeline(model, numRuns, camF, camB):
	if numRuns < 1:
		return
	if camF is None or camB is None:
		ERROR("Cannot perform benchmarking without cameras")
		return

	PRINT(f"Starting benchmarking with {numRuns} runs for each category:")

	INFO("Cam capture blocking...")
	imgF_org = None
	imgB_org = None
	start = time()   # START benchamrking
	for _ in range(numRuns):
		imgF_org = camF.capture_array()
		imgB_org = camB.capture_array()
	end = time()    # END benchmarking
	SUCCESS(f"Result: {end - start:.3f}s for {numRuns} runs => {((end - start) * 1000.0 / numRuns):.3f}ms/run on average")
	capture_wait_time = (end - start) / numRuns

	INFO("Cam capture pure...")
	imgF_org = None
	imgB_org = None
	diff = 0.0
	for _ in range(numRuns):
		sleep(capture_wait_time)
		start = time()   # START benchamrking
		imgF_org = camF.capture_array()
		imgB_org = camB.capture_array()
		end = time()    # END benchmarking
		diff += end - start
	SUCCESS(f"Result: {diff:.3f}s for {numRuns} runs => {(diff * 1000.0 / numRuns):.3f}ms/run on average")

	INFO("Resizing...")
	imgF = None
	imgB = None
	start = time()   # START benchamrking
	for _ in range(numRuns):
		imgF = cv2.resize(imgF_org, IMG_SIZE, interpolation=cv2.INTER_NEAREST)
		imgB = cv2.resize(imgB_org, IMG_SIZE, interpolation=cv2.INTER_NEAREST)
	end = time()    # END benchmarking
	SUCCESS(f"Result: {end - start:.3f}s for {numRuns} runs => {((end - start) * 1000.0 / numRuns):.3f}ms/run on average")


	INFO("AI model and serialize...")
	dataF = None
	#dataB = None
	resF, resB = (x.results for x in model.predict_batch([imgF, imgB])) # First is always delayed
	start = time()   # START benchamrking
	for _ in range(numRuns):
		#resF = model(imgF).results
		#resB = model(imgB).results
		resF, resB = (x.results for x in model.predict_batch([imgF, imgB]))
		dataF = serializeAIResults(resF)
		dataB = serializeAIResults(resB)
	end = time()    # END benchmarking
	SUCCESS(f"Result: {end - start:.3f}s for {numRuns} runs => {((end - start) * 1000.0 / numRuns):.3f}ms/run on average")

	INFO("Mask preparation...")
	masksF = None
	masksB = None
	start = time()   # START benchamrking
	for _ in range(numRuns):
		masksF = prepareMasks(dataF[0], dataF[1], dataF[2], CAM_ID_FRONT)
		masksB = prepareMasks(dataB[0], dataB[1], dataB[2], CAM_ID_BACK)
	end = time()    # END benchmarking
	SUCCESS(f"Result: {end - start:.3f}s for {numRuns} runs => {((end - start) * 1000.0 / numRuns):.3f}ms/run on average")

	INFO("Mask evaluation...")
	positionsF = None
	positionsB = None
	start = time()   # START benchamrking
	for _ in range(numRuns):
		positionsF = evaluateMasks(masksF, CAM_ID_FRONT)
		positionsB = evaluateMasks(masksB, CAM_ID_BACK)
	end = time()    # END benchmarking
	SUCCESS(f"Result: {end - start:.3f}s for {numRuns} runs => {((end - start) * 1000.0 / numRuns):.3f}ms/run on average")

	INFO("Position merging and send to esp...")
	start = time()   # START benchamrking
	for _ in range(numRuns):
		positions = mergePositions(positionsF, positionsB)
		sendToESP(positions)
	end = time()    # END benchmarking
	SUCCESS(f"Result: {end - start:.3f}s for {numRuns} runs => {((end - start) * 1000.0 / numRuns):.3f}ms/run on average")


	PRINT(f"Starting combined single process benchmarking for {numRuns} runs:")
	start = time()   # START benchamrking
	for _ in range(numRuns):
		imgF_org = camF.capture_array()
		imgB_org = camB.capture_array()
		imgF = cv2.resize(imgF_org, IMG_SIZE, interpolation=cv2.INTER_NEAREST)
		imgB = cv2.resize(imgB_org, IMG_SIZE, interpolation=cv2.INTER_NEAREST)
		resF, resB = (x.results for x in model.predict_batch([imgF, imgB]))
		masksF, classIDsF, confidencesF = serializeAIResults(resF)
		masksB, classIDsB, confidencesB = serializeAIResults(resB)
		masksF = prepareMasks(masksF, classIDsF, confidencesF, CAM_ID_FRONT)
		masksB = prepareMasks(masksB, classIDsB, confidencesB, CAM_ID_BACK)
		positionsF = evaluateMasks(masksF, CAM_ID_FRONT)
		positionsB = evaluateMasks(masksB, CAM_ID_BACK)
		positions = mergePositions(positionsF, positionsB)
		sendToESP(positions)
	end = time()    # END benchmarking
	SUCCESS(f"Result: {end - start:.3f}s for {numRuns} runs => {((end - start) * 1000.0 / numRuns):.3f}ms/run on average")



########################################################################
#	Visualization
########################################################################
def visualizeFrame(model, camF, camB):
	img0 = camF.capture_array() if camF is not None else cv2.imread("frame_001353_blue.jpg")
	img1 = camB.capture_array() if camB is not None else cv2.imread("frame_012234_yellow.jpg")
	img0 = cv2.resize(img0, IMG_SIZE, interpolation=cv2.INTER_NEAREST)
	img1 = cv2.resize(img1, IMG_SIZE, interpolation=cv2.INTER_NEAREST)
	img0VisualizeRes = model(img0)
	img1VisualizeRes = model(img1)
	res0 = model(img0).results
	res1 = model(img1).results
	masks0, classIDs0, confidences0 = serializeAIResults(res0)
	masks1, classIDs1, confidences1 = serializeAIResults(res1)
	masks0 = prepareMasks(masks0, classIDs0, confidences0, CAM_ID_FRONT)
	masks1 = prepareMasks(masks1, classIDs1, confidences1, CAM_ID_BACK)
	positions0 = evaluateMasks(masks0, CAM_ID_FRONT)
	positions1 = evaluateMasks(masks1, CAM_ID_BACK)
	#cv2.imshow("Front Camera", img0)
	#cv2.imshow("Back Camera", img1)
	cv2.imshow("Front Camera AI", img0VisualizeRes.image_overlay)
	cv2.imshow("Back Camera", img1VisualizeRes.image_overlay)
	positions = mergePositions(positions0, positions1)
	sendToESP(positions)


def liveVisualization(model, camF, camB):
	ARGS = getARGS()
	while(True):
		PRINT("Working...")
		visualizeFrame(model, camF, camB)

		#INFO("Press <Space> to continue with next live frame")
		if ARGS.benchmark_runs > 0:
			INFO("Press 'q' or 'Q' to start benchmarking")
		elif not ARGS.no_main:
			INFO("Press 'q' or 'Q' to start execution of main program")
		else:
			INFO("Press 'q' or 'Q' to quit")


		key = cv2.waitKey(1) & 0xFF
		if key in [ord('q'), ord('Q')]:
			break

	PRINT("Closing images...")
	cv2.destroyAllWindows()


def liveCalibration(camF, camB):
	ARGS = getARGS()
	while(True):
		img0 = camF.capture_array() if camF is not None else cv2.imread("frame_001353_blue.jpg")
		img1 = camB.capture_array() if camB is not None else cv2.imread("frame_012234_yellow.jpg")
		img0 = cv2.resize(img0, IMG_SIZE, interpolation=cv2.INTER_NEAREST)
		img1 = cv2.resize(img1, IMG_SIZE, interpolation=cv2.INTER_NEAREST)

		x, y = CALIBRATION_POINT_PERFECT
		cv2.line(img0, (x-3, y  ), (x+3, y  ), (255, 0, 255), 1)
		cv2.line(img0, (x  , y-3), (x  , y+3), (255, 0, 255), 1)
		cv2.line(img1, (x-3, y  ), (x+3, y  ), (255, 0, 255), 1)
		cv2.line(img1, (x  , y-3), (x  , y+3), (255, 0, 255), 1)
		
		entry1 = BOT_POLYGONS.get(1)
		if entry1 is None:
			return img1

		points1 = entry1[False] if isinstance(entry1, dict) else entry1

		h, w = img1.shape[:2]
		scale = np.array([
			w / IMG_SIZE[0],
			h / IMG_SIZE[1],
		], dtype=np.float32)
		pts = (points1 * scale).astype(np.int32)
		cv2.polylines(img1, [pts.reshape(-1, 1, 2)], True, (255, 0, 255), 1)
		
		entry0 = BOT_POLYGONS.get(0)
		if entry0 is None:
			return img0

		points0 = entry0[True] if isinstance(entry0, dict) else entry0

		h, w = img0.shape[:2]
		scale = np.array([
			w / IMG_SIZE[0],
			h / IMG_SIZE[1],
		], dtype=np.float32)
		pts = (points0 * scale).astype(np.int32)
		cv2.polylines(img0, [pts.reshape(-1, 1, 2)], True, (255, 0, 255), 1)
		
		points0 = entry0[False] if isinstance(entry0, dict) else entry0

		h, w = img0.shape[:2]
		scale = np.array([
			w / IMG_SIZE[0],
			h / IMG_SIZE[1],
		], dtype=np.float32)
		pts = (points0 * scale).astype(np.int32)
		cv2.polylines(img0, [pts.reshape(-1, 1, 2)], True, (255, 0, 255), 1)
		
		cv2.imshow("img Front", img0)
		cv2.imshow("img Back", img1)


		#INFO("Press <Space> to continue with next live frame")
		if ARGS.benchmark_runs > 0:
			INFO("Press 'q' or 'Q' to start benchmarking")
		elif not ARGS.no_main:
			INFO("Press 'q' or 'Q' to start execution of main program")
		else:
			INFO("Press 'q' or 'Q' to quit")


		key = cv2.waitKey(1) & 0xFF
		if key in [ord('q'), ord('Q')]:
			break

	PRINT("Closing images...")
	cv2.destroyAllWindows()
