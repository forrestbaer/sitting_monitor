#include <wiringPi.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>

const int ECHO = 6;
const int TRIG = 5;
const int LED = 23;

int sitcounter = 0;

enum status{NotAtDesk,Sitting,Unknown};
enum status sitting;

// create daemon process so it runs in the background
// and reports sit/stands via syslog another tool can
// utilize syslog for reporting
//
static void daemonize() {
  pid_t pid;

  pid = fork();

  if (pid < 0) exit(EXIT_FAILURE);
  if (pid > 0) exit(EXIT_SUCCESS);
  if (setsid() < 0) exit(EXIT_FAILURE);

  signal(SIGCHLD, SIG_IGN);
  signal(SIGHUP, SIG_IGN);

  pid = fork();

  if (pid < 0) exit(EXIT_FAILURE);
  if (pid > 0) exit(EXIT_SUCCESS);

  umask(0);
  chdir("/");

  int x;
  for (x = sysconf(_SC_OPEN_MAX); x >= 0; x--) {
    close(x);
  }

  openlog("sitmon", LOG_PID, LOG_DAEMON);
}

// calculate distance in cm from the
// sensor. a good range is up to 133cm
// outside of that it's not very exact
// less than 110cm is considered sitting
// within range
//
double getDistance() {
  struct timeval t1, t2;
  double elapsedTime, distance;

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  while(digitalRead(ECHO) == 0) {
    gettimeofday(&t1, NULL);
  }

  while(digitalRead(ECHO) == 1) {
    gettimeofday(&t2, NULL);
  }

  elapsedTime = t2.tv_usec - t1.tv_usec;
  distance = elapsedTime * 0.017;

  return distance;
}

// check if we're sitting down
// by referencing the distance
// and confirming a sit by five
// seconds of it being within range
//
int checkSitting(double d) {
  if (d <= 110 && sitcounter < 5) {
    sitcounter++;
  }
  if (d > 110 && sitcounter > 0) {
    sitcounter--;
  }

  printf("%d : %d\n", sitcounter, sitting);

  if (d <= 110 && sitcounter == 5 && sitting != Sitting) {
    digitalWrite(LED, HIGH); 
    sitting = Sitting;
    syslog(LOG_INFO, "Sat down.");
  }
  if (d > 110 && sitcounter == 0 && sitting != NotAtDesk) {
    digitalWrite(LED, LOW);
    sitting = NotAtDesk;
    syslog(LOG_INFO, "Got up.");
  }

  return sitting;
}


int main() {
  daemonize();

  double dist;
  int sitting;
  struct timeval lt, ct;

  wiringPiSetup();
  wiringPiSetupGpio();
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  syslog(LOG_NOTICE, "Sitlog started.");

  while (1) {
    dist = getDistance();
    sitting = checkSitting(dist);
    // set up watchdog here to 
    delay(1000);
  }

  syslog(LOG_NOTICE, "Sitlog terminated.");
  closelog();

  return EXIT_SUCCESS;
}
