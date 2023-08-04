#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h> // Include the string.h header for strlen

#define DEVICE_PATH "/dev/encryptor"
#define ENCRYPTOR_IOC_MAGIC 'e'
#define GET_KEY _IOR(ENCRYPTOR_IOC_MAGIC, 1, int)
#define SET_NEGATIVE_KEY _IOW(ENCRYPTOR_IOC_MAGIC, 2, int)
#define RESET_KEY _IOW(ENCRYPTOR_IOC_MAGIC, 3, int)


int main()
{
    int fd;
    int ret, data;
    char buffer[100];
    char result[100];

    int caesar_key; // Variable to store the Caesar key

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

    printf("You have set the key to %d\n", caesar_key);
    int choice;
    printf("Choose an option:\n");
    printf("1. Encrypt a message\n");
    printf("2. Decrypt a message\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);

    if (choice == 1)
    {
        // Encrypt a message
        printf("Enter a message to encrypt: ");
        scanf(" %[^\n]", buffer);

        // Encrypt the message using the Caesar cipher
        for (int i = 0; i < strlen(buffer); ++i)
        {
            result[i] = buffer[i];
        }
        result[strlen(buffer)] = '\0';

        // Write the encrypted message to the device
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
        ret = ioctl(fd, SET_NEGATIVE_KEY, &caesar_key);
        if (ret < 0)
         {
        perror("Failed To Make KEY NEGATIVE");
        close(fd);
        return -1;
         }
        // Decrypt a message
        printf("Enter a message to decrypt: ");
        scanf(" %[^\n]", buffer);

        // Decrypt the message using the Caesar cipher
        for (int i = 0; i < strlen(buffer); ++i)
        {
            result[i] = buffer[i];
        }
        result[strlen(buffer)] = '\0';

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
