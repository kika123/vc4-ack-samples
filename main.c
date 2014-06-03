#include <pi.h>
int i = 0;
void ice_putc(char* rba, char c) {
	while(rba[0] != 0 ) { /* Long Penalty up to 2 seconds */
	}
	rba[1]=c;
	rba[0]=1;
}
void ice_puts(char* rba,char *c) {
	int i;
	for(i=0;c[i] != '\0';i++) {
		ice_putc(rba,c[i]);
	}
}
int main(void) { /* Bare Metal or Kernel VideoCore: No Argument Passing */
	
	/* The Program : Really ? : Commodity Functions */
	void* r0 = (*pi_kernel_parameters).r0; /* Not Used : Use It As You Like */
	void* r1 = (*pi_kernel_parameters).r1; /* Future Expansion */
	char* r2 = (*pi_kernel_parameters).r2; /* ICE character printing function (busywait mechanism) */
	void* r3 = (*pi_kernel_parameters).r3; /* ICE string storing for more than one character printing */
	unsigned r4 = (*pi_kernel_parameters).r4; /* Not Used : Use It As You Like */
	unsigned r5 = (*pi_kernel_parameters).r5; /* Future Expansion */
	ice_puts(r2,"Hello from VideoCore ( ACK ) \n");
	exit(0);
	for(i=0 ; i < (2048 * 16384) ; i++) {
	}
}
