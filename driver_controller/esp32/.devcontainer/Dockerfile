FROM espressif/idf:release-v5.4

RUN apt-get update && apt-get upgrade && apt-get install -y usbutils udev

RUN echo "source /opt/esp/idf/export.sh" >> /root/.bashrc