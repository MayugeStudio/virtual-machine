#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define ARRAY_LEN(xs) sizeof(xs)/sizeof(*xs)
#define DA_ALLAY_DEFAULT_CAPACITY 256
#define DA_APPEND(xs, item) \
    do { \
        if ((xs)->count >= (xs)->capacity) { \
            (xs)->capacity = (xs)->capacity == 0 ? DA_ALLAY_DEFAULT_CAPACITY : (xs)->capacity * 2; \
            (xs)->items = realloc((xs)->items, (xs)->capacity*sizeof(*(xs)->items)); \
        } \
        (xs)->items[(xs)->count++] = (item); \
    } while (0)

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

// TODO: Replace `end` with `len`
typedef struct Token {
    const char *begin;
    const char *end;
    bool eof;
} Token;

bool token_equal_cstr(Token t, const char *cstr)
{
    size_t t_len = t.end - t.begin;
    size_t c_len = strlen(cstr);
    if (t_len != c_len) return false;

    for (size_t i=0; i < t_len && cstr[i] != '\0'; ++i) {
        if (t.begin[i] != cstr[i]) {
            return false;
        }
    }
    return true;
}

int token_to_int(Token t)
{
    size_t t_len = t.end - t.begin;
    int n = 0;
    // TODO: Support proper error message.
    for (size_t i=0; i<t_len; ++i) {
        assert(isdigit(t.begin[i]));
        n *= 10;
        n += t.begin[i] - '0';
    }

    return n;
}

typedef struct Tokens {
    struct Token *items;
    size_t count;
    size_t capacity;
} Tokens;

typedef enum OpKind {
    OpKind_Push = 0,
    OpKind_Add,
    OpKind_Write,
} OpKind;

const char *op_names[] = {
    "push",
    "add",
    "write",
};

const OpKind op_kinds[] = {
    OpKind_Push,
    OpKind_Add,
    OpKind_Write,
};

const size_t op_operand_counts[] = {
    [OpKind_Push] = 1,
    [OpKind_Add] = 0,
    [OpKind_Write] = 1,
};

typedef struct Operation {
    OpKind kind;
    int operand;
} Operation;

typedef struct Operations {
    struct Operation *items;
    size_t count;
    size_t capacity;
} Operations;

typedef struct Lexer {
    const char *file_path;
    struct Tokens tokens;
    struct Operations ops;
} Lexer;

// TODO: use arena allocator
Lexer *new_lexer(const char *file_path)
{
    Lexer *lexer = (Lexer *)malloc(sizeof(Lexer));
    if (lexer == NULL) {
        fprintf(stderr, "Not enough memory to allocate\n");
        exit(EXIT_FAILURE);
    }
    lexer->file_path = file_path;
    return lexer;
}

// TODO: Introduce 'next_token' function or something like this.
void lex_file(Lexer *l, const char *content, const size_t content_size)
{
    size_t cur = 0;
    size_t begin = 0;

    while (cur <= content_size) {
        cur += 1;
        if (isspace(content[cur])) {
            Token token = { .begin=&content[begin], .end=&content[cur], .eof=false };
            DA_APPEND(&l->tokens, token);
            cur += 1;
            begin = cur;
            continue;
        }
    }

    Token token = { .begin=&content[begin], .end=&content[begin], .eof=true };
    DA_APPEND(&l->tokens, token);
}

// TODO: Support zero token code.
void parse_tokens(Lexer *l)
{
    size_t n = 0;
    while (!l->tokens.items[n].eof) {
        for (size_t i=0; i<ARRAY_LEN(op_names); ++i) {
            assert(ARRAY_LEN(op_names) == ARRAY_LEN(op_kinds));
            if (token_equal_cstr(l->tokens.items[n], op_names[i])) {
                printf("Equal\n");
                int operand = -1;
                if (op_operand_counts[i] != 0) {
                    // TODO: Support several operands.
                    // TODO: Use error-message-function Instead of assertion.
                    assert(n+1 < l->tokens.count);
                    // NOTE: parse next token to get operand.
                    //       That means next token of operation is expected always operand.
                    n += 1;
                    operand = token_to_int(l->tokens.items[n]);
                }
                Operation op = {
                    .kind = op_kinds[i],
                    .operand = operand,
                };
                DA_APPEND(&l->ops, op);
                break;
            }
        }
        n+=1;
    }
}

int main(void)
{
    size_t size;
    const char *file_path = "example.vasm";
    char *buffer = File_read_all(file_path, &size);
    Lexer *lexer = new_lexer(file_path);
    lex_file(lexer, buffer, size);
    for (size_t i=0; i<lexer->tokens.count; ++i) {
        Token t = lexer->tokens.items[i];
        printf("%ld: \'%.*s\' %s \n", i+1, (int)(t.end - t.begin), t.begin, t.eof ? "EOF" : "");
    }

    parse_tokens(lexer);
    printf("------------------------------------------\n");
    printf("Operations:\n");

    for (size_t i=0; i<lexer->ops.count; ++i) {
        Operation op = lexer->ops.items[i];
        printf("kind: %d\n operand: %d\n", (int)op.kind, op.operand);
        printf("########################\n");
    }

    free(buffer);
    free(lexer);
    return 0;
}

// : 1 1 + done get_at @1

