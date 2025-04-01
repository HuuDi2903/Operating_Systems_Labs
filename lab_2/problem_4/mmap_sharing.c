#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#define FILE_SIZE 4096

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <mode>\n", argv[0]);
        printf("Mode: w - Write, r - Read\n");
        return 1;
    }
    
    char mode = argv[1][0];
    if (mode != 'w' && mode != 'r') {
        printf("Invalid mode. Use 'w' or 'r'\n");
        return 1;
    }
    
    int fd;
    void *mapped_mem;
    
    if (mode == 'w') {
        // Create/Open file for writing
        fd = open("shared_file.dat", O_RDWR | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            perror("open");
            exit(1);
        }
        
        // Extend file to required size
        if (lseek(fd, FILE_SIZE - 1, SEEK_SET) == -1) {
            perror("lseek");
            close(fd);
            exit(1);
        }
        
        // Write a byte at the end
        if (write(fd, "", 1) != 1) {
            perror("write");
            close(fd);
            exit(1);
        }
        
        // Map file to memory
        mapped_mem = mmap(NULL, FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (mapped_mem == MAP_FAILED) {
            perror("mmap");
            close(fd);
            exit(1);
        }
        
        // Now we can write to the mapped memory
        printf("Enter text to write to the shared file (max %d chars):\n", FILE_SIZE - 1);
        char input[FILE_SIZE];
        fgets(input, FILE_SIZE, stdin);
        
        // Copy input to mapped memory
        strcpy((char *)mapped_mem, input);
        printf("Data written to mapped file\n");
        
        // Ensure data is written to the file
        if (msync(mapped_mem, FILE_SIZE, MS_SYNC) == -1) {
            perror("msync");
        }
              
    } else {
        // Open existing file for reading
        fd = open("shared_file.dat", O_RDONLY);
        if (fd == -1) {
            perror("open");
            exit(1);
        }
        
        // Map file to memory
        mapped_mem = mmap(NULL, FILE_SIZE, PROT_READ, MAP_SHARED, fd, 0);
        if (mapped_mem == MAP_FAILED) {
            perror("mmap");
            close(fd);
            exit(1);
        }
        
        // Read and print content from mapped memory
        printf("Data read from mapped file:\n%s", (char *)mapped_mem);
    }
    
    // Unmap memory
    if (munmap(mapped_mem, FILE_SIZE) == -1) {
        perror("munmap");
    }
    
    // Close file
    close(fd);
    
    return 0;
}