
/**************************************************************
 * Class:  CSC-415-02 Spring 2023
 * Name:Christian McGlothen
 * Student ID: 918406078
 * GitHub UserID: Chris3953
 * Project: Assignment 6 -Device Driver 
 *
 * File: Mcglothen_Christian_HW6_main.c 
 *
 * Description: This is a test file that interacts with a device driver
 * by using linux open, read, and write commands. by using these commands it will perform 
 * the actions of encryptor device driver by writing user input string into the driver and reading 
 * the encrypted/decrypted message from the driver. 
 *
 **************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h> // Include the string.h header for strlen


// This #define statement creates a symbolic constant named DEVICE_PATH,
// which represents the file path to the device file used for communication
// with the encryptor driver.

#define DEVICE_PATH "/dev/encryptor"

//creates unique keys so it does not interfere with 
//other running drivers iotcl commands
#define ENCRYPTOR_IOC_MAGIC 'e'
#define GET_KEY _IOR(ENCRYPTOR_IOC_MAGIC, 1, int)
#define SET_NEGATIVE_KEY _IOW(ENCRYPTOR_IOC_MAGIC, 2, int)
#define RESET_KEY _IOW(ENCRYPTOR_IOC_MAGIC, 3, int)


int main()
{
    int fd; //file decriptor
    int ret, data;
    char result[100];

    int caesar_key; // Variable to store the Caesar key set in driver 

    // Open the device file
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0)
    {
        perror("Failed to open the device file");
        return -1;
    }

    // Get the Caesar key from the driver using an ioctl command
    ret = ioctl(fd, GET_KEY, &caesar_key);
    if (ret < 0)
    {
        perror("Failed to get the Caesar key");
        close(fd);
        return -1;
    }

   //prompting user for input decrypt or encrypt
    printf("You have set the key to %d\n", caesar_key);
    int choice;
    printf("Choose an option:\n");
    printf("1. Encrypt a message\n");
    printf("2. Decrypt a message\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);

    if (choice == 1)
    {
        // Brabs the user message 
        printf("Enter a message to encrypt: ");
        scanf(" %[^\n]", result);

        //adds null terminator 
        result[strlen(result)] = '\0';

        // Write the message to the device driver for encryption
        ret = write(fd, result, strlen(result));

        if (ret < 0)
        {
            perror("Failed to write to the device");
            close(fd);
            return -1;
        }

        // read back to user
        ret = read(fd, result, strlen(result));

        printf("Encrypted message: %s\n", result);
    }
    else if (choice == 2)
    {
        //if you are decrypting, then you want to flip the sign of caesar key 
        //ioctl command SET_NEGATIVE DOES THIS 
        ret = ioctl(fd, SET_NEGATIVE_KEY, &caesar_key);
        if (ret < 0)
         {
        perror("Failed To Make KEY NEGATIVE");
        close(fd);
        return -1;
         }
        // grab the users message 
        printf("Enter a message to decrypt: ");
        scanf(" %[^\n]", result);

         //Null terminator 
        result[strlen(result)] = '\0';
        
        //decrypt the message by writing to the deivce driver 
        ret = write(fd, result, strlen(result));

        if (ret < 0)
        {
            perror("Failed to write to the device");
            close(fd);
            return -1;
        }

        // read back to user
        ret = read(fd, result, strlen(result));

        printf("Decrypted message: %s\n", result);

        //flip the sign back to the original value for the next iteration of project 
        ret=ioctl(fd, RESET_KEY, &caesar_key); 
    }
    else
    {
        printf("Invalid choice\n");
    }

    // Close the device file
    close(fd);

    return 0;
}
