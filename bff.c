#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BFF_VERSION_MAJOR 0
#define BFF_VERSION_MINOR 1
#define BFF_VERSION_PATCH 0

#define DEFAULT_MEMSIZE 30000

typedef struct context
{
    char* instructions;
    size_t instrIndex;
    unsigned char* memory;
    size_t memorySize;
    size_t memoryIndex;
} Context;

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
        perror("readFile - malloc");
        return NULL;
    }

    size = fread(buf, sizeof(char), size, file);
    buf[size] = '\0';

    fclose(file);

    return buf;
}

static int init(Context* ctx, const char* filename, size_t memorySize)
{
    ctx->instructions = readFile(filename);

    if (ctx->instructions == NULL)
        return EXIT_FAILURE;

    ctx->memory = calloc(memorySize, sizeof(unsigned char));

    if (ctx->memory == NULL) {
        free(ctx->instructions);
        perror("init - calloc");
        return EXIT_FAILURE;
    }

    ctx->instrIndex = 0;
    ctx->memorySize = memorySize;
    ctx->memoryIndex = 0;

    return EXIT_SUCCESS;
}

static void cleanup(Context* ctx)
{
    free(ctx->memory);
    free(ctx->instructions);
}

static void interpret(Context* ctx)
{
    char current;
    size_t loopStart, loopEnd;
    int count;

    while ((current = ctx->instructions[ctx->instrIndex])) {
        switch (current) {
            case '>':
                ++ctx->memoryIndex;
                break;

            case '<':
                --ctx->memoryIndex;
                break;

            case '+':
                ++ctx->memory[ctx->memoryIndex];
                break;

            case '-':
                --ctx->memory[ctx->memoryIndex];
                break;

            case '.':
                putchar(ctx->memory[ctx->memoryIndex]);
                break;

            case ',':
                ctx->memory[ctx->memoryIndex] = getchar();
                break;

            case '[':
                loopStart = ++ctx->instrIndex;

                for (count = 1, loopEnd = loopStart; count; ++loopEnd) {
                    if (ctx->instructions[loopEnd] == '[') ++count;
                    else if (ctx->instructions[loopEnd] == ']') --count;
                }

                while (ctx->memory[ctx->memoryIndex]) {
                    interpret(ctx);
                    ctx->instrIndex = loopStart;
                }

                ctx->instrIndex = loopEnd - 1;
                break;

            case ']':
                return;
        }

        ++ctx->instrIndex;
    }
}

static void usage(const char* programName, int status)
{
    if (status == EXIT_SUCCESS) {
        printf("Usage: %s [-hv] [-s memsize] FILE...\n"
            "\t-h: Display this help and exit\n"
            "\t-s memsize: Use memsize bytes of memory\n"
            "\t-v: Display version information and exit\n", programName);
    } else {
        fprintf(stderr, "Usage: %s [-hv] [-s memsize] FILE...\n"
            "Try '%s -h' for more information.\n", programName, programName);
    }

    exit(status);
}

static void version(const char* programName)
{
    printf("%s (Brainfuck Forever) %d.%d.%d\n"
        "Copyright (c) 2019 Ulysse RIGAUT.\n"
        "Released under the MIT License.\n",
        programName, BFF_VERSION_MAJOR, BFF_VERSION_MINOR, BFF_VERSION_PATCH);

    exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[])
{
    int opt, i;
    char* programName;
    int memorySize = DEFAULT_MEMSIZE;
    char* last = strrchr(argv[0], '/');

    programName = (last != NULL) ? ++last : argv[0];

    while ((opt = getopt(argc, argv, "hs:v")) != -1) {
        switch (opt) {
            case 'h':
                usage(programName, EXIT_SUCCESS);

            case 's':
                memorySize = atoi(optarg);
                break;

            case 'v':
                version(programName);

            default:
                usage(programName, EXIT_FAILURE);
        }
    }

    if (optind >= argc || memorySize <= 0)
        usage(programName, EXIT_FAILURE);

    for (i = optind; i < argc; ++i) {
        Context ctx;

        if (init(&ctx, argv[i], memorySize) == EXIT_FAILURE)
            continue;

        interpret(&ctx);

        cleanup(&ctx);
    }

    return EXIT_SUCCESS;
}
