#include "CsvParser.hpp"

#include "csv.h"
#include <stdexcept>

// CsvParser enforces non-null invariants internally.
// The underlying libcsv API is therefore always called with valid arguments.

namespace csv {
  struct CsvParser::impl {
    struct csv_parser m_parser{};

    ~impl() {
      csv_free(&m_parser);
    }
  };

  CsvParser::CsvParser()
      : m_pimpl(std::make_unique<impl>()) {
    int result = csv_init(&m_pimpl->m_parser, 0);
    if (result != 0) {
      throw std::runtime_error("CSV Parser Initialization Failed");
    }
  }

  CsvParser::CsvParser(unsigned char delim, unsigned char quote, Options options)
      : m_pimpl(std::make_unique<impl>()) {
    unsigned char c_options = 0;
    for (const auto& opt : options) {
      c_options |= static_cast<unsigned char>(opt);
    }
    int result = csv_init(&m_pimpl->m_parser, static_cast<unsigned char>(c_options));
    if (result != 0) {
      throw std::runtime_error("CSV Parser Initialization Failed");
    }
    csv_set_delim(&m_pimpl->m_parser, delim);
    csv_set_quote(&m_pimpl->m_parser, quote);
  }

  CsvParser::~CsvParser() = default;
  
  unsigned char CsvParser::get_delimiter() const noexcept {
    return csv_get_delim(&m_pimpl->m_parser);
  }

  unsigned char CsvParser::get_quote() const noexcept {
    return csv_get_quote(&m_pimpl->m_parser);
  }

  void CsvParser::set_delimiter(unsigned char c) {
    csv_set_delim(&m_pimpl->m_parser, c);
  }

  void CsvParser::set_quote(unsigned char c) {
    csv_set_quote(&m_pimpl->m_parser, c);
  }

  void CsvParser::set_space_func(int (*f)(unsigned char)) {
    csv_set_space_func(&m_pimpl->m_parser, f);
  }

  void CsvParser::set_term_func(int (*f)(unsigned char)) {
    csv_set_term_func(&m_pimpl->m_parser, f);
  }

  void CsvParser::set_realloc_func(void *(*f)(void *, size_t)) {
    csv_set_realloc_func(&m_pimpl->m_parser, f);
  }

  size_t CsvParser::write( void *dest, size_t dest_size,
                          const void *src, size_t src_size) {
    return csv_write(dest, dest_size, src, src_size);
  }

  int CsvParser::fwrite(FILE *fp, const void *src, size_t src_size) {
    return csv_fwrite(fp, src, src_size);
  }

  size_t CsvParser::write2( void *dest, size_t dest_size,
                           const void *src, size_t src_size,
                           unsigned char quote) {
    return csv_write2(dest, dest_size, src, src_size, quote);
  }

  int CsvParser::fwrite2(FILE *fp, const void *src, size_t src_size,
                         unsigned char quote) {
    return csv_fwrite2(fp, src, src_size, quote);
  }

  size_t CsvParser::parse(const void *s, size_t len,
                          void (*cb1)(void *, size_t, void *),
                          void (*cb2)(int c, void *),
                          void *data) {
    size_t result = csv_parse(&m_pimpl->m_parser, s, len, cb1, cb2, data);
    int c_error = csv_error(&m_pimpl->m_parser);
    if (c_error != 0) {
      const char *errmsg = csv_strerror(c_error);
      throw CsvError(std::string("CSV Parsing Error: ") + errmsg, static_cast<CsvError::ErrorType>(c_error), result);
    }
    return result;
  }


  void CsvParser::finish(  void (*cb1)(void *, size_t, void *),
                          void (*cb2)(int c, void *),
                          void *data) {
    int result = csv_fini(&m_pimpl->m_parser, cb1, cb2, data);
    if (result != 0) {
      throw std::runtime_error(csv_strerror(csv_error(&m_pimpl->m_parser)));
    }
  }

  void CsvParser::set_options(Options options) {
    unsigned char c_options = 0;
    for (Option opt : options) {
      c_options |= static_cast<unsigned char>(opt);
    }
    csv_set_opts(&m_pimpl->m_parser, c_options);
  }

  void CsvParser::set_block_size(size_t size) {
    csv_set_blk_size(&m_pimpl->m_parser, size);
  }

  size_t CsvParser::get_block_size() const noexcept {
    return m_pimpl->m_parser.blk_size;
  }

  size_t CsvParser::get_buffer_size() const noexcept {
    return csv_get_buffer_size(&m_pimpl->m_parser);
  }

} // namespace csv
