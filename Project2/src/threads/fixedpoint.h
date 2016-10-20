#ifndef THREADS_FIXEDPOINT_H
#define THREADS_FIXEDPOINT_H

#include <stdint.h>
#define fp 16384


typedef int32_t fixedPoint;

fixedPoint intToFloat(int32_t n);
int32_t floatToInt(fixedPoint x);
int32_t floatToIntNearest(fixedPoint x);

fixedPoint sumFloats(fixedPoint x, fixedPoint y);
fixedPoint subtractFloats(fixedPoint x, fixedPoint y);
fixedPoint multiplyFloats(fixedPoint x, fixedPoint y);
fixedPoint divideFloats(fixedPoint x, fixedPoint y);

fixedPoint sumFloatsAndInts(fixedPoint x, int32_t n);
fixedPoint subtractFloatsAndInts(fixedPoint x, int32_t n);
fixedPoint multiplyFloatsAndInts(fixedPoint x, int32_t n);
fixedPoint divideFloatsAndInts(fixedPoint x, int32_t n);

#endif