#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_COMMAND_LENGTH 512
#define MAX_LINE_LENGTH 256

/*
 * This version of the C backup engine reads its configuration
 * from a CSV file, making it dynamic and highly reusable.
 *
 * The program now accepts the CSV file path as a command-line argument.
 * It reads each line from the file, parses the backup parameters,
 * and executes a remote rsync command for each job.
 */

// Struct to hold the parameters for a single backup job
typedef struct {
    char sourceUser[50];
    char sourceHost[50];
    char sourcePath[100];
    char destUser[50];
    char destHost[50];
    char destPath[100];
} BackupJob;

int main(int argc, char *argv[]) {
    // Check if the user provided a CSV file path as a command-line argument
    if (argc < 2) {
        printf("Modo de uso: %s <caminho_para_o_ficheiro_csv>\n", argv[0]);
        return 1;
    }

    const char *csvFilePath = argv[1];
    FILE *file = fopen(csvFilePath, "r");

    // Check if the file was opened successfully
    if (file == NULL) {
        perror("Erro ao abrir o ficheiro de configuração");
        return 1;
    }

    char line[MAX_LINE_LENGTH];
    char command[MAX_COMMAND_LENGTH];
    int result;

    printf("Iniciando o processo de backup diferencial a partir do ficheiro: %s\n", csvFilePath);
    printf("-------------------------------------------\n");

    // Skip the header line of the CSV file
    if (fgets(line, sizeof(line), file) == NULL) {
        printf("O ficheiro está vazio ou não tem cabeçalho.\n");
        fclose(file);
        return 1;
    }

    // Read the file line by line
    while (fgets(line, sizeof(line), file) != NULL) {
        // Remove the newline character from the end of the line
        line[strcspn(line, "\n")] = 0;

        // Use strtok to parse the CSV line
        // The strtok function is used to split a string into tokens based on a delimiter.
        // It modifies the original string, so we need to be careful with its usage.
        char *token = strtok(line, ",");
        if (token == NULL) continue; // Skip empty lines

        // Create a BackupJob struct and populate it with data from the CSV
        BackupJob job;
        strcpy(job.sourceHost, token);

        token = strtok(NULL, ",");
        if (token == NULL) continue;
        strcpy(job.sourceUser, token);

        token = strtok(NULL, ",");
        if (token == NULL) continue;
        strcpy(job.sourcePath, token);

        token = strtok(NULL, ",");
        if (token == NULL) continue;
        strcpy(job.destHost, token);

        token = strtok(NULL, ",");
        if (token == NULL) continue;
        strcpy(job.destUser, token);

        token = strtok(NULL, ",");
        if (token == NULL) continue;
        strcpy(job.destPath, token);

        // We ignore the StartTime column as requested
        // token = strtok(NULL, ",");
        // if (token == NULL) continue;
        // strcpy(job.startTime, token);

        // --- Construct the full shell command string for the current job ---
        // The structure is: ssh <source_user>@<source_host> "rsync <flags> <source_path> <dest_user>@<dest_host>:<dest_path>"
        snprintf(command, MAX_COMMAND_LENGTH,
                 "ssh %s@%s \"rsync -avz %s %s@%s:%s\"",
                 job.sourceUser, job.sourceHost, job.sourcePath, job.destUser, job.destHost, job.destPath);

        printf("Executando backup para a origem: %s@%s:%s\n", job.sourceUser, job.sourceHost, job.sourcePath);
        printf("Comando a ser executado:\n%s\n", command);
        printf("-------------------------------------------\n");

        // --- Execute the command using the system() function ---
        result = system(command);

        // --- Check the result of the command execution ---
        if (result == 0) {
            printf("Backup concluído com sucesso para este trabalho.\n");
        } else {
            printf("Erro no backup para este trabalho. Código de retorno: %d\n", result);
            fprintf(stderr, "Falha ao executar o comando. Por favor, verifique as configurações no ficheiro CSV.\n");
        }
        printf("-------------------------------------------\n");
    }

    fclose(file); // Close the file
    printf("Todos os trabalhos de backup foram processados.\n");

    return 0; // Return with success code
}
