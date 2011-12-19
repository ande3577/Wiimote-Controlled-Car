echo configuring for release version - OpenWRT
echo cleaning previous configuration
make distclean
echo launching autoreconf
autoreconf -i
echo Note: OpenWRT will call ./Configure
