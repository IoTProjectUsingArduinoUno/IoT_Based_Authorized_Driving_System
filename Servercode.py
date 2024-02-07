import firebase_admin
from firebase_admin import credentials, storage
import cv2
from io import BytesIO
import numpy as np
import requests
import time
#from google.colab import drive
#drive.mount('/content/drive')
#from flask import Flask
#app = Flask(__name__)
# Initialize Firebase
cred = credentials.Certificate("dlfpdb-firebase-adminsdk-y3fdt-0d7872bf79.json")
firebase_admin.initialize_app(cred, {"storageBucket": "dlfpdb.appspot.com"})
fn=set()
blobs = storage.bucket().list_blobs()
for blob in blobs:
  if blob.name.endswith("/"):
    fn.add(blob.name.rstrip("/"))
#thingspeak initialization
channel_id = '*******'//replace it with your channel id
write_api_key = '*********'//replace it with yor write_api_key
thingspeak_read_url = "https://api.thingspeak.com/channels/******/feeds.json?results=1"
thingspeak_write_url = "https://api.thingspeak.com/update?api_key=*********&field3="
def check_thingspeak(feedno):
    try:
        response = requests.get(thingspeak_read_url)
        data = response.json()
        field_value = data['feeds'][0]['field'+str(feedno)]
        return str(field_value)
    except Exception as e:
        print("Error:", e)
        return None
#@app.route("/verify/<idnum>")
#def isvalidfinger(idnum:str,fingerimage:np.ndarray)->bool:
def isvalidfinger():
  while(check_thingspeak(4)!="verified"):
    idnum=str(check_thingspeak(1))
    s=str(check_thingspeak(1))
    s=s.replace("\n","")
    idnum=s
    if idnum in fn:
      fpdata_directory = idnum+"/"
      blobs = storage.bucket().list_blobs(prefix=fpdata_directory)
      for blob in blobs:
        file_path = blob.name
        if not file_path.endswith("/"):
          print(f"current file: {file_path}")
          fingerprint_database_bytes = BytesIO()
          blob.download_to_file(fingerprint_database_bytes)
          fingerprint_database_bytes.seek(0)
          fingerprint_database_image = cv2.imdecode(np.frombuffer(fingerprint_database_bytes.read(), dtype=np.uint8), cv2.IMREAD_COLOR)
          #print(fingerprint_database_image.dtype,fingerprint_database_image.shape)
          sift = cv2.xfeatures2d.SIFT_create()
          #keypoints_1, descriptors_1 = sift.detectAndCompute(fingerimage, None)
          keypoints_1, descriptors_1 = sift.detectAndCompute(fingerprint_database_image, None)
          keypoints_2, descriptors_2 = sift.detectAndCompute(fingerprint_database_image, None)
          matches = cv2.FlannBasedMatcher(dict(algorithm=1,trees=10),dict()).knnMatch(descriptors_1, descriptors_2, k=2)
          #print(len(matches))
          match_points = []
          for p, q in matches:
              if p.distance < 0.7 *q.distance:
                  match_points.append(p)
          keypoints = 0
          #print(len(match_points))
          if len(keypoints_1) <= len(keypoints_2):
              keypoints = len(keypoints_1)
          else:
              keypoints = len(keypoints_2)
          #print(keypoints)
          if (len(match_points) / keypoints)>0.65:
              print("% match: ", len(match_points) / keypoints * 100)
              print("Figerprint ID: " + str(file_path))
              print((len(match_points) / keypoints))
              #result = cv2.drawMatches(test_original, keypoints_1, fingerprint_database_image, keypoints_2, match_points, None)
              #result = cv2.resize(result, None, fx=2.5, fy=2.5)
              return requests.get(thingspeak_write_url+str("valid&field4=verified"))
      else:
        return requests.get(thingspeak_write_url+str("invalid")+str("&field4=verified"))
    else:
      return requests.get(thingspeak_write_url+str("invalid")+str("&field4=verified")) 
while(True):
  isvalidfinger()
  time.sleep(5)
#if __name__=='__main__':
#   app.run(debug=True)
