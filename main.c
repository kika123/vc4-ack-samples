#include <pi.h>
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
	char iptr[64];
	int* r2_i = (*pi_kernel_parameters).r2; /* WorkAround */
	void* r0 = (*pi_kernel_parameters).r0; /* Not Used : Use It As You Like */
	void* r1 = (*pi_kernel_parameters).r1; /* Future Expansion */
	char* r2 = (*pi_kernel_parameters).r2; /* ICE character printing function (busywait mechanism) */
	void* r3 = (*pi_kernel_parameters).r3; /* Debugging Structure */
	unsigned r4 = (*pi_kernel_parameters).r4; /* Not Used : Use It As You Like */
	unsigned r5 = (*pi_kernel_parameters).r5; /* Future Expansion */
	ice_puts(r2,"Hello from VideoCore ( ACK ) \n");
	if (  r3 == 0) {
		ice_puts(r2,"R3 pointer not supplied \n");
	}
	sprintf(iptr, "Address : 0x%08x \n" ,r3);
	r2[7] = 1;
	ice_puts(r2,iptr);
	exit(0);
	
}
