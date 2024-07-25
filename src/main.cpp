#include "../include/lexer.h"

#include <memory>
#include <fcntl.h>
#include <time.h>

int main (int argc, char *argv[]) {
    std::string file_path = "./test/parser_test.dp";
    
    struct timespec start, end;

    std::cout << "-------------------------- Reading source file ----------------------------------" << std::endl;

    clock_gettime(CLOCK_REALTIME, &start);

    FILE* input_file = fopen(file_path.c_str(), "r");
    if (!input_file) {
        std::cerr << "Error opening file!" << std::endl;
        return 1;
    }

    char ch;
    std::string content = "";
    while ((ch = fgetc(input_file)) != EOF) {
        putchar(ch);
        content += ch;
    }

    fclose(input_file);
    content += EOF;


    clock_gettime(CLOCK_REALTIME, &end);

    double t_ns = (double)(end.tv_sec - start.tv_sec) * 1.0e9 +
              (double)(end.tv_nsec - start.tv_nsec);

    std::cout << "Elapsed time: " << t_ns << " ns" << std::endl;
    std::cout << "---------------------------------- Lexical Analysis ----------------------------------" << std::endl;

    clock_gettime(CLOCK_REALTIME, &start);

    std::unique_ptr<cLexer> lexer = std::make_unique<cLexer>(content);
    lexer->lex();
    clock_gettime(CLOCK_REALTIME, &end);

    t_ns = (double)(end.tv_sec - start.tv_sec) * 1.0e9 +
              (double)(end.tv_nsec - start.tv_nsec);

    std::cout << "Elapsed time: " << t_ns << " ns" << std::endl;

    lexer->print_tokens();

    return 0;
}
