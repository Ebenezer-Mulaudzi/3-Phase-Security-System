import cv2
import os
import random
import serial 
import numpy as np
import face_recognition
import firebase_admin
from firebase_admin import credentials, db

# Initialize Firebase Admin SDK with your service account credentials
cred = credentials.Certificate("serviceAccountKey.json")
firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://student-register-3249a-default-rtdb.firebaseio.com/'
})

# Reference to the Realtime Database root
ref = db.reference('Students')

# Load known faces and their corresponding names from a directory
known_faces = []
known_face_names = []
known_faces_dir = "Images/"

arduino_port = 'COM24'  # Change this to your Arduino's port
baud_rate = 9600

arduino = serial.Serial(arduino_port, baud_rate)

for filename in os.listdir(known_faces_dir):
    if filename.endswith(".jpeg"):
        face_image = face_recognition.load_image_file(known_faces_dir + filename)
        encoding = face_recognition.face_encodings(face_image)[0]
        known_faces.append(encoding)
        known_face_names.append(filename.split(".")[0])

# Initialize webcam (change the index if you have multiple cameras)
cap = cv2.VideoCapture(0)

while True:
    ret, frame = cap.read()
    small_frame = cv2.resize(frame, (0, 0), fx=0.25, fy=0.25)
    face_locations = face_recognition.face_locations(small_frame)
    face_encodings = face_recognition.face_encodings(small_frame, face_locations)

    for (top, right, bottom, left), face_encoding in zip(face_locations, face_encodings):
        name = "Unknown"

        # Compare the face to the known faces
        matches = face_recognition.compare_faces(known_faces, face_encoding)
        face_distances = face_recognition.face_distance(known_faces, face_encoding)
        best_match_index = np.argmin(face_distances)

        if matches[best_match_index]:
            name = known_face_names[best_match_index]

            finger_ID = arduino.readline().decode('utf-8').strip()
            print("Received data:", finger_ID)

            if finger_ID.startswith("M"):
                global matched_id 
                matched_id = int(finger_ID[1:])
                print(f"Matched ID: {matched_id}")
                print(type(matched_id))

                ref = db.reference('Students')


            # Search for data in the Realtime Database using the name
            data = ref.child(name).get()
            if data:
                #print(f"Name: {name}, Data: {data}")
                fingerID = int(data['fingerprint'])
                print(f"fingerID: {fingerID}")
                print(type(fingerID))
               
                

                if matched_id == fingerID:
                        
                    code = ''.join(random.choices('0123456789', k=4))
                    code += "\n"

                    arduino.write(code.encode('utf-8'))
                    print(code)

            else:
                print(f"Name: {name}, No data found in the database")

        # Draw a rectangle and label on the face
        top *= 4
        right *= 4
        bottom *= 4
        left *= 4
        cv2.rectangle(frame, (left, top), (right, bottom), (0, 0, 255), 2)
        font = cv2.FONT_HERSHEY_DUPLEX
        cv2.putText(frame, name, (left + 6, bottom - 6), font, 0.5, (255, 255, 255), 1)

    cv2.imshow('Video', frame)

    key = cv2.waitKey(1) & 0xFF
    if key == ord('q'):
        break


