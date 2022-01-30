#include <wiringPi.h>
#include <sys/time.h>
#include <stdio.h>

const int ECHO = 6;
const int TRIG = 5;
const int LED = 23;

int sitting = 0;
int sitcounter = 0;

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
  if (d <= 90 && sitcounter < 5) {
    sitcounter++;
  }
  if (d > 90 && sitcounter > 0) {
    sitcounter--;
  }

  printf("%d : %d\n", sitcounter, sitting);

  if (d <= 90 && sitcounter == 5 && sitting == 0) {
    digitalWrite(LED, 1); 
    sitting = 1;
  }
  if (d > 90 && sitcounter == 0 && sitting == 1) {
    digitalWrite(LED, 0);
    sitting = 0;
  }

  return sitting;
}

int main() {
  double dist;
  int sitting;
  struct timeval lt, ct;

  wiringPiSetup();
  wiringPiSetupGpio();
  pinMode(LED, OUTPUT);

  while (1) {
    dist = getDistance();
    sitting = checkSitting(dist);
    // set up watchdog here to 
    delay(1000);
  }

  return 0;
}
