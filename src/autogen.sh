echo launching autoreconf
autoreconf -i
echo launching configure
./configure
echo launching make
make
