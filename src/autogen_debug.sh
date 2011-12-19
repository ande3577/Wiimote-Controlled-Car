echo configuring for pc debug version
echo cleaning previous configuration
make distclean
echo launching autoreconf
autoreconf -i
echo launching configure
./configure CFLAGS='-D_DEBUG=1 -D_PC_SIM=1 -g -O0 -Wall' LDFLAGS='-static'
