#include "yajl_alloc.h"
#include "yajl_lex.h"
#include "yajl_validator.h"

void test_valid();
void test_invalid();
void test_chunked();

int main() {
  test_valid();
  test_invalid();
  test_chunked();
}

void test_valid() {
  const char * json = "{" \
    "\"object\": {\"array\": [1,2.0,true,null,\"hello\",\"\\u0000\"]}," \
    "\"empty \\u0000 array\": []," \
    "\"bool\": true," \
    "\"null\": null," \
    "\"int\": 1," \
    "\"double\": 1.0," \
    "\"string\": \"hello\"," \
    "\"string_with_escapes\": \"\\u0000\"" \
    "}";

  printf("\nInput: %s\n", json);

  yajl_alloc_funcs allocFuncs;
  yajl_set_default_alloc_funcs(&allocFuncs);
  yajl_lexer lexer = yajl_lex_alloc(&allocFuncs, 0, 1);

  struct yajl_validator val;
  yajl_validator_init(&val, lexer);

  const unsigned char * buffer = (const unsigned char *)json;
  int len = strlen(json);
  unsigned int offset = 0;
  const unsigned char * outBuf;
  unsigned int outLen;

  yajl_tok tok;
  do {
    tok = yajl_validator_lex(&val, buffer, len, &offset, &outBuf, &outLen);
  } while (!yajl_validator_tok_is_terminal(tok));
}

void test_invalid() {
  const char * json = "[,]";
  printf("\nInvalid Input: %s\n", json);

  yajl_alloc_funcs allocFuncs;
  yajl_set_default_alloc_funcs(&allocFuncs);
  yajl_lexer lexer = yajl_lex_alloc(&allocFuncs, 0, 1);

  struct yajl_validator val;
  yajl_validator_init(&val, lexer);

  const unsigned char * buffer = (const unsigned char *)json;
  int len = strlen(json);
  unsigned int offset = 0;
  const unsigned char * outBuf;
  unsigned int outLen = 0;

  yajl_tok tok;
  do {
    tok = yajl_validator_lex(&val, buffer, len, &offset, &outBuf, &outLen);
    if (val.error) {
      puts("  ERROR");
      val.error = 0;
    }
  } while (!yajl_validator_tok_is_terminal(tok));
}

void test_chunked() {
  const char * json = "[12345,6]";
  int chunk_size = 5; // less than full input
  printf("\nChunked Input: %s (chunk size: %d)\n", json, chunk_size);

  yajl_alloc_funcs allocFuncs;
  yajl_set_default_alloc_funcs(&allocFuncs);
  yajl_lexer lexer = yajl_lex_alloc(&allocFuncs, 0, 1);

  struct yajl_validator val;
  yajl_validator_init(&val, lexer);

  const unsigned char * buffer = (const unsigned char *)json;
  int len = chunk_size;
  unsigned int offset = 0;
  const unsigned char * outBuf;
  unsigned int outLen = 0;

  yajl_tok tok;

  // first chunk_size bytes
  do {
    tok = yajl_validator_lex(&val, buffer, len, &offset, &outBuf, &outLen);
  } while (!yajl_validator_tok_is_terminal(tok));

  // remainder
  buffer += chunk_size;
  offset = 0;
  len = strlen((const char *)buffer);
  do {
    tok = yajl_validator_lex(&val, buffer, len, &offset, &outBuf, &outLen);
  } while (!yajl_validator_tok_is_terminal(tok));
}
