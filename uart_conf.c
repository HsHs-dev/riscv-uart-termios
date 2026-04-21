/*
 * A program that initializes and configure a UART (Universal asynchronous receiver-transmitter)
 * interface on Linux using the termios library, which describe a general terminal interface that
 * is provided to control asynchronous communications ports.
 *
 * The program flow is:
 * - Open a serial device (specified by the user by giving a command-line argument,
 *   or a virtual terminal for the auto-testing script)
 * - Configures the UART to 8N1 data format, 115200 baud rate, and raw transfer mode
 * - Transmits a test message over the configured serial interface
 * - Receive and print any incoming data within a 5-seconds timeout window
 * - Gracefully handle any errors that occur in any of the previous steps
 * - Close the serial device and exit
 *
 * Author: Hassan Siddig
 * Date: April 2026
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>

/* if something went wrong, close the file descriptor of the port
 * and exit with failure code */
int failure(int fd) {
  close(fd);
  return EXIT_FAILURE;
}


int main(int argc, char* argv[]) {


  /* open the serial device for reading/writing in non-blocking mode */

  const char *term = (argc > 1) ? argv[1] : "/dev/ttyS0";

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

    failure(fd);
  }

  /* verify that the file descriptor refers to a terminal device */
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

    failure(fd);
  }


  /***************************************
   * configure the UART serial interface *
   ***************************************/

  struct termios conf;

  if (tcgetattr(fd, &conf) == -1) {
    perror("tcgetattr");
    failure(fd);
  }

  /* set the input/output baud rates to B115200 */

  if (cfsetispeed(&conf, B115200) == -1) {
    perror("cfsetispeed");
    failure(fd);
  }

  if (cfsetospeed(&conf, B115200) == -1) {
    perror("cfsetospeed");
    failure(fd);
  }

  /* 
   * configuring control modes:
   * - 8 data bits
   * - no parity bit
   * - 1 stop bit
   */


  // clear the character size bits
  conf.c_cflag &= ~CSIZE;

  // set the data bits to 8
  conf.c_cflag |= CS8;

  // disable parity bit
  conf.c_cflag &= ~PARENB;

  // enable receiver
  conf.c_cflag |= CREAD;

  // use 1 stop bit
  conf.c_cflag &= ~CSTOPB;


  /* 
   * configuring local modes:
   * - use noncanonical mode for direct sending and no-editing,
   *   allowing for raw transmitting
   * - disable echoing back data
   * - disable signals to avoid misinterpretation
   */

  // use noncanonical mode
  conf.c_lflag &= ~ICANON;

  // disable echo
  conf.c_lflag &= ~ECHO;

  // disable signals INTR, QUIT, [D]SUSP 
  conf.c_lflag &= ~ISIG;

  /* 
   * configuring input modes:
   *
   * - disable software flow control to avoid misinterpretation
   *
   */

  // disable flow control flags
  conf.c_iflag &= ~(IXON | IXOFF | IXANY);


  /* 
   * configuring output modes:
   *
   * - disable output post-processing to avoid misinterpretation
   *
   */

  // disable implementation-defined output processing (sending raw data)
  conf.c_oflag &= ~OPOST;


  /* set the configured terminal attributes immediately */
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

      default:
        perror("tcsetattr");
    }

    failure(fd);
  }

  /* transmit a test message over the serial port */
  const char* hello = "hello from the uart_conf program\n";
  ssize_t bytes_written = write(fd, hello, strlen(hello));
  if (bytes_written == -1) {
    perror("write");
    failure(fd);
  }
  fprintf(stdout, "wrote %zd bytes to serial port %s\n", bytes_written, term);


  /* read a message from the serial port within a 5-seconds timeout window */
  struct pollfd pfd;
  pfd.fd = fd;
  pfd.events = POLLIN;

  const int timeout = 5000;
  int pollret = poll(&pfd, 1, timeout);
  fprintf(stdout, "%d seconds to read data\n", (timeout/1000));
  if (pollret < 0) {
    perror("poll");
    failure(fd);
  } else if (pollret == 0) {
    fprintf(stderr, "Timeout: No Data\n");
    failure(fd);
  }

  if (pfd.revents & POLLIN) {
    char buf[256];
    memset(buf, 0, sizeof(buf));
    ssize_t read_bytes = read(fd, buf, sizeof(buf));
    if (read_bytes < 0) {
      perror("read");
      failure(fd);
    }
    fprintf(stdout, "Received %zd bytes: %s", read_bytes, buf);
  } 


  /* close the serial port and exit the program */

  close(fd);

  return EXIT_SUCCESS;
}
