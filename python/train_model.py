import keras
from keras import backend as K
from keras.layers.core import Dense, Activation
from keras.optimizers import Adam
from keras.metrics import categorical_crossentropy
from keras.preprocessing.image import ImageDataGenerator
from keras.preprocessing import image
from keras.models import Model
from keras.applications import imagenet_utils
from keras.layers import Dense, GlobalAveragePooling2D
from keras.applications import MobileNet
from keras.applications import MobileNetV2
from keras.applications.mobilenet import preprocess_input
import numpy as np
import configparser
from keras.optimizers import Adam
import os

MODELS_DIR = './mobileNetV2_imagenet_96_96_3'
if not os.path.exists(MODELS_DIR):
    os.mkdir(MODELS_DIR)
MODEL_TF = MODELS_DIR
MODEL_NO_QUANT_TFLITE = MODELS_DIR + 'model_no_quant.tflite'
MODEL_TFLITE = MODELS_DIR + 'model.tflite'
MODEL_TFLITE_MICRO = MODELS_DIR + 'model.cc'

import configparser

import tensorflow as tf

### Load Config files
config = configparser.ConfigParser()
config.read('config.ini')
epochs = int(config["MODEL"]["model_epochs"])


mobile = keras.applications.mobilenet.MobileNet()

resolution = (96,96)

def prepare_image(file):
    img_path = ''
    img = image.load_img(img_path + file, target_size=resolution)
    img_array = image.img_to_array(img)
    img_array_expanded_dims = np.expand_dims(img_array, axis=0)
    return keras.applications.mobilenet.preprocess_input(img_array_expanded_dims)

# Modell definieren
base_model = MobileNetV2(weights='imagenet', include_top=False, input_shape=(resolution + (3,)),
                         alpha=0.5)
base_model.trainable = False
x = base_model.output
x = GlobalAveragePooling2D()(x)
preds = Dense(2, activation='softmax')(x)

model = Model(inputs=base_model.input, outputs=preds)

# Datensatz verfügbar machen
datasetPath = "./dataset"
train_datagen = ImageDataGenerator(preprocessing_function=preprocess_input,
                                   validation_split=0.2)

train_generator = train_datagen.flow_from_directory(datasetPath,
                                                    target_size=resolution,
                                                    color_mode='rgb',
                                                    batch_size=10,
                                                    class_mode='categorical',
                                                    subset="training",
                                                    shuffle=True)

validation_generator = train_datagen.flow_from_directory(
    datasetPath,
    target_size=resolution,
    batch_size=10,
    class_mode='categorical',
    subset='validation')

label_map = (train_generator.class_indices)

# Modell erstellen
model.compile(optimizer='Adam', loss='categorical_crossentropy', metrics=['accuracy'])

# Model Trainieren
step_size_train = train_generator.n // train_generator.batch_size
history = model.fit(x=train_generator,
                    validation_data=validation_generator,
                    validation_steps=validation_generator.samples // 10,
                    steps_per_epoch=step_size_train,
                    epochs=epochs)

# Modell evaluieren
results = model.evaluate(validation_generator)

# Evaluationsergebnis speichern
config = configparser.ConfigParser()
config.read('config.ini')
config["MODEL"]["model_accuracy"] = str(results[1])
config["MODEL"]["model_loss"] = str(results[0])
for label, label_num in label_map.items():
    config["MODEL"][f"LABELS_{label_num}"] = label
with open('config.ini', 'w') as configfile:
    config.write(configfile)

# TF Model speichern
model.save('mobileNetV2_imagenet_96_96_3')

# Model in TF-Lite-Format umwandeln
converter = tf.lite.TFLiteConverter.from_saved_model(MODEL_TF)
converter.target_spec.supported_ops = [
    tf.lite.OpsSet.TFLITE_BUILTINS,
    tf.lite.OpsSet.SELECT_TF_OPS
]
model_no_quant_tflite = converter.convert()

# TF-Lite Modell speichern
open(MODEL_NO_QUANT_TFLITE, "wb").write(model_no_quant_tflite)


# TF-Lite Modell quantisieren
def representative_dataset():
    for _ in range(10):
        img = validation_generator.next()
        yield [img[0]]

converter.optimizations = [tf.lite.Optimize.DEFAULT]
converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
converter.inference_input_type = tf.int8
converter.inference_output_type = tf.int8
converter.representative_dataset = representative_dataset
converter.target_spec.supported_ops = [
    tf.lite.OpsSet.TFLITE_BUILTINS,
    tf.lite.OpsSet.SELECT_TF_OPS
]
model_tflite = converter.convert()

# Quantisiertes TF-Lite-Modell speichern
open(MODEL_TFLITE, "wb").write(model_tflite)

# Quantisiertes TF-Lite-Modell in Hexdump umwandeln
os.system(f"xxd -i {MODEL_TFLITE} > {MODEL_TFLITE_MICRO}")

# Datei für den Einsatz in der Embedded Software vorbereiten
REPLACE_TEXT = MODEL_TFLITE.replace('/', '_').replace('.', '_')
os.system(f"sed -i 's/'{REPLACE_TEXT}'/g_model/g' {MODEL_TFLITE_MICRO}")
os.system(
    f"sed -i 's/unsigned char g_model/alignas(8) const unsigned char g_materials_model_data/g' {MODEL_TFLITE_MICRO}")
os.system(f"sed -i 's/unsigned int g_model_len/const int g_materials_model_data_len/g' {MODEL_TFLITE_MICRO}")
os.system(f"sed -i '1s/^/#include \\x22.\/materials_model_data.h\\x22\\n/' {MODEL_TFLITE_MICRO}")
