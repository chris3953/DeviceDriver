#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DEVICE_PATH "/dev/encryptor"
#define ENCRYPT _IO('k', 1)
#define DECRYPT _IO('k', 2)

int main()
{
    int fd, result;
    char message[256];

    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        printf("Failed to open the encryptor device\n");
        return -1;
    }

    // Encrypt mode
    ioctl(fd, ENCRYPT);

    // Writing the message to encrypt
    printf("Enter the message to encrypt: ");
    fgets(message, sizeof(message), stdin);
    write(fd, message, strlen(message));

    // Reading the encrypted message
    read(fd, message, sizeof(message));
    printf("Encrypted message: %s\n", message);

    // Decrypt mode
    ioctl(fd, DECRYPT);

    // Writing the message to decrypt
    printf("Enter the message to decrypt: ");
    fgets(message, sizeof(message), stdin);
    write(fd, message, strlen(message));

    // Reading the decrypted message
    read(fd, message, sizeof(message));
    printf("Decrypted message: %s\n", message);

    close(fd);

    return 0;
}
