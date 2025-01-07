ARG VARIANT=bookworm
FROM mcr.microsoft.com/vscode/devcontainers/base:${VARIANT}

ENV DEBIAN_FRONTEND=noninteractive
RUN sudo apt-get update \
 && sudo apt-get -y install --no-install-recommends \
      clang \
      python3-venv \
      udev \
      usbutils

## Set up udev rules for PlatformIO
RUN curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core/develop/platformio/assets/system/99-platformio-udev.rules | sudo tee /etc/udev/rules.d/99-platformio-udev.rules
RUN service udev restart 
RUN usermod -a -G dialout vscode
RUN usermod -a -G plugdev vscode

ARG FEDORA_COMPAT=0
### Set up compatibility with Fedora host (if needed)
### Since Fedora uses `18` as the group for dialout, we need to add it to the container
RUN if [ "$FEDORA_COMPAT" = "1" ]; then \
  sudo groupadd -g 18 compat_dialout; \
  sudo usermod -a -G compat_dialout vscode; \
fi


# Install PlatformIO CLI
USER vscode
RUN curl -fsSL -o /tmp/get-platformio.py https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py
RUN python3 /tmp/get-platformio.py
RUN echo 'export PATH="$PATH:$HOME/.platformio/penv/bin"' | tee -a /home/vscode/.bashrc /home/vscode/.zshrc 
RUN echo 'export PATH="$PATH:$HOME/.platformio/penv/bin"' | sudo tee -a /root/.bashrc /root/.zshrc

WORKDIR /home/vscode/project

RUN $HOME/.platformio/penv/bin/pio project init -e seeed_xiao_esp32s3 --project-option="platform=espressif32" --project-option="framework=espidf"
RUN $HOME/.platformio/penv/bin/pio pkg install -p "platformio/espressif32 @ ^6.9.0" \
                    -t "platformio/tool-cmake @ ~3.16.0" \
                    -t "platformio/tool-ninja @ ^1.7.0" \
                    -t "espressif/tool-xtensa-esp-elf-gdb @ ~12.1.0" \
                    -t "platformio/toolchain-riscv32-esp @ 13.2.0" \
                    -t "platformio/toolchain-xtensa-esp-elf @ 13.2.0" \
                    -t "platformio/toolchain-esp32ulp @ ~1.23800.0" \
                    -t "espressif/tool-riscv32-esp-elf-gdb @ ~12.1.0"                     
                  