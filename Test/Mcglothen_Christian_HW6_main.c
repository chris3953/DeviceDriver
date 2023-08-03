#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h> // Include the string.h header for strlen

#define DEVICE_PATH "/dev/encryptor"
#define CALC_IOC_MAGIC 'c'
#define CALC_IOC_SUM _IOWR(CALC_IOC_MAGIC, 1, int)
#define CALC_IOC_GET_KEY _IOR(CALC_IOC_MAGIC, 2, int)


// Caesar key used in the driver (should match the value in the driver)

// Function to encrypt/decrypt a single character using the Caesar cipher
static char caesar_cipher(char ch, int key) {
    if (ch >= 'A' && ch <= 'Z') {
        ch = 'A' + (ch - 'A' + key + 26) % 26;
    } else if (ch >= 'a' && ch <= 'z') {
        ch = 'a' + (ch - 'a' + key + 26) % 26;
    }
    return ch;
}

/// @brief 
/// @return 
int main() {
    int fd;
    int ret, data;
    char buffer[100];
    char result[100];

    int caesar_key; // Variable to store the Caesar key

    // Open the device file
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open the device file");
        return -1;
    }

    // Get the Caesar key from the driver using an ioctl command
    ret = ioctl(fd, CALC_IOC_GET_KEY, &caesar_key);
    if (ret < 0) {
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

    if (choice == 1) {
        // Encrypt a message
        printf("Enter a message to encrypt: ");
        scanf(" %[^\n]", buffer);

        // Encrypt the message using the Caesar cipher
        for (int i = 0; i < strlen(buffer); ++i) {
            result[i] = buffer[i]; 
        }
        result[strlen(buffer)] = '\0';

        // Write the encrypted message to the device
        ret = write(fd, result, strlen(result));
        if (ret < 0) {
            perror("Failed to write to the device");
            close(fd);
            return -1;
        }

         //read back to user
         ret = read(fd, result, strlen(result));

        printf("Encrypted message: %s\n", result);
    } else if (choice == 2) {
        // Decrypt a message
        printf("Enter a message to decrypt: ");
        scanf(" %[^\n]", buffer);

        // Decrypt the message using the Caesar cipher
        for (int i = 0; i < strlen(buffer); ++i) {
            result[i] = caesar_cipher(buffer[i], -caesar_key);
        }
        result[strlen(buffer)] = '\0';

        printf("Decrypted message: %s\n", result);
    } else {
        printf("Invalid choice\n");
    }

  
    
    // Close the device file
    close(fd);

    return 0;
}

