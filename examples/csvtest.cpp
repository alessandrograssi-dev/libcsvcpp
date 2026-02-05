/*
csvtest - reads CSV data from stdin and output properly formed equivalent
          useful for testing the library
*/

#include "CsvParser.hpp"
#include <iostream>
#include <stdexcept>
#include <cstdio>
#include <cstdlib>
#include <memory>

static int put_comma;

void cb1 (void *p, size_t len, void *data) {
  if (put_comma) putc(',', stdout);
  csv::CsvParser::fwrite(stdout, p, len);
  put_comma = 1;
}

void cb2 (int c, void *p) {
  put_comma = 0;
  putc('\n', stdout);
}

int main (void) {
  try {
    csv::CsvParser p;
    int i;
    char c;

    while ((i=getc(stdin)) != EOF) {
      c = i;
      p.parse(&c, 1, cb1, cb2, NULL);
    }
    p.finish(cb1, cb2, NULL);
  } catch (const std::exception& e) {
    fprintf(stderr, "Error: %s\n", e.what());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
