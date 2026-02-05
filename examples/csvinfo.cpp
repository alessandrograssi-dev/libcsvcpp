/*
csvinfo - reads CSV data from input file(s) and reports the number
          of fields and rows encountered in each file
*/

#include "CsvParser.hpp"
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <cstring>

using namespace csv;

struct FileDeleter {
  void operator()(FILE* f) const { if (f) std::fclose(f); }
};

struct counts {
  long unsigned fields = 0;
  long unsigned rows = 0;
};

void cb1 (void *s, size_t len, void *data) { 
  auto *d = static_cast<counts*>(data);
  ++d->fields;
}

void cb2 (int c, void *data) { 
  auto *d = static_cast<counts*>(data);
  ++d->rows;
}

static int is_space(unsigned char c) {
  if (c == CsvParser::CommonDelimiter::Space || 
      c == CsvParser::CommonDelimiter::Tab) 
      return 1;
  return 0;
}

static int is_term(unsigned char c) {
  if (c == CsvParser::CommonDelimiter::CR || 
      c == CsvParser::CommonDelimiter::LF) 
      return 1;
  return 0;
}


int
main (int argc, char *argv[])
{
  if (argc < 2) {
      std::fprintf(stderr, "Usage: csvinfo [-s] files\n");
      return EXIT_FAILURE;
  }

  try {
    std::unique_ptr<FILE, FileDeleter> infile(nullptr);
    CsvParser p;
    char buf[1024];

    p.set_space_func(is_space);
    p.set_term_func(is_term);

    while (*(++argv)) {
      if (strcmp(*argv, "-s") == 0) {
        p.set_options({CsvParser::Option::Strict});
        continue;
      }

      infile.reset(fopen(*argv, "rb"));
      if (!infile) {
        std::fprintf(stderr, "Failed to open %s: %s\n", *argv, strerror(errno));
        continue;
      }

      counts c;

      while (std::size_t bytes_read = std::fread(buf, 1, sizeof(buf), infile.get())) {
        p.parse(buf, bytes_read, cb1, cb2, &c);
      }

      p.finish(cb1, cb2, &c);

      if (ferror(infile.get())) {
        std::fprintf(stderr, "Error while reading file %s\n", *argv);
        continue;
      }

      std::printf("%s: %lu fields, %lu rows\n", *argv, c.fields, c.rows);
    }

    return EXIT_SUCCESS;

  } catch (const std::exception& e) {
    std::fprintf(stderr, "Error: %s\n", e.what());
    return EXIT_FAILURE;
  }
}
 
