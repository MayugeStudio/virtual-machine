#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

//--------------------------------------------------
// File
//--------------------------------------------------

char *File_read_all(const char *path, size_t *size)
{
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not open file: \"%s\"\n", path);
        exit(EXIT_FAILURE);
    }
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    *size = file_size;
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *)malloc(file_size + 1); // +1 for null-termination-string
    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read file: \"%s\" \n", path);
        exit(EXIT_FAILURE);
    }

    int ret = fread(buffer, sizeof(char), file_size, file);
    if (ret < 0) {
        fprintf(stderr, "Could not read file: \"%s\"\n", path);
        exit(EXIT_FAILURE);
    }
    return buffer;
}

typedef struct Token {
    const char *begin;
    const char *end;
    bool eol;
} Token;

typedef struct Tokens {
    struct Token *items;
    size_t count;
    size_t capacity;
} Tokens;

#define DYN_ALLAY_DEFAULT_CAPACITY 256

#define DA_APPEND(xs, item) \
    do { \
        if ((xs)->count >= (xs)->capacity) { \
            (xs)->capacity = (xs)->capacity == 0 ? DYN_ALLAY_DEFAULT_CAPACITY : (xs)->capacity * 2; \
            (xs)->items = realloc((xs)->items, (xs)->capacity*sizeof(*(xs)->items)); \
        } \
        (xs)->items[(xs)->count++] = (item); \
    } while (0)


typedef struct Lexer {
    struct Tokens tokens;
} Lexer;


// TODO: use arena allocator
Lexer *new_lexer()
{
    Lexer *lexer = (Lexer *)malloc(sizeof(Lexer));
    if (lexer == NULL) {
        fprintf(stderr, "Not enough memory to allocate\n");
        exit(EXIT_FAILURE);
    }
    return lexer;
}

// TODO: Introduce 'next_token' function or something like this.
void lex_file(Lexer *l, const char *content, const size_t content_size)
{
    size_t cur = 0;
    size_t begin = 0;

    while (cur < content_size) {
        cur += 1;
        if (isspace(content[cur])) {
            Token token = { .begin=&content[begin], .end=&content[cur], .eol=false };
            DA_APPEND(&l->tokens, token);
            cur += 1;
            begin = cur;
            continue;
        }
    }
}

int main(void)
{
    size_t size;
    char *buffer = File_read_all("example.vasm", &size);
    Lexer *lexer = new_lexer();
    lex_file(lexer, buffer, size);
    for (int i=0; i<(int)lexer->tokens.count; ++i) {
        Token t = lexer->tokens.items[i];
        if (t.eol) {
            printf("%d: EOL\n", i+1);
        } else {
            printf("%d: \'%.*s\'\n", i+1, (int)(t.end - t.begin), t.begin);
        }
    }

    free(buffer);
    free(lexer);
    return 0;
}

// : 1 1 + done get_at @1

