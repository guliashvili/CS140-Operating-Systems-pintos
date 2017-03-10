#include "fixedpoint.h"

//#include "debug.h"
// void tests(){
// 	int x = 2.5 * fp;
// 	int y = 6.25 * fp;
// 	int z = 4.5 * fp;
// 	assert(multiplyFloats(x , x) == y);
// 	assert(divideFloats(y , x) == x);
// 	assert(sumFloatsAndInts(x, 2) == z);
// 	assert(subtractFloatsAndInts(z, 2) == x);
// }

fixedPoint intToFloat(int32_t n){
	return n*fp;

}
int32_t floatToInt(fixedPoint x){
	return x/fp;
}


int32_t floatToIntNearest(fixedPoint x){
	if(x>0) return (x + fp/2)/fp;
	return (x - fp/2)/fp;
}

fixedPoint sumFloats(fixedPoint x, fixedPoint y){
	return x+y;

}

fixedPoint subtractFloats(fixedPoint x, fixedPoint y){
	return x-y;
}

fixedPoint multiplyFloats(fixedPoint x, fixedPoint y){
	return (((int64_t) x) * y) / fp;

}

fixedPoint divideFloats(fixedPoint x, fixedPoint y){
	return (((int64_t) x) * fp) / y;
}

fixedPoint sumFloatsAndInts(fixedPoint x, int32_t n){
	return x+n*fp;
}

fixedPoint subtractFloatsAndInts(fixedPoint x, int32_t n){
	return x-n*fp;
}


fixedPoint multiplyFloatsAndInts(fixedPoint x, int32_t n){
	return x*n;
}

fixedPoint divideFloatsAndInts(fixedPoint x, int32_t n){
	return x/n;
}


// int main(){

// 	tests();
// 	return 0;
//}