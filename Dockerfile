FROM ubuntu:22.04

WORKDIR /app

RUN apt update && apt install -y libavahi-client3 libncurses5-dev

COPY NDI_SDK/ /app/NDI_SDK/
COPY video-engine/ve /app/video-engine/ve 

