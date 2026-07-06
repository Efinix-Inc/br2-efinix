#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

void print_hex(const char *label, const char *str, int len) {
    printf("%s", label);
    for (int i = 0; i < len; i++) {
        printf("%02X ", (unsigned char)str[i]);
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    char *device = "/dev/rpmsg0";
    int iteration = 50;

    // Parse command line arguments
    if (argc >= 2) {
        device = argv[1];
    }
    if (argc >= 3) {
        iteration = atoi(argv[2]);
    }

    int fd = open(device, O_RDWR);
    if (fd < 0) {
        perror("Error: Cannot open RPMsg device");
        return 1;
    }

    printf("=== Starting OpenAMP Echo Test ===\n");
    printf("Target Device: %s\n", device);
    printf("Iterations   : %d\n----------------------------------------------\n", iteration);

    int sweep_sizes[] = {4, 15, 63, 64, 65, 127, 128, 129, 255, 256, 257, 496};

    int seq_num = 0;
    int success_count = 0;
    int failure_count = 0;
    char current_token = 'A';

    char tx_buf[512];
    char rx_buf[1024];

    for (int s = 0; s < iteration; s++) {
        int idx = s % 12;
        int current_len = sweep_sizes[idx];
        seq_num++;

        memset(tx_buf, current_token, current_len);
        tx_buf[current_len] = '\0';

        ssize_t written = write(fd, tx_buf, current_len);
        if (written < 0) {
            perror("\nFAIL: Write syscall error");
            failure_count++;
            break;
        }

        memset(rx_buf, 0, sizeof(rx_buf));
        ssize_t bytes_read = read(fd, rx_buf, sizeof(rx_buf) - 1);

        if (bytes_read > 0) {
            char cleaned_rx[1024];
            int c_idx = 0;
            for (int i = 0; i < bytes_read; i++) {
                if (rx_buf[i] != '\0' && rx_buf[i] != '\r' && rx_buf[i] != '\n') {
                    cleaned_rx[c_idx++] = rx_buf[i];
                }
            }
            cleaned_rx[c_idx] = '\0';

            char *extracted_data = cleaned_rx;
            if (strncmp(cleaned_rx, "BareMetal_", 10) == 0) {
                char *colon = strchr(cleaned_rx, ':');
                if (colon) {
                    extracted_data = colon + 1;
                    if (*extracted_data == ' ') extracted_data++;
                }
            }

            if (strcmp(tx_buf, extracted_data) == 0) {
                success_count++;
                printf("Packet #%d (Len: %d, Token: %c) -> PASSED \n", 
                       seq_num, current_len, current_token);
            } else {
                failure_count++;
                printf("\n========================================================\n");
                printf("FAIL: Data Mismatch on Size: %d bytes (Packet #%d)!\n", current_len, seq_num);
                printf("========================================================\n");
                printf("  Sent string     : %s\n", tx_buf);
                print_hex("  Sent Hex bytes  : ", tx_buf, current_len);
                printf("--------------------------------------------------------\n");
                printf("  Recv raw string : %s\n", rx_buf);
                print_hex("  Recv raw Hex    : ", rx_buf, bytes_read);
                printf("--------------------------------------------------------\n");
                printf("  Extracted data  : %s\n", extracted_data);
                print_hex("  Extracted Hex   : ", extracted_data, strlen(extracted_data));
                printf("========================================================\n\n");
            }
        } else {
            failure_count++;
            printf("FAIL: Timeout/Empty read on Swept Size: %d bytes (Packet #%d) \n", 
                   current_len, seq_num);
        }

        usleep(100000);

        // Rotate token character
        current_token++;
        if (current_token > 'Z') current_token = 'A';
    }

    printf("--------------------------------------------------------\n");
    printf("Echo Test Complete. Total: %d, Success: %d, Failures: %d\n", seq_num, success_count, failure_count);
    
    close(fd);
    return (failure_count == 0) ? 0 : 1;
}
