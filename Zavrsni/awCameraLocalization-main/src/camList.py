import cv2

def list_cameras():
    available_cameras = []
    for i in range(10):  # Check first 10 device indices
        cap = cv2.VideoCapture(i)
        if cap.isOpened():
            available_cameras.append(i)
            cap.release()
    return available_cameras

cameras = list_cameras()
if cameras:
    print("Available cameras:", cameras)
else:
    print("No cameras found.")
