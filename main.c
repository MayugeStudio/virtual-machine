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
    size_t len;
    size_t capacity;
} Tokens;

#define DYN_ALLAY_DEFAULT_CAPACITY 256

void push_token(Tokens *s, Token token)
{
    if (s->len >= s->capacity) {
        if (s->capacity == 0) s->capacity = DYN_ALLAY_DEFAULT_CAPACITY;
        else s->capacity *= 2;
        s->items = realloc(s->items, s->capacity*sizeof(*s->items));
    }
    s->items[s->len++] = token;
}

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


void lex_file(Lexer *l, const char *content, const size_t content_size)
{
    size_t cur = 0;
    size_t begin = 0;

    while (cur < content_size) {
        cur += 1;
        if (isspace(content[cur])) {
            Token token = { .begin=&content[begin], .end=&content[cur], .eol=false };
            printf("INFO: \"%.*s\"\n", (int)(token.end-token.begin), token.begin);
            push_token(&l->tokens, token);
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
    printf("------------------\n");
    for (int i=0; i<(int)lexer->tokens.len; ++i) {
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
