#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
	int file;
	int adapterNumber = 1;
	char filename[20];

snprintf(filename, 19, "/dev/i2c-%d", adapterNumber);
	file = open(filename, O_RDWR);
	if (file < 0) {
				/* ERROR HANDLING; you can check errno to see what went wrong */
			exit(1);
		}

int addr = 0x4c; /* The I2C address */

if (ioctl(file, I2C_SLAVE, addr) < 0) {
				/* ERROR HANDLING; you can check errno to see what went wrong */
			exit(1);
		}

__u8 reg = 61; /* Device register to access */
	__s16 response;

response = i2c_smbus_read_byte_data(file, reg);
	if (response < 0) {
				printf("Error: i2c read failed\n");
		} else {
				printf("Response: 0x%x\n", response);
		}

return 0;
}

