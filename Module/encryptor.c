/**************************************************************
 * Class:  CSC-415-02 Spring 2023
 * Name:Christian McGlothen
 * Student ID: 918406078
 * GitHub UserID: Chris3953
 * Project: Assignment 6 -Device Driver 
 *
 * File: encryptor.c
 *
 * Description:  The encryptor.c file is an example of a simple Linux kernel module
 * that implements a character device driver named "encryptor." This device driver 
 * allows user programs to interact with it using standard file operations 
 * such as open, close, read, write, and also includes custom IOCTL (input/output control)
 * commands for managing a Caesar cipher-based encryption/decryption scheme. 

 *
 **************************************************************/
//include statements need to create a device driver
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/ioctl.h>

#define DEVICE_NAME "encryptor"

//creates unique keys so it does not interfere with 
//other running drivers iotcl commands
#define ENCRYPTOR_IOC_MAGIC 'e'
#define GET_KEY _IOR(ENCRYPTOR_IOC_MAGIC, 1, int)
#define SET_NEGATIVE_KEY _IOW(ENCRYPTOR_IOC_MAGIC, 2, int)
#define RESET_KEY _IOW(ENCRYPTOR_IOC_MAGIC, 3, int)

//module information 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Christian Mcglothen");
MODULE_DESCRIPTION("Encryptor Device Driver");





//device drivers use major and minor numbers to identify and communicate with devices
static int major_number;

static char result_str[100];
static int caesar_key = 3; // Default Caesar key

//Allows user to set the caesar key in parameters of sudo insmod/ or kernal parameter  
module_param(caesar_key, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(caesar_key, "Caesar key for encryption/decryption");

// Function to encrypt/decrypt a single character using the Caesar cipher
// It shifts alphabetic characters by a specified key value while maintaining case.

static char caesar_cipher(char ch, int key)
{
    // Check if the character is an uppercase letter (A-Z)
    if (ch >= 'A' && ch <= 'Z')
    {
        // Apply the Caesar cipher shift to uppercase letters
        // - 'A' is subtracted to normalize the character range to 0-25
        // - The key is added, and 26 is added to handle negative keys and wraparound
        // - The result is taken modulo 26 to ensure the shift stays within the alphabet range
        // - Finally, 'A' is added back to convert the normalized value back to an uppercase character
        ch = 'A' + (ch - 'A' + key + 26) % 26;
    }
    // Check if the character is a lowercase letter (a-z)
    else if (ch >= 'a' && ch <= 'z')
    {
        // Apply the Caesar cipher shift to lowercase letters
        // Similar to the uppercase case, but with 'a' used as the baseline
        ch = 'a' + (ch - 'a' + key + 26) % 26;
    }
    
    // Return the modified character, which may be shifted or unchanged
    return ch;
}


// driver open command, we dont do much because we are using the linux open function 
static int encryptor_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Encryptor device opened\n");
    return 0;
}
// driver release command, we dont do much because we are using the linux open function 
static int encryptor_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Encryptor device released\n");
    return 0;
}

//driver read command in which we read from the user buffer and copy it into our own buffer 
//called result_str 
static ssize_t encryptor_read(struct file *file, char __user *user_buffer, size_t count, loff_t *offset)
{
    int len = strlen(result_str);

    if (len > count)
        len = count;

    // Here is where we copy into our buffer 
    // this will work by calling read() in the test file 
    // by making a copy of whats in the device driver and displaying back to the user later 
    if (copy_to_user(user_buffer, result_str, len) != 0)
        return -EFAULT;

    return len;
}


// this is the devices driver verison of linux write command 
static ssize_t encryptor_write(struct file *file, const char __user *user_buffer, size_t count, loff_t *offset)
{
    char input_str[100];
    int i;

    if (count >= sizeof(input_str))
        return -EINVAL;
   
    //copying what the user gave us into our character array/buffer input_str
    // so we can encrypt or decrypt it 
    if (copy_from_user(input_str, user_buffer, count) != 0)
        return -EFAULT;

    // Decrypt the input string before storing it as the result
    for (i = 0; i < count; ++i)
    {
        input_str[i] = caesar_cipher(input_str[i], caesar_key);
    }

    // adds null terminator as we are given a c string 
    input_str[count] = '\0';

    //copies the encrypted message into our buffer result_str 
    // in which the read commands will display back to the user 
    snprintf(result_str, sizeof(result_str), "%s", input_str);

    printk(KERN_INFO "Received input: %s\n", input_str); // Print the received input
    return count;
}

//This method triggers certain actions in the test file in order to
//it helps retrive information or modify values 
static long encryptor_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
  
    int input;
    
    // the first if statement checks to see if the GET_KEY command was activated 
    if (cmd == GET_KEY)
    {   
        //if its acticated then we want to pass the caesar key to the test file 
        if (copy_to_user((int *)arg, &caesar_key, sizeof(int)) != 0)
            return -EFAULT;
    }
    //This if statement sets the caesar key to a negative value/ checks to see if it was 
    //activated in the test file 
    else if (cmd == SET_NEGATIVE_KEY)
    {   
        //copies the key into variable int input in order to change the sign value 
        if (copy_from_user(&input, (int *)arg, sizeof(int)) != 0)
            return -EFAULT;
        
        // Set the Caesar key to the negative value
        caesar_key = -input;
        printk(KERN_INFO "Caesar key set to: %d\n", caesar_key);
    
    }
    //Here the code repeats like above except this is a trigger to set the key 
    //back to its orginal state/value 
    else if (cmd == RESET_KEY)
    {
        if (copy_from_user(&input, (int *)arg, sizeof(int)) != 0)
            return -EFAULT;
        
        // Set the Caesar key to its original value
        caesar_key = input;
        printk(KERN_INFO "Caesar key set to: %d\n", caesar_key);
    }
    else
    {
        return -ENOTTY;
    }

    return 0;
}

//This tructure is key in "Over Writing"
// linux read, open, write commands 
// this structure tells the test file that when we use these linux
//commands such as write, to use the driver verison of Write 
static struct file_operations encryptor_fops = {
    .open = encryptor_open,
    .release = encryptor_release,
    .read = encryptor_read,
    .write = encryptor_write,
    .unlocked_ioctl = encryptor_ioctl,
};

//initialize the driver
static int __init encryptor_init(void)
{

    // Generate a major number in order to use sudo command mknod 
    //The mknod command in Unix-like operating systems is used to create special files, 
    //specifically character devices or block devices, which are used to interact with 
    //device drivers in the kernel. These files provide an interface for user-space applications 
    //to communicate with kernel-level device drivers.
    major_number = register_chrdev(0, DEVICE_NAME, &encryptor_fops);
    if (major_number < 0)
    {
        printk(KERN_ALERT "Failed to register encryptor device driver\n");
        return major_number;
    }

    printk(KERN_INFO "Encryptor device driver loaded with major number %d\n", major_number);

    printk(KERN_INFO "Caesar key set to: %d\n", caesar_key);

    return 0;
}

static void __exit encryptor_exit(void)
{
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "Encryptor device driver unloaded\n");
}

module_init(encryptor_init);
module_exit(encryptor_exit);

