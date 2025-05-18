import cv

# Define the RTP stream URL (replace with your actual RTP stream parameters)
url = "rtp://@192.168.2.1:5600"

# Create a VideoCapture object
cap = cv.VideoCapture(url, cv.CAP_FFMPEG)

# Check if the camera opened successfully
if not cap.isOpened():
    print("Error: Could not open the stream.")
    exit()

# Read and display frames from the stream
while True:
    ret, frame = cap.read()
    if not ret:
        print("Error: Failed to capture frame.")
        break
    
    # Display the frame
    cv2.imshow('RTP Stream', frame)
    
    # Check for 'q' key to exit
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Release the VideoCapture object and close any OpenCV windows
cap.release()
cv2.destroyAllWindows()
