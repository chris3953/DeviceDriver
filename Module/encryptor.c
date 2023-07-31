#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/ioctl.h>

#define DEVICE_NAME "encryptor"
#define CALC_IOC_MAGIC 'c'
#define CALC_IOC_SUM _IOWR(CALC_IOC_MAGIC, 1, int)

static int major_number;
static int result = 0; // Define the 'result' variable here

static int calculator_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Encryptor device opened\n");
    return 0;
}

static int calculator_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Encryptor device released\n");
    return 0;
}

static ssize_t calculator_read(struct file *file, char __user *user_buffer, size_t count, loff_t *offset)
{
    int len;
    char result_str[16];

    snprintf(result_str, sizeof(result_str), "%d\n", result);
    len = strlen(result_str);

    if (copy_to_user(user_buffer, result_str, len) != 0)
        return -EFAULT;

    return len;
}

static ssize_t calculator_write(struct file *file, const char __user *user_buffer, size_t count, loff_t *offset)
{
    char input_str[16];

    if (copy_from_user(input_str, user_buffer, count) != 0)
        return -EFAULT;

    input_str[count] = '\0';
    sscanf(input_str, "%d", &result);

    return count;
}

static long calculator_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int input;

    if (_IOC_TYPE(cmd) != CALC_IOC_MAGIC)
        return -ENOTTY;

    if (_IOC_NR(cmd) > 1)
        return -ENOTTY;

    if (copy_from_user(&input, (int *)arg, sizeof(int)) != 0)
        return -EFAULT;

    switch (cmd) {
        case CALC_IOC_SUM:
            result += input;
            break;
        default:
            return -ENOTTY;
    }

    return 0;
}

static struct file_operations calculator_fops = {
    .open = calculator_open,
    .release = calculator_release,
    .read = calculator_read,
    .write = calculator_write,
    .unlocked_ioctl = calculator_ioctl,
};

static int __init calculator_init(void)
{
    major_number = register_chrdev(0, DEVICE_NAME, &calculator_fops);
    if (major_number < 0) {
        printk(KERN_ALERT "Failed to register encryptor device driver\n");
        return major_number;
    }

    printk(KERN_INFO "Encryptor device driver loaded with major number %d\n", major_number);
    return 0;
}

static void __exit calculator_exit(void)
{
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "Encryptor device driver unloaded\n");
}

module_init(calculator_init);
module_exit(calculator_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Christian Mcglothen");
MODULE_DESCRIPTION("Encryptor Device Driver");
