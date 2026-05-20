# Docker Stuff 

We need to use the hiro98 image 

## DockerFile 

This is the Dockerfile 

```
FROM hiro98/pykstars:latest

WORKDIR /stuff

COPY . /stuff

RUN pip3 install -r ./.pe/lib/python3.12/site-packages/KStars_Catalog_Tool-0.0.2-py3.12.egg-info/requires.txt \
    --break-system-packages
```

That should be it 

### Build the new Image 

We build using

    docker build -t pykstars .

At the end I see 

```text
 ---> 5e2478ec19f4
Successfully built 5e2478ec19f4
Successfully tagged pykstars:latest 
```


# Run the Container 
To Use this container we do a 

```bash
docker run -it --rm \
  -v /home/tim/Dev/Python/Astro/kstars-catalogs-master:/stuff \
  -w /stuff \
  pykstars \
  bash
```


