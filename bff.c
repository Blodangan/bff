#include <stdio.h>
#include <stdlib.h>

static char* readFile(const char* filename)
{
    char* buf;
    long size;
    FILE* file = fopen(filename, "r");

    if (file == NULL) {
        perror(filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size = ftell(file);
    rewind(file);

    buf = malloc((size + 1) * sizeof(char));

    if (buf == NULL) {
        fclose(file);
        perror("malloc");
        return NULL;
    }

    fread(buf, sizeof(char), size, file);
    buf[size] = '\0';

    fclose(file);

    return buf;
}

int main(int argc, char *argv[])
{
    int i;

    if (argc == 1) {
        printf("Usage: %s FILE...\n", argv[0]);
        return EXIT_FAILURE;
    }

    for (i = 1; i < argc; ++i) {
        char* buf = readFile(argv[i]);

        if (buf == NULL)
            continue;

        printf("%s\n", buf);

        free(buf);
    }

    return EXIT_SUCCESS;
}
