/*
csvfix - reads (possibly malformed) CSV data from input file
         and writes properly formed CSV to output file
*/

#include "CsvParser.hpp"
#include <cstdlib>
#include <cstring>
#include <exception>
#include <cerrno>
#include <memory>

using namespace csv;

struct FileDeleter {
  void operator()(FILE* f) const { if (f) std::fclose(f); }
};

void cb1 (void *s, size_t i, void *outfile) {
  CsvParser::fwrite(static_cast<FILE *>(outfile), s, i);
  fputc(',', static_cast<FILE *>(outfile));
}

void cb2 (int c, void *outfile) {
  fseek(static_cast<FILE *>(outfile), -1, SEEK_CUR);
  fputc('\n', static_cast<FILE *>(outfile));
}

int main (int argc, char *argv[]) {
  char buf[1024];
  size_t i;
  std::unique_ptr<FILE, FileDeleter> infile(nullptr);
  std::unique_ptr<FILE, FileDeleter> outfile(nullptr);

  if (argc != 3) {
    std::fprintf(stderr, "Usage: csv_fix infile outfile\n");
    return EXIT_FAILURE;
  }

  if (!strcmp(argv[1], argv[2])) {
    std::fprintf(stderr, "Input file and output file must not be the same!\n");
    return EXIT_FAILURE;
  }
  
  try {
    CsvParser p;
    infile.reset(fopen(argv[1], "rb"));
    if (infile == nullptr) {
      std::fprintf(stderr, "Failed to open file %s: %s\n", argv[1], strerror(errno));
      return EXIT_FAILURE;
    }

    outfile.reset(fopen(argv[2], "wb"));
    if (outfile == nullptr) {
      std::fprintf(stderr, "Failed to open file %s: %s\n", argv[2], strerror(errno));
      return EXIT_FAILURE;
    }

    while (std::size_t n = std::fread(buf, 1, sizeof(buf), infile.get())) {
      p.parse(buf, n, cb1, cb2, outfile.get());
    }

    p.finish(cb1, cb2, outfile.get());

    if (ferror(infile.get())) {
      fprintf(stderr, "Error reading from input file");
      remove(argv[2]);
      exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
    
  } catch (const std::exception& e) {
    std::fprintf(stderr, "Error reading from input file: %s\n", e.what());
    remove(argv[2]);
    return EXIT_FAILURE;
  }
}

