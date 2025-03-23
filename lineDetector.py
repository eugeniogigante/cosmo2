import cv2
import json
import base64
from flask import Flask, jsonify
import threading
import numpy as np

app = Flask(__name__)

class LineDetector:
    def __init__(self):
        pass

    def get_lines(self, frame):
        """Rileva le linee nel frame e restituisce le coordinate in formato JSON."""
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        edges = cv2.Canny(gray, 50, 150, apertureSize=3)
        lines = cv2.HoughLinesP(edges, 1, np.pi / 180, threshold=100, minLineLength=50, maxLineGap=10)
        
        line_data = []
        if lines is not None:
            for line in lines:
                x1, y1, x2, y2 = line[0]
                line_data.append({"x1": int(x1), "y1": int(y1), "x2": int(x2), "y2": int(y2)})
        
        return json.dumps({"lines": line_data})

# Crea un'istanza del rilevatore di linee
detector = LineDetector()

# Variabile globale per memorizzare le coordinate delle linee rilevate e l'immagine corrente
current_lines = []
current_frame = None

def capture_frames():
    global current_lines, current_frame
    cap = cv2.VideoCapture(0)  # Apre la webcam

    while True:
        ret, frame = cap.read()
        if not ret:
            break

        # Rileva le linee nel frame corrente
        line_json = detector.get_lines(frame)
        current_lines = json.loads(line_json)["lines"]

        # Disegna le linee rilevate e aggiungi le informazioni
        for i, line in enumerate(current_lines):
            x1, y1, x2, y2 = line["x1"], line["y1"], line["x2"], line["y2"]
            
            # Disegna la linea
            cv2.line(frame, (x1, y1), (x2, y2), (0, 255, 0), 2)
            
            # Aggiungi le coordinate dei punti di inizio e fine
            cv2.putText(frame, f"Line {i+1}: ({x1}, {y1}) -> ({x2}, {y2})", (x1, y1 - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 1)

        # Memorizza il frame corrente
        current_frame = frame

        cv2.imshow("Line Detection", frame)

        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()

@app.route('/lines', methods=['GET'])
def get_lines():
    """Endpoint per ottenere le coordinate delle linee rilevate."""
    return jsonify(current_lines)

@app.route('/stream', methods=['GET'])
def get_stream():
    """Endpoint per ottenere l'immagine con le linee rilevate in streaming, codificata in base64."""
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