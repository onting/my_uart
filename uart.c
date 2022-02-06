#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>

static ssize_t uart_read(struct file *fp, char *buf, size_t nbytes, loff_t *off)
{
	return nbytes;
}

static ssize_t uart_write(struct file *fp, const char *str, size_t nbytes, loff_t *off)
{
	return nbytes;
}

static int uart_open(struct inode *node, struct file *fp)
{
	return 0;
}

static int uart_release(struct inode *node, struct file *fp)
{
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
		printk("UART has been installed\n");
	}
	
	return err;
}

static void __exit unregister_dev(void)
{
	unregister_chrdev(221, "my_uart");
}

module_init(register_dev);
module_exit(unregister_dev);

