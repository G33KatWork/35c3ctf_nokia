FROM ubuntu:18.04
MAINTAINER Andreas Galauner <andreas@galauner.de>

#Set debian frontend to noninteractive
ENV DEBIAN_FRONTEND noninteractive

#Update stuff
RUN apt-get update
RUN apt-get -y upgrade

#Use bash - not dash
RUN rm /bin/sh && ln -s /bin/bash /bin/sh

#Install stuff
RUN apt-get install -y locales git vim curl wget diffstat unzip gawk texinfo gcc-multilib \
    build-essential chrpath libsdl1.2-dev xterm parted dosfstools mtools syslinux cpio \
    python python2.7

#Set the locale to en_US.UTF-8
ADD files/locale /etc/default/locale
RUN locale-gen en_US.UTF-8 &&\
  DEBIAN_FRONTEND=noninteractive dpkg-reconfigure locales

#Create an unprivileged user
RUN useradd -m -G users --shell /bin/bash build
RUN mkdir -p /home/build
RUN chown -R build /home/build

#Setup a project environment
RUN mkdir -p /project && \
    chown build:users /project
VOLUME /project
WORKDIR /project

#entrypoint script
ADD files/entrypoint.sh /bin/entrypoint.sh
RUN chmod +x /bin/entrypoint.sh

#Start init system on entry
ENTRYPOINT ["/bin/entrypoint.sh"]
