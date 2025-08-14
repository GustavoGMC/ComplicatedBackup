FROM gcc
COPY . /usr/src/hardbkp
WORKDIR /usr/src/hardbkp
RUN gcc -o motor motor.c
CMD ["./motor"]