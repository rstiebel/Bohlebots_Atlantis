from Setup import *
from Test import liveVisualization, liveCalibration, benchmarkPipeline
from Detection import *
from time import sleep
import degirum as dg
import psutil
import os


def initModel():
    model = dg.load_model(model_name='yolov8n_seg',inference_host_address='@local',zoo_url='./model_zoo_custom/yolov8n_seg.json')
    model(np.zeros((IMG_SIZE[0], IMG_SIZE[1], 3), dtype=np.uint8))   # Needed as the first call takes a while, which messes up starting the other processes
    return model



def feedAI(model, camF, camB):
    #
    # Capture
    imgF = camF.capture_array('main')
    imgB = camB.capture_array('main')
    #
    # Resize
    imgF = cv2.resize(imgF, IMG_SIZE, interpolation=cv2.INTER_NEAREST)
    imgB = cv2.resize(imgB, IMG_SIZE, interpolation=cv2.INTER_NEAREST)
    #
    # Inference
    resF, resB = (x.results for x in model.predict_batch([imgF, imgB]))
    #
    # Serialize
    masksF, classIDsF, confidencesF = serializeAIResults(resF)
    masksB, classIDsB, confidencesB = serializeAIResults(resB)

    masksF = prepareMasks(masksF, classIDsF, confidencesF, CAM_ID_FRONT)
    masksB = prepareMasks(masksB, classIDsB, confidencesB, CAM_ID_BACK)

    positionsF = evaluateMasks(masksF, CAM_ID_FRONT)
    positionsB = evaluateMasks(masksB, CAM_ID_BACK)

    positions = mergePositions(positionsF, positionsB)
    sendToESP(positions)
    
def main(model, camF, camB):
    try:
        while True:
            feedAI(model, camF, camB)
    except KeyboardInterrupt:
        print("Programm wird beendet (Ctrl+C)")

########################################################################
#	Main Entry Point
########################################################################
if __name__ == "__main__":
    #
    # Init
    initARGS()
    ARGS = getARGS()
    camF, camB = initCams()
    model = initModel()
    #
    # Visualization
    if ARGS.visualize:
        liveVisualization(model, camF, camB)
    #
    # Calibration
    if ARGS.calibrate:
        liveCalibration(camF, camB)
    #
    # Benchmark
    benchmarkPipeline(model, ARGS.benchmark_runs, camF, camB)
    #
    # Main
    if not ARGS.no_main:
        main(model, camF, camB)
