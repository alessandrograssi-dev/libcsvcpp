#include "CsvParser.hpp"
#include <cstring>

#define CSV_END 0
#define CSV_COL 1
#define CSV_ROW 2
#define CSV_ERR 3

// Backwards compatibility with legacy test code that expects these macros to be defined
#define CSV_COMMA csv::CsvParser::CommonDelimiter::Comma
#define CSV_QUOTE csv::CsvParser::CommonDelimiter::Quote

#define CSV_STRICT static_cast<unsigned char>(csv::CsvParser::Option::Strict)
#define CSV_STRICT_FINI static_cast<unsigned char>(csv::CsvParser::Option::StrictFini)
#define CSV_EMPTY_IS_NULL static_cast<unsigned char>(csv::CsvParser::Option::EmptyIsNull)
#define CSV_REPALL_NL static_cast<unsigned char>(csv::CsvParser::Option::RepAllNl)

#define DO_TEST(name, options) test_parser("test" #name, options, test ## name ## _data, \
        sizeof(test ## name ## _data) - 1, test ## name ## _results, \
        CSV_COMMA, CSV_QUOTE, NULL, NULL)

#define DO_TEST_CUSTOM(name, options, d, q, s, t) test_parser("custom" #name, options, custom ## name ## _data, \
        sizeof(custom ## name ## _data) - 1, custom ## name ## _results, d, q, s, t)

struct event {
  int event_type; 
  int retval;
  size_t size;  /* Number of bytes in this event or return value from csv_fini */
  char *data;   /* Data for this event */
};

struct event *event_ptr;
int event_idx;
size_t row, col;

void
fail_parser (char * test_name, char * message)
{
  fprintf(stderr, "Parser test %s failed on event %d: %s\n", test_name, event_idx, message);
  exit(EXIT_FAILURE);
}

void
fail_writer (char * test_name, char * message)
{
  fprintf(stderr, "Writer test %s failed: %s\n", test_name, message);
  exit(EXIT_FAILURE);
}

void
cb1 (void *data, size_t len, void *t)
{
  char * test_name = static_cast<char *>(t);

  /* Make sure we were expecting a column */
  if (event_ptr->event_type != CSV_COL)
    fail_parser(test_name, "didn't expect a column");

  /* Check the actual size against the expected size */
  if (event_ptr->size != len)
    fail_parser(test_name, "actual data length doesn't match expected data length");

  /* Check the actual data against the expected data */
  if ((event_ptr->data == NULL || data == NULL)) {
    if (event_ptr->data != data)
      fail_parser(test_name, "actual data doesn't match expected data");
  }
  else if (memcmp(event_ptr->data, data, len) != 0)
    fail_parser(test_name, "actual data doesn't match expected data");

  event_idx++;
  event_ptr++;
  col++;
}

void
cb2 (int c, void *t)
{
  char * test_name = static_cast<char *>(t);

  /* Make sure we were expecting an the end of a row */
  if (event_ptr->event_type != CSV_ROW)
    fail_parser(test_name, "didn't expect end of row");

  /* Check that the row ended with the character we expected */
  if (event_ptr->retval != c)
    fail_parser(test_name, "row ended with unexpected character");

  event_idx++;
  event_ptr++;
  col = 1;
  row++;
}

void
test_parser (char *test_name, unsigned char options, void *input, size_t len, struct event expected[],
             char delimiter, char quote, int (*space_func)(unsigned char), int (*term_func)(unsigned char))
{
  using namespace csv;
  int result = 0;
  size_t retval;
  size_t size;
  
  for (size = 1; size <= len; size++) {
    size_t bytes_processed = 0;
    CsvParser p;
    std::vector<CsvParser::Option> options_vec;
    if (options & CSV_STRICT)        options_vec.push_back(CsvParser::Option::Strict);
    if (options & CSV_REPALL_NL)     options_vec.push_back(CsvParser::Option::RepAllNl);
    if (options & CSV_STRICT_FINI)   options_vec.push_back(CsvParser::Option::StrictFini);
    if (options & CSV_EMPTY_IS_NULL) options_vec.push_back(CsvParser::Option::EmptyIsNull);

    p.set_options(options_vec);
    p.set_delimiter(delimiter);
    p.set_quote(quote);
    p.set_space_func(space_func);
    p.set_term_func(term_func);

    row = col = 1;
    event_ptr = &expected[0];
    event_idx = 1;

    do {
      size_t bytes = size < len - bytes_processed ? size : len - bytes_processed;
      try {
        retval = p.parse(input + bytes_processed, bytes, cb1, cb2, test_name);
      } catch (const CsvError &e) {
        if (event_ptr->event_type != CSV_ERR) {
          fail_parser(test_name, "unexpected parse error occurred");
        } else {
          return;
        }
      }
      bytes_processed += bytes;
    } while (bytes_processed < len);

    try {
      p.finish(cb1, cb2, test_name);
    } catch (const std::runtime_error &e) {
      if (event_ptr->event_type != CSV_ERR) {
        fail_parser(test_name, "unexpected parse error occurred");
      } else {
        return;
      }
    }

    if (event_ptr->event_type != CSV_END)
      fail_parser(test_name, "unexpected end of input");
  }
}

void
test_writer (char * test_name, char *input, size_t input_len, char *expected, size_t expected_len)
{
  size_t actual_len;
  char *temp = static_cast<char *>(malloc(input_len*2 + 2));
  if (!temp) {
    fprintf(stderr, "Failed to allocate memory in test_writer!\n");
    exit(EXIT_FAILURE);
  }

  actual_len = csv::CsvParser::write(temp, input_len*2 + 2, input, input_len);
  if (actual_len != expected_len)
    fail_writer(test_name, "actual length doesn't match expected length");
  if (memcmp(temp, expected, actual_len) != 0)
    fail_writer(test_name, "actual data doesn't match expected data");
}

void
test_writer2 (char * test_name, char *input, size_t input_len, char *expected, size_t expected_len, char quote)
{
  size_t actual_len;
  char *temp = static_cast<char *>(malloc(input_len*2 + 2));
  if (!temp) {
    fprintf(stderr, "Failed to allocate memory in test_writer!\n");
    exit(EXIT_FAILURE);
  }

  actual_len = csv::CsvParser::write2(temp, input_len*2 + 2, input, input_len, quote);
  if (actual_len != expected_len)
    fail_writer(test_name, "actual length doesn't match expected length");
  if (memcmp(temp, expected, actual_len) != 0)
    fail_writer(test_name, "actual data doesn't match expected data");
  free(temp);
}


int main (void) {

  /* Parser Tests */

  /* Parser tests presented here consist of two parts:
     1. the raw data to be parsed as an array of char
     2. the behavior expected from attempting to parse the data
  
     The latter is encapsulated into an array of struct event, each event
     described the type of event that was expected at that point and details
     of the event.  There are four event types:
     1. CSV_END - signifies the successful end of parsing, no events should
                  follow in this test
     2. CSV_COL - signifies that a column or CSV element is expected to be
                  submitted to the appropriate callback function
     3. CSV_ROW - specifies the end of a row has been reached
     4. CSV_ERR - signifies that an error is expected at this point, no
                  further events should follow in this test

     In addition to the event type, an event is also characterized by a size
     and a data member in the event structure.  The meaning of these fields
     depends on the event type.

     The size member for CSV_COL is the size of the expected data column,
     for CSV_ROW is it the size of the terminating character which should
     always be 1.  The size for CSV_END should always be zero, for CSV_ERR
     the size is not used.

     The data member represents the actual data after parsing for a CSV_COL,
     the terminating character for CSV_ROW (the first character of the provided
     char array is the one used), and is not used for CSV_END or CSV_ERR, it
     should be NULL in these cases.
  */


  char test01_data[] = " 1,2 ,  3         ,4,5\x0d\x0a";
  char test02_data[] = ",,,,,\x0a";
  char test03_data[] = "\",\",\",\",\"\"";
  char test04_data[] = "\"I call our world Flatland,\x0a"
                       "not because we call it so,\x0a"
                       "but to make its nature clearer\x0a"
                       "to you, my happy readers,\x0a"
                       "who are privileged to live in Space.\"";
  char test05_data[] = "\"\"\"a,b\"\"\",,\" \"\"\"\" \",\"\"\"\"\" \",\" \"\"\"\"\",\"\"\"\"\"\"";
  char test06_data[] = "\" a, b ,c \", a b  c,";
  char test07_data[] = "\" \"\" \" \" \"\" \"";
  char test07b_data[] = "\" \"\" \" \" \"\" \"";
  char test08_data[] = "\" abc\"                                             "
                       "                                                     "
                       "                                                     "
                       "                                                     "
                       "                                                     "
                       "                                                     "
                       "                                                     "
                       "                                                     "
                       "                                          \", \"123\"";
  char test09_data[] = "";
  char test10_data[] = "a\x0a";
  char test11_data[] = "1,2 ,3,4\x0a";
  char test12_data[] = "\x0a\x0a\x0a\x0a";
  char test12b_data[] = "\x0a\x0a\x0a\x0a";
  char test13_data[] = "\"abc\"";
  char test14_data[] = "1, 2, 3,\x0a\x0d\x0a  \"4\", \x0d,";
  char test15_data[] = "1, 2, 3,\x0a\x0d\x0a  \"4\", \x0d\"\"";
  char test16_data[] = "\"1\",\"2\",\" 3 ";
  char test16b_data[] = "\"1\",\"2\",\" 3 ";
  char test17_data[] = " a\0b\0c ";
  char test18_data[] = "12345678901234567890123456789012";
  char test19_data[] = "  , \"\" ,";

  /* Custom tests */
  char custom01_data[] = "'''a;b''';;' '''' ';''''' ';' ''''';''''''";

  /* |1|2|3|4|5| */
  struct event test01_results[] =
    { {CSV_COL, 0, 1, "1"},
      {CSV_COL, 0, 1, "2"},
      {CSV_COL, 0, 1, "3"},
      {CSV_COL, 0, 1, "4"},
      {CSV_COL, 0, 1, "5"},
      {CSV_ROW, '\x0d', 1, NULL}, {CSV_END, 0, 0, NULL} };
    
  /* ||||||| */
  struct event test02_results[] =
    { {CSV_COL, 0, 0, ""},
      {CSV_COL, 0, 0, ""},
      {CSV_COL, 0, 0, ""},
      {CSV_COL, 0, 0, ""},
      {CSV_COL, 0, 0, ""},
      {CSV_COL, 0, 0, ""},
      {CSV_ROW, '\x0a', 1, NULL}, {CSV_END, 0, 0, NULL} };

  /* |,|,|| */
  struct event test03_results[] = 
    { {CSV_COL, 0, 1, ","},
      {CSV_COL, 0, 1, ","},
      {CSV_COL, 0, 0, ""},
      {CSV_ROW, -1, 1, NULL}, {CSV_END, 0, 0, NULL} };

  struct event test04_results[] = 
    { {CSV_COL, 0, 147, "I call our world Flatland,\x0a"
                     "not because we call it so,\x0a"
                     "but to make its nature clearer\x0a"
                     "to you, my happy readers,\x0a"
                     "who are privileged to live in Space."},
      {CSV_ROW, -1, 1, NULL}, {CSV_END, 0, 0, NULL} };

  /* |"a,b"|| "" |"" | ""|""| */
  struct event test05_results[] = 
    { {CSV_COL, 0, 5, "\"a,b\""},
      {CSV_COL, 0, 0, ""},
      {CSV_COL, 0, 4, " \"\" "},
      {CSV_COL, 0, 3, "\"\" "},
      {CSV_COL, 0, 3, " \"\""},
      {CSV_COL, 0, 2, "\"\""},
      {CSV_ROW, -1, 1, NULL}, {CSV_END, 0, 0, NULL} };

  /* | a, b ,c |a b  c|| */
  struct event test06_results[] = 
    { {CSV_COL, 0, 9, " a, b ,c "},
      {CSV_COL, 0, 6, "a b  c"},
      {CSV_COL, 0, 0, ""},
      {CSV_ROW, -1, 1, NULL}, {CSV_END, 0, 0, NULL} };

  /* | " " " " | */
  struct event test07_results[] = 
    { {CSV_COL, 0, 9, " \" \" \" \" "},
      {CSV_ROW, -1, 1, NULL}, {CSV_END, 0, 0, NULL} };

  /* Will cause error with CSV_STRICT set */
  struct event test07b_results[] = 
    { {CSV_ERR, 0, 0, NULL} };

  struct event test08_results[] = 
    { {CSV_COL, 0, 463,   " abc\"                                               "
                       "                                                     "
                       "                                                     "
                       "                                                     "
                       "                                                     "
                       "                                                     "
                       "                                                     "
                       "                                                     "
                       "                                        "},
      {CSV_COL, 0, 3, "123"},
      {CSV_ROW, -1, 1, NULL}, {CSV_END, 0, 0, NULL} };
                     
  /* empty */
  struct event test09_results[] = 
    { {CSV_END, 0, 0, NULL} };

  /* |a| */
  struct event test10_results[] = 
    { {CSV_COL, 0, 1, "a"},
      {CSV_ROW, '\x0a', 1, NULL}, {CSV_END, 0, 0, NULL} };

  /* |1|2|3|4| */
  struct event test11_results[] = 
    { {CSV_COL, 0, 1, "1"},
      {CSV_COL, 0, 1, "2"},
      {CSV_COL, 0, 1, "3"}, 
      {CSV_COL, 0, 1, "4"},
      {CSV_ROW, '\x0a', 1, NULL}, {CSV_END, 0, 0, NULL} };

  /* empty */
  struct event test12_results[] = 
    { {CSV_END, 0, 0, NULL} };

  /* Test CSV_REPALL_NL */
  struct event test12b_results[] = 
    { {CSV_ROW, '\x0a', 1, NULL},
      {CSV_ROW, '\x0a', 1, NULL},
      {CSV_ROW, '\x0a', 1, NULL},
      {CSV_ROW, '\x0a', 1, NULL}, {CSV_END, 0, 0, NULL} };

  /* |abc| */
  struct event test13_results[] =
    { {CSV_COL, 0, 3, "abc"},
      {CSV_ROW, -1, 1, NULL}, {CSV_END, 0, 0, NULL} };

  /* |1|2|3||
     |4||
     |||       */
  struct event test14_results[] =
    { {CSV_COL, 0, 1, "1"},
      {CSV_COL, 0, 1, "2"},
      {CSV_COL, 0, 1, "3"},
      {CSV_COL, 0, 0, ""},
      {CSV_ROW, '\x0a', 1, NULL},
      {CSV_COL, 0, 1, "4"},
      {CSV_COL, 0, 0, ""},
      {CSV_ROW, '\x0d', 1, NULL},
      {CSV_COL, 0, 0, ""},
      {CSV_COL, 0, 0, ""},
      {CSV_ROW, -1, 0, NULL}, {CSV_END, 0, 0, NULL} }; 
      
  /* |1|2|3||
     |4||
     ||       */
  struct event test15_results[] =
    { {CSV_COL, 0, 1, "1"},
      {CSV_COL, 0, 1, "2"},
      {CSV_COL, 0, 1, "3"},
      {CSV_COL, 0, 0, ""},
      {CSV_ROW, '\x0a', 1, NULL},
      {CSV_COL, 0, 1, "4"},
      {CSV_COL, 0, 0, ""},
      {CSV_ROW, '\x0d', 1, NULL},
      {CSV_COL, 0, 0, ""},
      {CSV_ROW, -1, 0, NULL}, {CSV_END, 0, 0, NULL} };


  /* |1|2| 3 | */
  struct event test16_results[] = 
    { {CSV_COL, 0, 1, "1"},
      {CSV_COL, 0, 1, "2"},
      {CSV_COL, 0, 3, " 3 "},
      {CSV_ROW, -1, 1, NULL}, {CSV_END, 0, 0, NULL} };

  /* Will cause an error with CSV_STRICT_FINI set */
  struct event test16b_results[] = 
    { {CSV_COL, 0, 1, "1"},
      {CSV_COL, 0, 1, "2"},
      {CSV_ERR, 0, 0, NULL} };

  /* |a\0b\0c| */
  struct event test17_results[] = 
    { {CSV_COL, 0, 5, "a\0b\0c"},
      {CSV_ROW, -1, 1, NULL}, {CSV_END, 0, 0, NULL} };

  /* Test CSV_EMPTY_IS_NULL */
  struct event test19_results[] = 
    { {CSV_COL, 0, 0, NULL},
      {CSV_COL, 0, 0, ""},
      {CSV_COL, 0, 0, NULL},
      {CSV_ROW, -1, 1, NULL}, {CSV_END, 0, 0, NULL} };

  /* |'a;b'|| '' |'' | ''|''| */
  struct event custom01_results[] = 
    { {CSV_COL, 0, 5, "'a;b'"},
      {CSV_COL, 0, 0, ""},
      {CSV_COL, 0, 4, " '' "},
      {CSV_COL, 0, 3, "'' "},
      {CSV_COL, 0, 3, " ''"},
      {CSV_COL, 0, 2, "''"},
      {CSV_ROW, -1, 1, NULL}, {CSV_END, 0, 0, NULL} };

  DO_TEST(01, 0);
  DO_TEST(01, CSV_STRICT);
  DO_TEST(01, CSV_STRICT | CSV_EMPTY_IS_NULL);
  DO_TEST(02, 0);
  DO_TEST(02, CSV_STRICT);
  DO_TEST(03, 0);
  DO_TEST(03, CSV_STRICT);
  DO_TEST(04, 0);
  DO_TEST(04, CSV_STRICT);
  DO_TEST(05, 0);
  DO_TEST(05, CSV_STRICT);
  DO_TEST(05, CSV_STRICT | CSV_STRICT_FINI);
  DO_TEST(06, 0);
  DO_TEST(06, CSV_STRICT);
  DO_TEST(07, 0);
  DO_TEST(07b, CSV_STRICT);
  DO_TEST(08, 0);
  DO_TEST(09, 0);
  DO_TEST(09, CSV_EMPTY_IS_NULL);
  DO_TEST(10, 0);
  DO_TEST(11, 0);
  DO_TEST(11, CSV_EMPTY_IS_NULL);
  DO_TEST(12, 0);
  DO_TEST(12, CSV_EMPTY_IS_NULL);
  DO_TEST(12b, CSV_REPALL_NL);
  DO_TEST(12b, CSV_REPALL_NL | CSV_EMPTY_IS_NULL);
  DO_TEST(13, 0);
  DO_TEST(14, 0);
  DO_TEST(14, CSV_STRICT);
  DO_TEST(15, 0);
  DO_TEST(15, CSV_STRICT);
  DO_TEST(16, 0);
  DO_TEST(16, CSV_STRICT);
  DO_TEST(16b, CSV_STRICT | CSV_STRICT_FINI);
  DO_TEST(16, 0);
  DO_TEST(16, CSV_STRICT);
  DO_TEST(17, 0);
  DO_TEST(17, CSV_STRICT);
  DO_TEST(17, CSV_STRICT | CSV_EMPTY_IS_NULL);
  DO_TEST(19, CSV_EMPTY_IS_NULL);

  DO_TEST_CUSTOM(01, 0, ';', '\'', NULL, NULL);

  /* Writer Tests */

  /* The writer tests are simpler, the test_writer function is used to
     test the csv_write function and takes five arguments: the name of
     the test, the data to convert, the length of that data, the expected
     resulting CSV data, and the expected length of the result.
  */

  test_writer("1", "abc", 3, "\"abc\"", 5);
  test_writer("2", "\"\"\"\"\"\"\"\"", 8, "\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"", 18);

  test_writer2("1", "abc", 3, "'abc'", 5, '\'');
  test_writer2("2", "''''''''", 8, "''''''''''''''''''", 18, '\'');

  puts("All tests passed");
  return 0;
}
