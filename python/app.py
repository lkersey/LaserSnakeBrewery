from flask import Flask, jsonify
from flask_cors import CORS
import sql

app = Flask(__name__)
cors = CORS(app, resources={r"/*": {"origins": "*"}})

@app.route("/status", methods=['GET',])
def status():
    return jsonify(sql.get_status()), 200

if __name__ == '__main__':
    app.run()
