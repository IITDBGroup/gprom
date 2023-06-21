./autogen.sh
./configure --with-oci-lib=/home/felicien/Documents/iit/installation/instantclient_21_9 --with-oci-headers=/home/felicien/Documents/iit/installation/instantclient_21_9/sdk/include --with-ocilib=/home/felicien/Documents/iit/installation/ocilib --with-libpq-headers=/usr/include/postgresql/ --disable-monetdb
make
sudo make install