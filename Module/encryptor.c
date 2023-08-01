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

#define CAESAR_KEY 3 // You can change the Caesar key here

static int major_number;
static char result_str[100];
static int result = 0; // Define the 'result' variable here

// Function to encrypt/decrypt a single character using the Caesar cipher
// Function to encrypt/decrypt a single character using the Caesar cipher
static char caesar_cipher(char ch, int key)
{
    if (ch >= 'A' && ch <= 'Z')
    {
        ch = 'A' + (ch - 'A' + key + 26) % 26;
    }
    else if (ch >= 'a' && ch <= 'z')
    {
        ch = 'a' + (ch - 'a' + key + 26) % 26;
    }
    return ch;
}

static int encryptor_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Encryptor device opened\n");
    return 0;
}

static int encryptor_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Encryptor device released\n");
    return 0;
}

static ssize_t encryptor_read(struct file *file, char __user *user_buffer, size_t count, loff_t *offset)
{
    int len = strlen(result_str);
    int i;

    // Encrypt the result string before sending it to the user
    for (i = 0; i < len; ++i)
    {
        result_str[i] = caesar_cipher(result_str[i], CAESAR_KEY);
    }

    if (copy_to_user(user_buffer, result_str, len) != 0)
        return -EFAULT;

    return len;
}

static ssize_t encryptor_write(struct file *file, const char __user *user_buffer, size_t count, loff_t *offset)
{
    char input_str[100];
    int i;

    if (count >= sizeof(input_str))
        return -EINVAL;

    if (copy_from_user(input_str, user_buffer, count) != 0)
        return -EFAULT;

    // Decrypt the input string before storing it as the result
    for (i = 0; i < count; ++i)
    {
        input_str[i] = caesar_cipher(input_str[i], -CAESAR_KEY);
    }

    input_str[count] = '\0';
    snprintf(result_str, sizeof(result_str), "%s", input_str);

    return count;
}

static long encryptor_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int input;

    if (_IOC_TYPE(cmd) != CALC_IOC_MAGIC)
        return -ENOTTY;

    if (_IOC_NR(cmd) > 1)
        return -ENOTTY;

    if (copy_from_user(&input, (int *)arg, sizeof(int)) != 0)
        return -EFAULT;

    switch (cmd)
    {
    case CALC_IOC_SUM:
        result += input;
        break;
    default:
        return -ENOTTY;
    }

    return 0;
}

static struct file_operations encryptor_fops = {
    .open = encryptor_open,
    .release = encryptor_release,
    .read = encryptor_read,
    .write = encryptor_write,
    .unlocked_ioctl = encryptor_ioctl,
};

static int __init encryptor_init(void)
{
    major_number = register_chrdev(0, DEVICE_NAME, &encryptor_fops);
    if (major_number < 0)
    {
        printk(KERN_ALERT "Failed to register encryptor device driver\n");
        return major_number;
    }

    printk(KERN_INFO "Encryptor device driver loaded with major number %d\n", major_number);
    return 0;
}

static void __exit encryptor_exit(void)
{
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "Encryptor device driver unloaded\n");
}

module_init(encryptor_init);
module_exit(encryptor_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Christian Mcglothen");
MODULE_DESCRIPTION("Encryptor Device Driver");
