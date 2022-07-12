./download_testfiles.sh
. /opt/intel/openvino_2022/setupvars.sh
mkdir b
cd b
cmake ..
make
./test | sort -k2nr | head -5
