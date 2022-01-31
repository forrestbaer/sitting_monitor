#include <wiringPi.h>
#include <sys/time.h>
#include <time.h>
#include <sys/stat.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>

#define LOGF "/home/monk/log/sitmon.log"

static int ECHO = 6;
static int TRIG = 5;
static int LED = 23;

static int INRANGE = 90;
static int HYSTERESIS = 30;

int sitcounter = 0;

enum status{Away,Here,Unknown};
enum status sitting;

void logMessage(filename,message)
char *filename;
char *message;
{
  char ctn[1000];
  time_t t = time(NULL);
  struct tm *p = localtime(&t);
  strftime(ctn, 1000, "%-k:%M,%F", p);

  FILE *logfile;
  logfile=fopen(filename,"a+");
  if(!logfile) return;
  fprintf(logfile,"%s,%s\n",ctn, message);
  fclose(logfile);
}

void sigHandler(sig)
  int sig;
{
  switch(sig) {
    case SIGTERM:
      logMessage(LOGF, "stopped");
      digitalWrite(LED, LOW);
      exit(0);
      break;
    case SIGINT:
      logMessage(LOGF, "stopped");
      digitalWrite(LED, LOW);
      exit(0);
      break;
  }
}

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
  signal(SIGINT, &sigHandler);
  signal(SIGTERM, &sigHandler);

  pid = fork();

  if (pid < 0) exit(EXIT_FAILURE);
  if (pid > 0) exit(EXIT_SUCCESS);

  umask(0);
  chdir("/");

  int x;
  for (x = sysconf(_SC_OPEN_MAX); x >= 0; x--) {
    close(x);
  }

  logMessage(LOGF, "started");
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
// and confirming a sit by ten
// seconds of it being within range
//
int checkSitting(double d) {
  if (d <= INRANGE && sitcounter < HYSTERESIS) {
    sitcounter++;
  }
  if (d > INRANGE && sitcounter > 0) {
    sitcounter--;
  }

  if (d <= INRANGE && sitcounter == HYSTERESIS && sitting != Here) {
    digitalWrite(LED, HIGH); 
    sitting = Here;
    logMessage(LOGF, "sat");
  }
  if (d > INRANGE && sitcounter == 0 && sitting != Away) {
    digitalWrite(LED, LOW);
    sitting = Away;
    logMessage(LOGF, "stood");
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

  while (1) {
    dist = getDistance();
    sitting = checkSitting(dist);
    // set up watchdog here to 
    delay(1000);
  }

  logMessage(LOGF, "stopped");

  return EXIT_SUCCESS;
}
