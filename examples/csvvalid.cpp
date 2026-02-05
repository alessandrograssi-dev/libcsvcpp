/*
csvvalid - determine if files are properly formed CSV files and display
           position of first offending byte if not
*/

#include "CsvParser.hpp"
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <memory>
#include <iostream>

using namespace csv;

struct FileDeleter {
  void operator()(FILE* f) const { if (f) std::fclose(f); }
};

int
main (int argc, char *argv[])
{
  std::unique_ptr<FILE, FileDeleter> infile(nullptr);
  int i;
  char buf[1024];
  size_t bytes_read;
  size_t pos;
  size_t retval = 0;
  bool error_occurred = false;

  if (argc < 2) {
    fprintf(stderr, "Usage: csvvalid files\n");
    return EXIT_FAILURE;
  }

  try {
    CsvParser p;
    p.set_options({CsvParser::Option::Strict}); 

    for (i = 1; i < argc; i++) {
      pos = 0;
      infile.reset(fopen(argv[i], "rb"));
      if (!infile) {
        fprintf(stderr, "Failed to open %s: %s, skipping\n", argv[i], strerror(errno));
        continue;
      }
      while ((bytes_read=fread(buf, 1, 1024, infile.get())) > 0) {
        try {
          p.parse(buf, bytes_read, NULL, NULL, NULL);
        } catch (const CsvError &e) {
          if (e.type == CsvError::ErrorType::Eparse) {
            printf("%s: malformed at byte %lu\n", argv[i], (unsigned long)pos + e.bytes_parsed + 1);
          } else {
            printf("Error while processing %s: %s\n", argv[i], e.what());
          }
          error_occurred = true;
        }
        if (error_occurred) break;
        pos += bytes_read;
      }
      if (!error_occurred) {
        printf("%s well-formed\n", argv[i]);
      }
      p.finish(NULL, NULL, NULL);
      error_occurred = false;
    }

    return EXIT_SUCCESS;

  } catch (const std::exception &e) {
    fprintf(stderr, "Exception: %s\n", e.what());
    return EXIT_FAILURE;
  }
}