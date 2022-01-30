#include <wiringPi.h>
#include <sys/time.h>
#include <stdio.h>

int ECHO = 6;
int TRIG = 5;
int LED = 23;

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

int checkSitting(double d) {
  int sitting = 0;

  if (d <= 90 && sitting == 0) {
    digitalWrite(LED, 1);
    sitting = 1;
  }
  if (d > 90) {
    digitalWrite(LED, 0);
    sitting = 0;
  }

  return sitting;
}

int main() {
  double dist;
  int  sitting;

  wiringPiSetup();
  wiringPiSetupGpio();
  pinMode(LED, OUTPUT);

  while (1) {
    dist = getDistance();
    sitting = checkSitting(dist);
    if (sitting) {}
    delay(1000);
  }

  return 0;
}
