#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>

#define UART_BAUD_RATE 115200

struct BCM2711_AUX {
	unsigned IRQ;
	unsigned ENABLES;
	unsigned MU_IO_REG;
	unsigned MU_IER_REG;
	unsigned MU_IIR_REG;
	unsigned MU_LCR_REG;
	unsigned MU_MCR_REG;
	unsigned MU_LSR_REG;
	unsigned MU_MSR_REG;
	unsigned MU_SCRATCH;
	unsigned MU_CNTL_REG;
	unsigned MU_STAT_REG;
	unsigned MU_BAUD_REG;
	unsigned SPI1_CNTL0_REG;
	unsigned SPI1_CNTL1_REG;
	unsigned SPI1_STAT_REG;
	unsigned SPI1_PEEK_REG;
	unsigned SPI1_IO_REGa;
	unsigned SPI1_IO_REGb;
	unsigned SPI1_IO_REGc;
	unsigned SPI1_IO_REGd;
	unsigned SPI1_TXHOLD_REGa;
	unsigned SPI1_TXHOLD_REGb;
	unsigned SPI1_TXHOLD_REGc;
	unsigned SPI1_TXHOLD_REGd;
	unsigned SPI2_CNTL0_REG;
	unsigned SPI2_CNTL1_REG;
	unsigned SPI2_STAT_REG;
	unsigned SPI2_PEEK_REG;
	unsigned SPI2_IO_REGa;
	unsigned SPI2_IO_REGb;
	unsigned SPI2_IO_REGc;
	unsigned SPI2_IO_REGd;
	unsigned SPI2_TXHOLD_REGa;
	unsigned SPI2_TXHOLD_REGb;
	unsigned SPI2_TXHOLD_REGc;
	unsigned SPI2_TXHOLD_REGd;
};

static struct BCM2711_AUX *p_AUX = (struct BCM2711_AUX *)0x7e2150000;
static int DeviceUseCount = 0;

static ssize_t uart_read(struct file *fp, char *user_buf, size_t nbytes, loff_t *off)
{
	char buf[8];
	int n;
	int len = 0;
	int err;

	while(len < nbytes) {
		n = 0;
		while(!(p_AUX->MU_IIR_REG & 1)); //polling
		while(n < sizeof(buf) && n < nbytes - len && p_AUX->MU_IIR_REG & 2) { // copy to buf
			buf[n++] = (char)p_AUX->MU_IO_REG;
		}
		err = copy_to_user(user_buf + len, buf, n); // copy to user buffer
		if(err < 0) {
			return -1;
		}
		len += n;
	}

	return len;
}

static ssize_t uart_write(struct file *fp, const char *user_str, size_t nbytes, loff_t *off)
{
	char buf[8];
	int i;
	int n;
	int len = 0;
	int err;

	while(len < nbytes) {
		while(!(p_AUX->MU_IIR_REG & 1)); //polling
		n = nbytes - len > sizeof(buf) ? sizeof(buf) : nbytes - len; //cut n into buf size
		err = copy_from_user(buf, user_str + len, n); //copy to kernel
		if(err < 0) { 
			return -1;
		}
		for(i = 0; i < n && p_AUX->MU_STAT_REG & 32; i++) { // send data
			p_AUX->MU_IO_REG = buf[i];
		}
		n = i;
		len += n;
	}

	return len;
}

static int uart_open(struct inode *node, struct file *fp)
{
	if(DeviceUseCount != 0) {
		return -EBUSY;
	}

	DeviceUseCount++;

	return 0;
}

static int uart_release(struct inode *node, struct file *fp)
{
	if(DeviceUseCount == 0) {
		return -EBADF;
	}

	DeviceUseCount--;
	return 0;
}


struct file_operations uart_fops = {
	.owner = THIS_MODULE,
	.read = uart_read,
	.write = uart_write,
	.open = uart_open,
	.release = uart_release,
};

static int __init register_dev(void)
{
	int err;

	err = register_chrdev(221, "my_uart", &uart_fops);
	if(err < 0) {
		printk("Unable to register driver\n");
	}
	else {
		p_AUX->ENABLES = 1; //enable mini UART
		p_AUX->MU_IER_REG = 3; //enable TX, RX interrupts
		p_AUX->MU_BAUD_REG = 250000000 / (8 * UART_BAUD_RATE) - 1;
		printk("UART driver has been installed\n");
	}
	
	return err;
}

static void __exit unregister_dev(void)
{
	p_AUX->ENABLES = 0; //disable entire peripherals
	unregister_chrdev(221, "my_uart");
}

module_init(register_dev);
module_exit(unregister_dev);

