import cv2
import json
import base64
from flask import Flask, jsonify
import threading

app = Flask(__name__)

class FaceTracker:
    def __init__(self):
        # Carica il modello pre-addestrato per il riconoscimento facciale
        self.face_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + "haarcascade_frontalface_default.xml")

    def get_faces(self, frame):
        """Rileva i volti nel frame e restituisce le coordinate in formato JSON."""
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        faces = self.face_cascade.detectMultiScale(gray, scaleFactor=1.1, minNeighbors=5, minSize=(30, 30))
        face_data = [{"x": int(x), "y": int(y), "w": int(w), "h": int(h)} for (x, y, w, h) in faces]
        return json.dumps({"faces": face_data})

# Crea un'istanza del tracker di volti
tracker = FaceTracker()

# Variabile globale per memorizzare le coordinate delle facce rilevate e l'immagine corrente
current_faces = []
current_frame = None

def capture_frames():
    global current_faces, current_frame
    cap = cv2.VideoCapture(0)  # Apre la webcam

    while True:
        ret, frame = cap.read()
        if not ret:
            break

        # Rileva i volti nel frame corrente
        face_json = tracker.get_faces(frame)
        current_faces = json.loads(face_json)["faces"]

        # Disegna rettangoli attorno ai volti e aggiungi le coordinate dei vertici
        for face in current_faces:
            x, y, w, h = face["x"], face["y"], face["w"], face["h"]

            # Disegna il rettangolo attorno alla faccia
            cv2.rectangle(frame, (x, y), (x + w, y + h), (255, 0, 0), 2)

            # Aggiungi le coordinate dei vertici del riquadro
            cv2.putText(frame, f"TL: ({x}, {y})", (x, y - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 0, 0), 1)
            cv2.putText(frame, f"TR: ({x + w}, {y})", (x + w - 100, y - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 0, 0), 1)
            cv2.putText(frame, f"BL: ({x}, {y + h})", (x, y + h + 20), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 0, 0), 1)
            cv2.putText(frame, f"BR: ({x + w}, {y + h})", (x + w - 100, y + h + 20), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 0, 0), 1)

        # Memorizza il frame corrente
        current_frame = frame

        cv2.imshow("Face Tracking", frame)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()

@app.route('/faces', methods=['GET'])
def get_faces():
    """Endpoint per ottenere le coordinate delle facce rilevate."""
    return jsonify(current_faces)

@app.route('/stream', methods=['GET'])
def get_stream():
    """Endpoint per ottenere l'immagine con i riquadri delle facce in streaming, codificata in base64."""
    global current_frame

    if current_frame is None:
        return jsonify({"error": "Nessun frame disponibile"}), 404

    # Codifica l'immagine in formato JPEG
    _, buffer = cv2.imencode('.jpg', current_frame)
    # Converte l'immagine in una stringa base64
    jpg_as_text = base64.b64encode(buffer).decode('utf-8')

    return jsonify({"image": jpg_as_text})

if __name__ == "__main__":
    # Avvia la cattura dei frame in un thread separato
    threading.Thread(target=capture_frames, daemon=True).start()

    # Avvia il server Flask
    app.run(host='0.0.0.0', port=5000)