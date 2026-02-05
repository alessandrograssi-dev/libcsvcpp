#ifndef CSV_PARSER_HPP
#define CSV_PARSER_HPP

#include <cstddef>
#include <initializer_list>
#include <memory>
#include <stdexcept>

namespace csv {

  /**
   * @brief High-level C++ wrapper around the libcsv C library.
   *
   * CsvParser provides a safe, exception-based C++ interface while internally
   * delegating parsing and formatting to the underlying C implementation.
   * All errors are reported via exceptions; no error codes are exposed.
   */
  class CsvParser {

  public:
    /**
     * @brief Parsing and configuration options.
     *
     * These options correspond to libcsv flags and can be combined using
     * initializer lists when configuring the parser.
     */
    enum class Option : unsigned {
      Strict      = 1 << 0,  ///< Enable strict CSV parsing
      RepAllNl    = 1 << 1,  ///< Report all newline characters
      StrictFini  = 1 << 2,  ///< Error on unfinished quoted field at EOF
      AppendNull  = 1 << 3,  ///< Append null terminator to parsed fields
      EmptyIsNull = 1 << 4   ///< Treat empty fields as NULL
    };

    /**
     * @brief Common delimiter and control characters.
     */
    enum CommonDelimiter : unsigned char {
      Tab    = 0x09,
      Space  = 0x20,
      CR     = 0x0D,
      LF     = 0x0A,
      Comma  = 0x2C,
      Quote  = 0x22
    };

    using Options = std::initializer_list<Option>;

  public:
    /**
     * @brief Constructs a CSV parser with default settings.
     *
     * Defaults:
     * - Delimiter: comma (,)
     * - Quote character: double quote (")
     * - No options enabled
     */
    CsvParser();

    /**
     * @brief Constructs a CSV parser with custom delimiter, quote, and options.
     *
     * @param delim Field delimiter character
     * @param quote Quote character
     * @param options Parser configuration options
     *
     * @throws std::runtime_error on initialization failure
     */
    CsvParser(unsigned char delim, unsigned char quote, Options options);

    ~CsvParser();

    // ------------------------------------------------------------------
    // Configuration
    // ------------------------------------------------------------------

    unsigned char get_delimiter() const;
    void set_delimiter(unsigned char c);

    unsigned char get_quote() const;
    void set_quote(unsigned char c);

    void set_space_func(int (*f)(unsigned char));
    void set_term_func(int (*f)(unsigned char));
    void set_realloc_func(void *(*f)(void *, std::size_t));

    /**
     * @brief Sets parser options.
     *
     * Replaces all currently enabled options.
     *
     * @param options Options to enable
     *
     * @throws std::runtime_error if option configuration fails
     */
    void set_options(Options options);

    /**
     * @brief Sets internal buffer allocation block size.
     *
     * @throws std::runtime_error if size is invalid
     */
    void set_block_size(std::size_t size);

    std::size_t get_block_size() const;
    std::size_t get_buffer_size() const;

    // ------------------------------------------------------------------
    // CSV writing
    // ------------------------------------------------------------------

    /**
     * @brief Writes CSV-escaped data into a buffer.
     *
     * If @p dest is null, the required buffer size is returned without writing.
     *
     * @throws std::runtime_error on allocation or formatting errors
     */
    std::size_t write(
      void *dest,
      std::size_t dest_size,
      const void *src,
      std::size_t src_size
    );

    int fwrite(FILE *fp, const void *src, std::size_t src_size);

    std::size_t write2(
      void *dest,
      std::size_t dest_size,
      const void *src,
      std::size_t src_size,
      unsigned char quote
    );

    int fwrite2(
      FILE *fp,
      const void *src,
      std::size_t src_size,
      unsigned char quote
    );

    // ------------------------------------------------------------------
    // CSV parsing
    // ------------------------------------------------------------------

    /**
     * @brief Incrementally parses CSV data.
     *
     * Parsing state is preserved between calls, allowing streaming of large files.
     *
     * Callbacks are kept as C-style function pointers to avoid allocations and
     * preserve zero-overhead integration with the legacy parsing core.
     *
     * @param s Buffer containing CSV data
     * @param len Buffer size in bytes
     * @param cb1 Field callback (may be null)
     * @param cb2 Row callback (may be null)
     * @param data User-provided context pointer
     *
     * @return Number of bytes consumed from the buffer
     *
     * @throws std::runtime_error on parsing errors or memory failures
     */
    std::size_t parse(
      const void *s,
      std::size_t len,
      void (*cb1)(void *, std::size_t, void *),
      void (*cb2)(int, void *),
      void *data
    );

    /**
     * @brief Finalizes parsing and flushes any buffered data.
     *
     * Must be called after the last parse() invocation.
     *
     * @throws std::runtime_error if finalization fails
     */
    void finish(
      void (*cb1)(void *, std::size_t, void *),
      void (*cb2)(int, void *),
      void *data
    );

  private:
    struct impl;
    std::unique_ptr<impl> m_impl;
  };

} // namespace csv

#endif // CSV_PARSER_HPP
