FROM ubuntu:16.04
RUN rm /bin/sh && ln -s /bin/bash /bin/sh
RUN apt-get update
RUN apt-get install -y cmake build-essential subversion doxygen texlive-latex-base libboost-all-dev git python-vtk6 python-dev x11vnc fluxbox wget wmctrl xvfb
RUN git clone --depth=50 --branch=master https://github.com/aewallin/opencamlib.git aewallin/opencamlib
ENV CXX g++
ENV CC gcc
ENV DISPLAY :20
WORKDIR ./aewallin/opencamlib
RUN mkdir build
RUN ls 
WORKDIR ./build
RUN cmake ../src
RUN make
RUN make install
WORKDIR /usr/lib/x86_64-linux-gnu 
EXPOSE 5900
RUN useradd -ms /bin/bash cam
USER cam
ENV USER cam
WORKDIR /aewallin/opencamlib/
ADD dockerinit.sh /
CMD '/dockerinit.sh'

