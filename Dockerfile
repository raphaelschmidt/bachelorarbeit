FROM tensorflow/tensorflow

RUN pip install flask flask_cors keras tensorflow bing-image-downloader tdqm numpy paho-mqtt pandas tensorflow Pillow

RUN apt-get update && apt-get -qq install xxd

RUN apt-get -y install git wget flex bison gperf python3 python3-pip python3-setuptools cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0

RUN git clone --recursive https://github.com/espressif/esp-idf.git

RUN /esp-idf/install.sh

ENV IDF_PATH "/esp-idf"

RUN echo "alias get_idf='. /esp-idf/export.sh'" >> /root/.bashrc

RUN chmod +x /esp-idf/*

RUN . /esp-idf/export.sh

RUN apt -y remove cmake

RUN wget https://github.com/Kitware/CMake/releases/download/v3.20.1/cmake-3.20.1.tar.gz

RUN mkdir downloads

RUN tar -zxvf cmake-3.20.1.tar.gz -C /downloads

RUN cd /downloads/cmake-3.20.1 && ./configure

RUN make -C /downloads/cmake-3.20.1

RUN make install -C /downloads/cmake-3.20.1

ADD . /app

RUN chmod +x /app/sh/*

WORKDIR /app

CMD ["/bin/bash", "-c", "python3 server.py" ]

