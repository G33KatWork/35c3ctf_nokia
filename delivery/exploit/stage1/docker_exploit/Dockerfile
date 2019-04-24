FROM phusion/baseimage:0.11

RUN DEBIAN_FRONTEND=noninteractive apt-get update -q -y && apt-get upgrade -q -y
RUN DEBIAN_FRONTEND=noninteractive apt-get -q -y install vim net-tools telnet
RUN DEBIAN_FRONTEND=noninteractive apt-get -q -y install openvpn iputils-ping
RUN DEBIAN_FRONTEND=noninteractive apt-get -q -y install curl build-essential git autoconf libtool pkg-config apt-utils
RUN DEBIAN_FRONTEND=noninteractive apt-get -q -y install libtalloc-dev libpcsclite-dev libgnutls28-dev

# libosmocore
RUN cd /root && git clone git://git.osmocom.org/libosmocore.git \
    && cd libosmocore \
    && autoreconf -i \
    && ./configure && make && make install \
    && ldconfig \
    && cd ..

# Copy configs
ADD configs/openvpn /etc/openvpn/

RUN mkdir -p /root/.osmocom/bb
ADD configs/osmocombb/mobile.cfg /root/.osmocom/bb/

CMD ["/bin/bash"]

#mkdir /dev/net
#mknod /dev/net/tun c 10 200
#/usr/sbin/openvpn --cd /etc/openvpn/ --config gsm.conf
#/pwn/src/host/virt_phy/src/virtphy -s /tmp/osmocom_l2.one
#/pwn/src/host/layer23/src/mobile/mobile
#telnet localhost 4247
