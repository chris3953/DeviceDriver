#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h> // Include the string.h header for strlen

#define DEVICE_PATH "/dev/encryptor"
#define CALC_IOC_MAGIC 'c'
#define CALC_IOC_SUM _IOWR(CALC_IOC_MAGIC, 1, int)

// Caesar key used in the driver (should match the value in the driver)
#define CAESAR_KEY 3

// Function to encrypt/decrypt a single character using the Caesar cipher
static char caesar_cipher(char ch, int key) {
    if (ch >= 'A' && ch <= 'Z') {
        ch = 'A' + (ch - 'A' + key + 26) % 26;
    } else if (ch >= 'a' && ch <= 'z') {
        ch = 'a' + (ch - 'a' + key + 26) % 26;
    }
    return ch;
}

int main() {
    int fd;
    int ret, data;
    char buffer[100];
    char decrypt[100]; 
    

    // Open the device file
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open the device file");
        return -1;
    }

    // Test write: send data to the device
    printf("Enter a message to encrypt: ");
    fgets(buffer, sizeof(buffer), stdin);

    // Remove the trailing newline character, if present
    int len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }

    ret = write(fd, buffer, strlen(buffer));
    if (ret < 0) {
        perror("Failed to write to the device");
        close(fd);
        return -1;
    }

    // Test read: receive the encrypted data from the device
    ret = read(fd, buffer, sizeof(buffer));
    if (ret < 0) {
        perror("Failed to read from the device");
        close(fd);
        return -1;
    }
    

    // Test ioctl: add a value to the result
    data = 10;
    ret = ioctl(fd, CALC_IOC_SUM, &data);
    if (ret < 0) {
        perror("Failed to perform ioctl");
        close(fd);
        return -1;
    }

    // Test read after ioctl: receive the updated encrypted message from the device
    ret = read(fd, buffer, sizeof(buffer));
    if (ret < 0) {
        perror("Failed to read from the device");
        close(fd);
        return -1;
    }
    printf("Updated encrypted message: %s\n", buffer);

    printf("Enter a message to decrypt: "); 

    fgets(decrypt, sizeof(decrypt), stdin);

    // Decrypt the received message
    for (int i = 0; i < ret; ++i) {
        decrypt[i] = caesar_cipher(decrypt[i], -CAESAR_KEY);
    }
    decrypt[ret] = '\0';

    printf("Decrypted message: %s\n", decrypt);

    // Close the device file
    close(fd);

    return 0;
}
