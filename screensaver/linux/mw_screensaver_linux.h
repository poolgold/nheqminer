#include <math.h>

#define PI M_PI
#define PIx2 (M_PI * 2)
#define DEG2RAD (M_PI / 180)
#define RAD2DEG (180 / M_PI)

#define RSEPSILON 0.000001f
#define RS_PIo2 1.5707963268f
#define RS_PI 3.14159265359f
#define RS_PIx2 6.28318530718f

#ifndef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif

#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

#include <stdlib.h>

#define rsRandf(x) (((float)(x) * random()) / 2147483647)
#define rsRandi(x) (int)(rsRandf(x))
