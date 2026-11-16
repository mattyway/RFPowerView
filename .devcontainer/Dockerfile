FROM mcr.microsoft.com/devcontainers/python:3.12-bookworm

# Install build tools required for native unit testing (GCC, G++, Make)
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
    && apt-get -y install --no-install-recommends build-essential git \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Add vscode user to dialout/plugdev for serial device access if flashing hardware
RUN usermod -aG dialout,plugdev vscode || true

# Install PlatformIO CLI for vscode user
USER vscode
ENV PATH="/home/vscode/.local/bin:${PATH}"
RUN pip install --no-cache-dir --upgrade platformio
