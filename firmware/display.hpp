#include <MD_Parola.h>

#define MAX_LABEL     36 // up to 25 for username + 4 for " ()\0"; this leaves a minimum of 7 for amount
#define MAX_MESSAGE   512

#define SIZE_ALERTS   24
#define SIZE_LABEL    10

#define LABEL_SPEED   50

#define ZONE_END(z)   (SIZE_ALERTS + SIZE_LABEL*(z) - 1)
#define ZONE_BEGIN(z) (ZONE_END(z) - ((z) ? SIZE_LABEL : SIZE_ALERTS) + 1)

class LabelDisplay {
public:
    enum MatrixZone { ZONE_ALERTS, ZONE_FOLLOW, ZONE_SUB, ZONE_CHEER };

    LabelDisplay(MD_Parola* P, MatrixZone zone);
    void begin();
    void set(const char *user, uint32_t num);
protected:
    MD_Parola* _P;
    const MatrixZone _z;
    char buffer[MAX_LABEL];
};