# Docker to build kstars Catalog 

## mod Base container and Add some python stuff 

    docker run -it --name tim_tmp hiro98/pykstars /bin/bash

in the container we want to make these changes

    cd stuff 
    pip3 install -r ./.pe/lib/python3.12/site-packages/KStars_Catalog_Tool-0.0.2-py3.12.egg-info/requires.txt  --break-system-packages

We then **exit** from the container 

And we now save the changes as a new image 

    docker commit tim_tmp tim_kstars_cat 

We should be able to see this new image with 

    docker images 


##To Run 

So we have our Image - which we will not make any changes to (-rm) 
The folder we are in will be mounted as **/stuff**, and changes will be reflected in the host file system. 

    docker run -v $(pwd):/stuff --rm -it tim_kstars  -c 'bash' 
    

##  Check all is ok 

    python3 main.py

## See which catalogs are defined 

     python3 main.py list 

And you should see something like 

```text
 id                             name  precedence
  1                          OpenNGC         1.0
  2               NGC IC (Steinicke)         0.1
  3          Abell Planetary Nebulae         0.3
  4     Sharpless HII region Catalog         0.5
  5           Hickson Compact Groups         0.2
  6 Lynds' Catalogue of Dark Nebulae         1.0
  7        Barnard Planetary Nebulae         0.3
  8                         Gary Imm         0.4

```

# Build all 

    python3 main.py  build 


## build just 1 catalog 

Using the output from **list** (above)  use the id number. 

    python3 main.py build -c 8

# Clean the cache 

    python3 main.py  clean --cache-only 

or bigger clean 


    python3 main.py  clean 


