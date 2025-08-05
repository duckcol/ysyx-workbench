#include <stdio.h>

int main(){
	int r1, r2;

	while (1) {
		pc0: r1 = 0;
		pc1: r2 = 0;
		pc2: r2 = r2 + 1;
		pc3: r1 = r1 + r2;
		pc4: if(r2 < 100) goto pc2;
		pc5: 
		{ printf("r1:%d, r2:%d\n", r1, r2);
			return 0;
		}
	}
}
