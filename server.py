from flask import Flask
from flask_cors import CORS
from flask import request
from flask import jsonify
import requests
from bing_image_downloader import downloader
from tqdm import tqdm
import os
import configparser
import paho.mqtt.client as paho
import os.path

app = Flask(__name__,
            static_url_path='',
            static_folder='./',
            )

UPLOAD_FOLDER = '/dataset'
ALLOWED_EXTENSIONS = {'png', 'jpg', 'jpeg'}
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER

CORS(app)


@app.route('/')
def main():
    return app.send_static_file('index.html')


@app.route('/config', methods=['POST'])
def config():
    received_config = request.get_json()
    config = configparser.ConfigParser()
    config['WLAN'] = {k: received_config[k] for k in ('wlan_ssid', 'wlan_psk')}
    config['MQTT'] = {k: received_config[k] for k in
                      ('mqtt_server', 'mqtt_port', 'mqtt_user', 'mqtt_pass', 'mqtt_topic',)}
    config['MODEL'] = {k: received_config[k] for k in ('model_epochs',)}
    config['MODEL']["model_classes"] = str(len(classes))
    config['DEVICE'] = {k: received_config[k] for k in ('device_port',)}
    with open('config.ini', 'w') as configfile:
        config.write(configfile)
    return "OK"


@app.route('/mqtt', methods=['GET'])
def mqtt():
    key = request.args.get("key")
    if key == "start":
        def on_subscribe(client, userdata, mid, granted_qos):
            print("Subscribed: " + str(mid) + " " + str(granted_qos))

        def on_message(client, userdata, msg):
            print(msg.topic + " " + str(msg.qos) + " " + str(msg.payload))
            global predicted_class
            predicted_class = msg.payload
            print(predicted_class)

        def on_connect(client, userdata, flags, rc):
            print("CONNACK received with code %d." % (rc))

        config = configparser.ConfigParser()
        config.read('config.ini')
        mqtt_user = config["MQTT"]["mqtt_user"]
        mqtt_pass = config["MQTT"]["mqtt_pass"]
        mqtt_server = config["MQTT"]["mqtt_server"]
        mqtt_port = int(config["MQTT"]["mqtt_port"])
        mqtt_topic = config["MQTT"]["mqtt_topic"] + "/class_string"

        client.on_subscribe = on_subscribe
        client.on_message = on_message
        client.on_connect = on_connect
        client.username_pw_set(mqtt_user, mqtt_pass)
        client.connect(mqtt_server, mqtt_port)
        client.subscribe(mqtt_topic, qos=1)
        print(f"mqtt topic {mqtt_topic}")
        client.loop_start()

        return "true"

    elif key == "stop":
        client.loop_stop()
        return "false"

@app.route('/check', methods=['GET'])
def check():
    key = request.args.get("key")
    check = 0
    if key == "image_preparation":
        check = str(os.path.isfile("./state/image_preparation_done"))
        print("Image preparation checked")
    elif key == "training":
        check = str(os.path.isfile("./state/training_done"))
        print("Training checked")
    elif key == "build":
        check = str(os.path.isfile("./state/build_done"))
        print("build checked")
    elif key == "flash":
        check = str(os.path.isfile("./state/flash_done"))
        print("Flash checked")
    elif key == "classes":
        check = jsonify(classes)
        print("Images checked")
    elif key == "eval":
        config = configparser.ConfigParser()
        config.read('config.ini')
        model_accuracy = config["MODEL"]["model_accuracy"]
        model_loss = config["MODEL"]["model_loss"]
        check = jsonify((model_accuracy, model_loss))
        print("Images checked")
    elif key == "images":
        rclass = request.args.get("rclass")
        check = jsonify(getListOfFiles(f"./dataset/{rclass}"))
        print("Images checked")
    elif key == "prediction":
        check = predicted_class
        print("Prediction checked")
    return check


@app.route('/do', methods=['GET', 'POST'])
def do():
    key = request.args.get("key")
    if key == "image_preparation":
        prep_type = request.args.get("type")
        if request.args.get("num_images"):
            num_images = int(request.args.get("num_images"))
        else:
            num_images = 20
        klassen = request.get_json()
        image_preparation(prep_type, klassen, num_images)
    elif key == "training":
        print("Training started")
        os.system("./sh/start_train.sh")

    elif key == "build":
        prepare_cpp_files()
        os.system("./sh/start_build.sh")
        print("build started")
    elif key == "flash":
        config = configparser.ConfigParser()
        config.read('config.ini')
        device_port = config["DEVICE"]["device_port"]
        os.system(f"./sh/erase_flash.sh '{device_port}'")
        os.system(f"./sh/start_flash.sh '{device_port}'")
        print("Flash started")
    elif key == "stop":
        device_ip = request.args.get("ip")
        r = requests.get(f'http://{device_ip}/stop')
        print("Camera stopped")
    return "OK"


