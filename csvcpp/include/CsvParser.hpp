#ifndef CSV_PARSER_HPP
#define CSV_PARSER_HPP

#include <initializer_list>
#include <memory>
#include <vector>
#include <cstdio>
#include <cstddef>
#include <stdexcept>
#include <string>

namespace csv {

  /**
   * @brief Exception type for CSV parsing and processing errors.
   *
   * Extends std::runtime_error with additional CSV-specific error information,
   * including the type of error encountered and the number of bytes successfully
   * parsed before the error occurred.
   */
  struct CsvError : public std::runtime_error {
    /**
     * @brief Types of errors that can occur during CSV operations.
     */
    enum class ErrorType : unsigned char {
      Eparse   = 1,  ///< Parsing error (malformed CSV)
      Enomem   = 2,  ///< Out of memory
      Etoobig  = 3,  ///< Field or buffer size exceeds limits
      Einvalid = 4   ///< Invalid parameter or configuration
    };
    
    ErrorType type;           ///< The specific type of error that occurred
    size_t bytes_parsed;      ///< Number of bytes successfully parsed before error
    
    /**
     * @brief Constructs a CsvError with message, type, and optional byte count.
     *
     * @param msg Human-readable error description
     * @param t The type of CSV error
     * @param bytes_parsed Number of bytes processed before error (default: 0)
     */
    CsvError(const std::string& msg, ErrorType t, size_t bytes_parsed = 0)
        : std::runtime_error(msg), type(t), bytes_parsed(bytes_parsed) {}
  };


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
    enum class Option : unsigned char {
      Strict      = 1 << 0,  ///< Enable strict CSV parsing
      RepAllNl    = 1 << 1,  ///< Report all newline characters
      StrictFini  = 1 << 2,  ///< Error on unfinished quoted field at EOF
      AppendNull  = 1 << 3,  ///< Append null terminator to parsed fields
      EmptyIsNull = 1 << 4   ///< Treat empty fields as NULL
    };

    /**
     * @brief Common delimiter and control characters.
     */
    enum CommonDelimiter : char {
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
     *
     * @throws std::runtime_error if parser initialization fails
     */
    CsvParser();

    /**
     * @brief Constructs a CsvParser with the specified options.
     * 
     * @param options The configuration options for parsing CSV data, including
     *                delimiter, quote character, and other parsing behaviors.
     */
    explicit CsvParser(Options options);


    /**
     * @brief Constructs a CsvParser with the specified configuration options.
     *
     * @param options The vector of options for parsing CSV data, including
     *                delimiter, quote character, and other parsing behaviors.
     */
    explicit CsvParser(const std::vector<Option>& options);

    /**
     * @brief Constructs a CSV parser with custom delimiter, quote, and options.
     *
     * @param delim Field delimiter character
     * @param quote Quote character
     * @param options Parser configuration options
     *
     * @throws std::runtime_error if parser initialization fails
     */
    explicit CsvParser(unsigned char delim, unsigned char quote, Options options);


    /**
     * @brief Constructs a CSV parser with custom delimiter, quote, and options.
     *
     * @param delim Field delimiter character
     * @param quote Quote character
     * @param options Parser configuration options
     *
     * @throws std::runtime_error if parser initialization fails
     */
    explicit CsvParser(unsigned char delim, unsigned char quote, const std::vector<Option>& options);

    ~CsvParser();

    CsvParser(const CsvParser&) = delete;
    CsvParser& operator=(const CsvParser&) = delete;

    // ------------------------------------------------------------------
    // Configuration
    // ------------------------------------------------------------------

    [[nodiscard]] unsigned char get_delimiter() const noexcept;
    void set_delimiter(unsigned char c);

    [[nodiscard]] unsigned char get_quote() const noexcept;
    void set_quote(unsigned char c);

    void set_space_func(int (*f)(unsigned char));
    void set_term_func(int (*f)(unsigned char));
    void set_realloc_func(void *(*f)(void *, std::size_t));

    /**
     * @brief Sets parser options.
     *
     * Replaces all currently enabled options. Options are combined using bitwise OR.
     *
     * @param options Options to enable (e.g., {Option::Strict, Option::AppendNull})
     */
    void set_options(Options options);

    /**
     * @brief Sets the parser options for CSV parsing configuration.
     * 
     * Replaces all currently enabled options. Options are combined using bitwise OR.
     * 
     * @param options Vector of options to enable (e.g., {Option::Strict, Option::AppendNull})
     */
    void set_options(const std::vector<Option>& options);

    /**
     * @brief Sets internal buffer allocation block size.
     *
     * Controls how much the internal buffer grows when more space is needed.
     *
     * @param size Block size in bytes for buffer growth
     */
    void set_block_size(std::size_t size);

    [[nodiscard]] std::size_t get_block_size() const noexcept;
    [[nodiscard]] std::size_t get_buffer_size() const noexcept;

    // ------------------------------------------------------------------
    // CSV writing
    // ------------------------------------------------------------------

    /**
     * @brief Writes CSV-escaped data into a buffer.
     *
     * Wraps the source data in quotes and escapes any quote characters by doubling them.
     * If @p dest is null, only calculates and returns the required buffer size.
     *
     * @param dest Destination buffer (or null to calculate size)
     * @param dest_size Size of destination buffer
     * @param src Source data to escape and quote
     * @param src_size Size of source data
     *
     * @return Number of bytes written or required (if dest is null)
     */
    static std::size_t write(
      void *dest,
      std::size_t dest_size,
      const void *src,
      std::size_t src_size
    );

    static int fwrite(FILE *fp, const void *src, std::size_t src_size);

    static std::size_t write2(
      void *dest,
      std::size_t dest_size,
      const void *src,
      std::size_t src_size,
      unsigned char quote
    );

    static int fwrite2(
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
     * Must be called after the last parse() invocation. The parser state is
     * reset after this call, allowing the parser to be reused for a new document.
     *
     * @param cb1 Field callback (may be null)
     * @param cb2 Row callback (may be null) - receives -1 for EOF
     * @param data User-provided context pointer
     *
     * @throws std::runtime_error if finalization fails (e.g., unclosed quoted field
     *         in strict mode with StrictFini option)
     */
    void finish(
      void (*cb1)(void *, std::size_t, void *),
      void (*cb2)(int, void *),
      void *data
    );

  private:
    struct impl;
    std::unique_ptr<impl> m_pimpl;

    template <typename It>
    unsigned char convert_options_to_c_flags(It begin, It end) noexcept{
      unsigned char c_options = 0;
      for (It it = begin; it != end; ++it) {
        c_options |= static_cast<unsigned char>(*it);
      }
      return c_options;
    }
  };

} // namespace csv

#endif // CSV_PARSER_HPP
