# Cloud Native NDI Video

## Dependencies

```bash
sudo apt-get install libavahi-client3
sudo apt install libncurses5-dev
```

## NDI Config file

On Linux, place the `/NDIConfig/ndi-config.v1.json` into `~/.ndi/ndi-config.v1.json`.
Update the `ndi.network.ips` list to include the host names.

## Build Video Engine

```bash
(cd video-engine && make)
```

## Run Video Engine

```bash
LD_LIBRARY_PATH=`pwd`/NDI_SDK/lib/x86_64-linux-gnu ./video-engine/ve 
```

## Running in WSL 2

When running in WSL 2, you can configure the system to view streams in Studio Monitor running on the host and you can also generate streams on the host.

On the WSL machine, replace `~/.ndi/ndi-config.v1.json` with the file in `NDIConfig`.

Under network, add the IP address of the host seen from the VM. You can find it by doing `cat /etc/resolv.conf`

```json
...
"networks": {
      "discovery": "",
      "ips": "172.28.144.1"
    }
...
```

On the host, launch Access Manager. Under `External Sources` add the IP address of the VM seen from the host. You can find it with `wsl hostname -I` in Powershell.

Make sure that you remove the applications (i.e. VLC and WSL) from being monitored by the Windows firewall.

## Running in Docker

### Creating a network

USE THIS ONE

```bash
docker network create ndi
```

```bash
docker network create --driver bridge --opt multicast.enable=true my-multicast-network
```

### Building all containers

```bash
docker build -t ndi-directory-service -f Dockerfile.Discovery.Server .
```

```bash
docker build -t send-video -f Dockerfile.Send.Video .
```

```bash
docker build -t obs-ndi -f Dockerfile.OBS.NDI .
```

### Run all containers

```bash
docker run --rm --network=ndi --name directory-service ndi-directory-service
```

```bash
docker run --rm --network=ndi --name send-video 
```

If you have media files (.avi, .mp4, etc) in $HOME/media,

```bash
docker run --shm-size=256m --rm -it -p 5900:5900 -p 5901:5901 --network=ndi -v `echo $HOME`/media:/media -e VNC_PASSWD=123456 obs-ndi:latest
```
