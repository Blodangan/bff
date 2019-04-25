#include <stdio.h>
#include <stdlib.h>

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
        perror("malloc");
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
        perror("calloc");
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

#define MEMORY_SIZE 30000

int main(int argc, char* argv[])
{
    int i;

    if (argc == 1) {
        fprintf(stderr, "Usage: %s FILE...\n", argv[0]);
        return EXIT_FAILURE;
    }

    for (i = 1; i < argc; ++i) {
        Context ctx;

        /* TODO : getopt */
        if (init(&ctx, argv[i], MEMORY_SIZE) == EXIT_FAILURE)
            continue;

        interpret(&ctx);

        cleanup(&ctx);
    }

    return EXIT_SUCCESS;
}
