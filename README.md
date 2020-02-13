## What is `neptune`?
Neptune is an experimental learning project.<br>
Using C++ language to realize related functions of distributed system.
The code structure is mainly composed of two parts, base and module.

## `src/base`
It is a basic library of neptune.

## `src/module`
Various service components are integrated in this section,such as 'module/dfs'.

## `module/dfs` 
#### Depend on gtest
git clone https://github.com/google/googletest<br>
cd googletest/googletest<br>
mkdir build<br>
cd build<br>
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local<br>
make<br>
sudo make install<br>

#### Build
./build.sh

#### Run
##### metaserver
* cd build/release/release/<br>
* ./bin/metaserver -f /opt/neptune/dfs/conf/metaserver.conf -d

##### dataserver
* mkfs.ext4 /dev/sda<br>  
* mount -t ext4 /dev/sda /data/disk1<br>
* bash ./script/disk.sh format 1<br>
* ./bin/dataserver -f /opt/neptune/dfs/conf/dataserver.conf -i 1
