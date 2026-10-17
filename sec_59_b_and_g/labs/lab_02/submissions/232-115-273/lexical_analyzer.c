
#include <stdio.h>
#include <string.h>

int is_space(int c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

int is_letter(int c) {
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z') || c == '_';
}

int is_digit(int c) {
    return c >= '0' && c <= '9';
}

int is_alnum(int c) {
    return is_letter(c) || is_digit(c);
}

int is_keyword(const char *s) {
    const char *keywords[] = {"int", "if", "else", "while", "return"};

    for (int i = 0; i < 5; i++) {
        if (strcmp(s, keywords[i]) == 0)
            return 1;
    }
    return 0;
}

void emit(const char *lexeme, const char *token) {
    printf("%s,%s\n", lexeme, token);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: lexical_analyzer input.c\n");
        return 1;
    }

    FILE *input = fopen(argv[1], "rb");
    if (input == NULL) {
        perror("Input file");
        return 1;
    }

    int c;

    while ((c = fgetc(input)) != EOF) {
        if (is_space(c))
            continue;

        char lexeme[129];
        int i = 0;

        if (is_letter(c)) {
            do {
                lexeme[i++] = (char)c;
                c = fgetc(input);
            } while (c != EOF && is_alnum(c));

            lexeme[i] = '\0';
            if (c != EOF)
                ungetc(c, input);

            emit(lexeme, is_keyword(lexeme) ? "KEYWORD" : "IDENTIFIER");
            continue;
        }

        if (is_digit(c)) {
            do {
                lexeme[i++] = (char)c;
                c = fgetc(input);
            } while (c != EOF && is_digit(c));

            lexeme[i] = '\0';
            if (c != EOF)
                ungetc(c, input);

            emit(lexeme, "INTEGER");
            continue;
        }

        if (c == '/') {
            int next = fgetc(input);

            if (next == '/') {
                while ((c = fgetc(input)) != EOF && c != '\n')
                    ;
                continue;
            }

            if (next == '*') {
                int previous = 0;
                int closed = 0;

                while ((c = fgetc(input)) != EOF) {
                    if (previous == '*' && c == '/') {
                        closed = 1;
                        break;
                    }
                    previous = c;
                }

                if (!closed) {
                    fprintf(stderr, "Error: unclosed block comment\n");
                    fclose(input);
                    return 1;
                }
                continue;
            }

            if (next != EOF)
                ungetc(next, input);

            emit("/", "ARITHMETIC_OPERATOR");
            continue;
        }

        int next = fgetc(input);

        if ((c == '=' && next == '=') ||
            (c == '!' && next == '=') ||
            (c == '<' && next == '=') ||
            (c == '>' && next == '=') ||
            (c == '&' && next == '&') ||
            (c == '|' && next == '|')) {

            lexeme[0] = (char)c;
            lexeme[1] = (char)next;
            lexeme[2] = '\0';

            if (c == '&' || c == '|')
                emit(lexeme, "LOGICAL_OPERATOR");
            else
                emit(lexeme, "RELATIONAL_OPERATOR");

            continue;
        }

        if (next != EOF)
            ungetc(next, input);

        lexeme[0] = (char)c;
        lexeme[1] = '\0';

        if (c == '=')
            emit(lexeme, "ASSIGNMENT_OPERATOR");
        else if (c == '<' || c == '>')
            emit(lexeme, "RELATIONAL_OPERATOR");
        else if (c == '!')
            emit(lexeme, "LOGICAL_OPERATOR");
        else if (c == '+' || c == '-' || c == '*' || c == '%')
            emit(lexeme, "ARITHMETIC_OPERATOR");
        else if (c == '(' || c == ')' || c == '{' || c == '}' ||
                 c == '[' || c == ']' || c == ';' || c == ',')
            emit(lexeme, "DELIMITER");
        else
            emit(lexeme, "UNKNOWN");
    }

    fclose(input);
    return 0;
}
