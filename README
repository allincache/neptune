### What is `neptune`?
Neptune is an experimental learning project.
Using C++ coding to implement distributed functions.

## "src/base"
It is a basic library of neptune.

## "src/module/dfs" 
# Depend on gtest：
git clone https://github.com/google/googletest
cd googletest/googletest
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
make
sudo make install

# Build:
./build.sh

# Run
metaserver:
cd build/release/release/
./bin/metaserver -f /opt/neptune/dfs/conf/metaserver.conf -d

dataserver:
1)
mkfs.ext4 /dev/sda  
mount -t ext4 /dev/sda /data/disk1
2)
bash ./script/disk.sh format 1
3)
./bin/dataserver -f /opt/neptune/dfs/conf/dataserver.conf -i 1
