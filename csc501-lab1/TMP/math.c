#include <math.h>

double log(double x){
	int i;
	double result;
	double sum = -x;
	
	for(i = 2; i <= 20; i++){
		sum = sum - (pow(x,i)/i);
	}
	
	return sum;
}

double pow(double x, int y){
	if(y == 0){
		return 1;
	}
	if(x == 0){
		return 0;
	}
	
	int i;
	double result = x;
	for(i = 1; i < y; i++){
		result *= x;
	}
	
	return result;
}

double expdev(double lambda){
	double dummy;
	do
		dummy= (double) rand() / RAND_MAX;
	while(dummy == 0.0);
	return -log(dummy)/lambda;
}