FROM ubuntu:24.04
# podman build -t zephyr --build-arg USERNAME=vscode --progress plain --format=docker -f Dockerfile ./
# Use bash as the default shell
SHELL ["/bin/bash", "-c"]

USER root

ARG USERNAME
ARG UID=1000
ARG GID=$UID

RUN apt-get update && apt-get upgrade -y
RUN apt-get install -y --no-install-recommends git cmake ninja-build  \
    ccache dfu-util device-tree-compiler wget pipx usbutils minicom locales patch vim\
    python3-dev python3-pip python3-setuptools python3-tk python3-wheel xz-utils file \
    make gcc gcc-multilib g++-multilib libsdl2-dev libmagic1 python3-venv libncurses-dev

#gperf gdebi-core    

RUN userdel -rf ubuntu 
RUN if getent group ubuntu ; then groupdel ubuntu; fi \
    && groupadd -g ${GID} ${USERNAME} \
    && useradd --uid $UID --gid $GID -m $USERNAME \
    && usermod -aG dialout,plugdev $USERNAME\
    && apt-get install -y sudo \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME

RUN locale-gen en_US.UTF-8
ENV LANG=en_US.UTF-8
ENV LANGUAGE=en_US:en
ENV LC_ALL=en_US.UTF-8

USER $USERNAME

#build zephyr west enviroment
WORKDIR /home/$USERNAME 
# Init zephyr Project
ENV PATH=/home/$USERNAME/.local/bin:$PATH

RUN pipx install west && pipx ensurepath

#RUN west init -m https://github.com/zephyrproject-rtos/zephyr  --mr
RUN west init
#changed manifest file to download only stm32 build enviroment
COPY --chown=$USERNAME:$USERNAME west_stm32.yml /home/$USERNAME/zephyr

#upload project submodules
RUN west config manifest.file west_stm32.yml \
    && west update\
    && west zephyr-export \
    && west packages pip --install \
    && west sdk install -b /home/$USERNAME -t arm-zephyr-eabi

USER root
#WORKDIR /home/${USERNAME}/temp
#COPY JLink_Linux_V812_x86_64.deb JLink_Linux_V812_x86_64.deb
#RUN gdebi --n JLink_Linux_V812_x86_64.deb; exit 0
COPY /udev/60-openocd.rules /etc/udev/rules.d/60-openocd.rules
WORKDIR /home/${USERNAME}
USER $USERNAME
