#ifndef DISPLAY_H
#define DISPLAY_H

#include <MD_Parola.h>

#define MAX_LABEL      36 // up to 25 for username + 4 for " ()\0"; this leaves a minimum of 7 for amount
#define MAX_ALERT      64
#define MAX_MESSAGE    512

#define SIZE_ALERTS    24
#define SIZE_LABEL     10

#define LABEL_SPEED    50

#define LABEL_START(z) (SIZE_ALERTS + SIZE_LABEL*((z) - 1))
#define LABEL_END(z)   (SIZE_ALERTS + SIZE_LABEL*(z) - 1)

enum MatrixZone { ZONE_ALERTS, ZONE_FOLLOW, ZONE_SUB, ZONE_CHEER };
enum EventType : char {
  EVENT_FOLLOW = 'F',
  EVENT_SUB_NEW = 'S',
  EVENT_SUB_RENEW = 's',
  EVENT_SUB_GIFT = 'G',
  EVENT_CHEER = 'C',
  EVENT_RAID = 'R',
  EVENT_DONATE = 'D',
  EVENT_SHOUTOUT = 'O',
};

class LabelDisplay {
public:
  LabelDisplay(MD_Parola* P, MatrixZone zone);
  void begin();
  void set(const char *user, uint32_t num);
  void animate();
private:
  MD_Parola* _P;
  const MatrixZone _z;
  char buffer[MAX_LABEL];
};

// TODO handle timed rewards
class AlertDisplay {
public:
  enum AlertStage { ALERT_IDLE, ALERT_EVENT, ALERT_USER, ALERT_MESSAGE };

  AlertDisplay(MD_Parola* P, char* user, char* message);
  void begin();
  void set(EventType eventType, uint32_t num);
  void animate();
private:
  MD_Parola* _P;
  // taken straight from the serial input buffers to conserve space
  char* _user;
  char* _message;
  char alert[MAX_ALERT];
  bool withMessage;
  AlertStage stage = ALERT_IDLE;
};

#endif