#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>

int main(int argc, char* argv[]) {

  const char* term;

  if (argc == 2) {
    term = argv[1];
  } else {
    term = "/tmp/ttyV0";
  }


  // opening the serial device for reading/writing in non-blocking mode
  int fd = open(term, O_RDWR | O_NONBLOCK);
  if (fd == -1) {
    switch (errno) {
      case ENOENT:
        fprintf(stderr, "Serial device %s not found\n", term);
        break;
      case EACCES:
        fprintf(stderr, "Permission denied\n");
        break;
      case EBUSY:
        fprintf(stderr, "Serial device %s is busy\n", term);
        break;
      default:
        perror("open");
    }

    return EXIT_FAILURE;
  }

  // check if the fd refers to a terminal
  if (!(isatty(fd))) {
    switch (errno) {
      case EBADF:
        fprintf(stderr, "fd returned by opening %s is not valid\n", term);
        break;
      case ENOTTY:
        fprintf(stderr, "fd returned by opening %s does not refer to a terminal\n", term);
        break;

      default:
        perror("isatty");
    }

    return EXIT_FAILURE;
  }

  // configure the communication
  struct termios conf;

  if (tcgetattr(fd, &conf) == -1) {
    perror("tcgetattr");
    return EXIT_FAILURE;
  }

  // set the input baud rate, B115200 for compatibility
  if (cfsetispeed(&conf, B115200) == -1) {
    perror("cfsetispeed");
    return EXIT_FAILURE;
  }

  // set the output baud rate, B115200 for compatibility
  if (cfsetospeed(&conf, B115200) == -1) {
    perror("cfsetospeed");
    return EXIT_FAILURE;
  }

  // configuring control modes

  // configure to 8N1 config

  // clear the configuration
  conf.c_cflag &= ~CSIZE;

  // set the data bits to 8
  conf.c_cflag |= CS8;

  // disable parity bit
  conf.c_cflag &= ~PARENB;

  // enable receiver
  conf.c_cflag |= CREAD;

  // use 1 stop bit
  conf.c_cflag &= ~CSTOPB;


  // configuring local modes


  // use noncanonical mode
  conf.c_lflag &= ~ICANON;

  // disable echo
  conf.c_lflag &= ~ECHO;

  // disable signals INTR, QUIT, [D]SUSP 
  conf.c_lflag &= ~ISIG;


  // configuring input modes


  // disable flow control flags
  conf.c_iflag &= ~(IXON | IXOFF | IXANY);


  // configuring output modes

  // disable implementation-defined output processing (sending raw data)
  conf.c_oflag &= ~OPOST;


  // set the configured attributes
  if (tcsetattr(fd, TCSANOW, &conf) == -1) {
    switch (errno) {
      case EBADF:
        fprintf(stderr, "fd returned by opening %s is not valid\n", term);
        break;
      case ENOTTY:
        fprintf(stderr, "fd returned by opening %s does not refer to a terminal\n", term);
        break;
      case EINVAL:
        perror("TCSANOW");
        break;
      case EINTR:
        perror("tcsetattr");
        break;
      case EIO:
        perror("tcsetattr");
        break;

      default:
        perror("tcsetattr");
    }

    return EXIT_FAILURE;
  }

  // writing data
  char* hello = "hello from the uart_conf\n";
  write(fd, hello, strlen(hello));

  // reading data
  while (1) {

    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;

    const int timeout = 5000;
    int pollret = poll(&pfd, 1, timeout);
    if (pollret < 0) {
      perror("poll");
      return EXIT_FAILURE;
    } else if (pollret == 0) {
      fprintf(stdout, "Timeout: No Data\n");
      continue;
    }

    if (pfd.revents & POLLIN) {
      char buf[256];
      memset(buf, 0, sizeof(buf));
      ssize_t read_bytes = read(fd, buf, sizeof(buf));
      if (read_bytes < 0) {
        perror("read");
        break;
      }
      fprintf(stdout, "Received %zd bytes: %s", read_bytes, buf);
    } 

  }


  close(fd);

  return EXIT_SUCCESS;
}