@app.route('/undo', methods=['GET'])
def undo():
    key = request.args.get("key")
    if key == "image_preparation":
        os.system("rm ./state/image_preparation_done")
        print("Image preparation undone")
    elif key == "training":
        os.system("rm ./state/training_done")
        print("Training undone")
    elif key == "build":
        os.system("rm ./state/build_done")
        print("build undone")
    elif key == "image":
        img_path = request.args.get("img_path")
        os.system(f"rm ./{img_path}")
        print("Image Deleted")
    return "OK"

@app.route("/upload", methods=["POST"])
def upload():
    uploaded_files = request.files.getlist("files")
    rclass = request.args.get("rclass")
    for file in uploaded_files:
        print("save")
        print(f"./dataset/{rclass}")
        file.save(os.path.join(f"./dataset/{rclass}", file.filename))
    return "OK"

def image_preparation(type, klassen, num_images=100):
    global classes
    classes = []
    if type == "auto":
        for klasse in tqdm(eval(klassen["klassen"])):
            classes.append(klasse)
            downloader.download(klasse, limit=num_images, output_dir='dataset', adult_filter_off=True, force_replace=False,
                                timeout=10)

            lowercase_exts(f"dataset/{klasse}")

    elif type == "manual":
        for klasse in tqdm(eval(klassen["klassen"])):
            classes.append(klasse)
            os.system(f"mkdir 'dataset/{klasse}'")

    os.system("touch ./state/image_preparation_done")
    return 'Image preparation started'


def getListOfFiles(dirName):
    # create a list of file and sub directories
    # names in the given directory
    listOfFile = os.listdir(dirName)
    allFiles = list()
    # Iterate over all the entries
    for entry in listOfFile:
        # Create full path
        fullPath = os.path.join(dirName, entry)
        # If entry is a directory then get the list of files in this directory
        if os.path.isdir(fullPath):
            allFiles = allFiles + getListOfFiles(fullPath)
        else:
            allFiles.append(fullPath[2:])
    return allFiles

def lowercase_exts(folder):
    for fname in os.listdir(folder):
        name, ext = os.path.splitext(fname)
        os.rename(os.path.join(folder, fname), os.path.join(folder, name + ext.lower()))

def prepare_cpp_files():
    config = configparser.ConfigParser()
    config.read('config.ini')
    # Make model_settings.cc
    filename = "./cpp/main/model_settings.h"
    kCategoryCount = int(config["MODEL"]["model_classes"])

    with open(filename, "w+") as f:
        data = f.read()
        f.seek(0)
        f.write('#ifndef MATERIALS_MODEL_SETTINGS_H_\n')
        f.write('#define MATERIALS_MODEL_SETTINGS_H_\n')
        f.write('constexpr int kNumCols = 224;\n')
        f.write('constexpr int kNumRows = 224;\n')
        f.write('constexpr int kNumChannels = 3;\n')
        f.write('constexpr int kMaxImageSize = kNumCols * kNumRows * kNumChannels;\n')
        f.write(f'constexpr int kCategoryCount = {kCategoryCount};\n')
        f.write('extern const char* kCategoryLabels[kCategoryCount];\n')
        f.write('#endif')
        f.truncate()

    # Make model_settings.cc
    filename = "./cpp/main/model_settings.cc"

    temp_string = "{"
    for entry in config["MODEL"]:
        if entry.startswith("labels"):
            temp_string += '"' + config["MODEL"][entry] + '",'
    temp_string += "}"

    with open(filename, "w+") as f:
        data = f.read()
        f.seek(0)
        f.write('#include "model_settings.h"\n')
        f.write(f'const char* kCategoryLabels[kCategoryCount] = {temp_string};')
        f.truncate()

# Make config.h
    filename = "./cpp/main/config.h"

    WIFI_SSID = config["WLAN"]["wlan_ssid"]
    WIFI_PASSWD = config["WLAN"]["wlan_psk"]
    MQTT_SERVER = config["MQTT"]["mqtt_server"]
    MQTT_PORT = config["MQTT"]["mqtt_port"]
    MQTT_TOPIC = config["MQTT"]["mqtt_topic"]
    MQTT_USER = config["MQTT"]["mqtt_user"]
    MQTT_PASS = config["MQTT"]["mqtt_pass"]

    with open(filename, "w+") as f:
        data = f.read()
        f.seek(0)
        f.write(f'#define WIFI_SSID "{WIFI_SSID}"\n')
        f.write(f'#define WIFI_PASSWD "{WIFI_PASSWD}"\n')
        f.write(f'#define MQTT_URI "mqtt://{MQTT_SERVER}:{MQTT_PORT}"\n')
        f.write(f'#define MQTT_TOPIC "{MQTT_TOPIC}"\n')
        f.write(f'#define MQTT_USER "{MQTT_USER}"\n')
        f.write(f'#define MQTT_PASS "{MQTT_PASS}"')
        f.truncate()



if __name__ == '__main__':
    classes = []
    client = paho.Client()
    predicted_class = "none"
    app.run(host="0.0.0.0", port=5000)
