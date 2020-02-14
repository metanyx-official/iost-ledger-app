FROM ubuntu:18.04
ENV ADPU_PORT 4444
ENV VNC_PORT 55555

RUN apt-get update
# common packages
RUN apt-get -y install \
    python3 wget
# arm cortex compiler
RUN apt-get -y install \
    llvm clang \
    gcc-arm-none-eabi libstdc++-arm-none-eabi-newlib
# blue-loader-python dependencies
RUN apt-get -y install \
    libudev-dev libusb-1.0-0-dev \
    python3-dev python3-coverage python3-cryptography python3-hidapi python3-requests
# speculos dpendencies
RUN apt-get -y install \
    python3-pil python3-pyelftools python3-mnemonic python3-setuptools python3-construct \
    cmake libvncserver-dev perl-modules qemu-user-static \
    gdb-multiarch libc6-dev-armhf-cross gcc-arm-linux-gnueabihf gcc-multilib-arm-linux-gnueabi
#g++-multilib-arm-linux-gnueabihf gcc-multilib-arm-linux-gnueabi g++-8-multilib-arm-linux-gnueabi
#gcc-multilib g++-multilib \
# create limited user
RUN adduser --quiet --disabled-password bob
USER bob
WORKDIR /home/bob


# add sources and build app and sdk
ADD ./ ./.app
RUN cp -r .app app
RUN sed -i 's/-I\/usr\/include/-I\/usr\/arm-linux-gnueabi\/include/' app/sdk/nanos-secure-sdk/Makefile.defines
RUN cd app/sdk/python-yubicommon && \
    python3 setup.py install --user && \
    ln -s $(../build-helpers/find-installed-package.sh yubicommon) ../python-u2flib-host/vendor/yubicommon && \
    cd ../python-u2flib-host && \
    python3 setup.py install --user && \
    cd ../blue-loader-python && \
    python3 setup.py install --user && \
    cd ../.. && \
    env BOLOS_SDK=sdk/nanos-secure-sdk make
RUN cd app/sdk/speculos && \
    cmake -Bbuild -H. -DCMAKE_BUILD_TYPE=Debug -DWITH_VNC=1 && \
    make -C build -j3 && \
    make -C build install

# run app builder and emulator
EXPOSE ${ADPU_PORT}
EXPOSE ${VNC_PORT}
ENTRYPOINT ["python3", "app/x.py", "app/sdk/speculos"]

#build-essential libc6-i386 libc6-dev-i386 python python-pip libudev-dev libusb-1.0-0-dev python3-dev git
#ENV BOLOS_ENV /work/bolos
#RUN mkdir -p ${BOLOS_ENV}
#WORKDIR ${BOLOS_ENV}
#ADD clang+llvm-4.0.0-x86_64-linux-gnu-ubuntu-16.04.tar.xz ${BOLOS_ENV}
#RUN ln -s "clang+llvm-4.0.0-x86_64-linux-gnu-ubuntu-16.04" "clang-arm-fropi"
#ADD gcc-arm-none-eabi-5_3-2016q1-20160330-linux.tar.bz2 ${BOLOS_ENV}
#ARG BOLOS_SDK_NAME=nanos-secure-sdk-nanos-1421
#ADD ${BOLOS_SDK_NAME}.* ${BOLOS_ENV}
#ENV BOLOS_SDK ${BOLOS_ENV}/${BOLOS_SDK_NAME}
#RUN git clone https://github.com/LedgerHQ/blue-loader-python.git && ( cd blue-loader-python ; pip install ledgerblue )
#RUN rm -rf blue-loader-python
#CMD /bin/bash
# https://launchpad.net/gcc-arm-embedded/5.0/5-2016-q1-update/+download/gcc-arm-none-eabi-5_3-2016q1-20160330-linux.tar.bz2
# http://releases.llvm.org/4.0.0/clang+llvm-4.0.0-x86_64-linux-gnu-ubuntu-16.04.tar.xz
