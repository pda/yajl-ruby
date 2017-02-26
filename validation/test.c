#include "yajl_alloc.h"
#include "yajl_lex.h"
#include "yajl_validator.h"

int main() {
  const char * json = "{" \
                       "\"object\": {\"array\": [1,2]}," \
                      "\"bool\": true," \
                      "\"null\": null," \
                      "\"int\": 1," \
                     "\"double\": 1.0," \
                     "\"string\": \"hello\"," \
                     "\"string_with_escapes\": \"\\u0000\"" \
                     "}";

  printf("Input: %s\n", json);

  yajl_alloc_funcs allocFuncs;
  yajl_set_default_alloc_funcs(&allocFuncs);
  yajl_lexer lexer = yajl_lex_alloc(&allocFuncs, 0, 1);

  struct yajl_validator val = {
    .lexer = lexer,
    .ctx_stack = {yajl_validator_context_root},
    .ctx_stack_depth = 0,
    .state = yajl_validator_state_root
  };

  unsigned int offset = 0;
  const unsigned char * outBuf;
  unsigned int outLen;

  yajl_tok tok;
  do {
    tok = yajl_validator_lex(
      &val,
      (const unsigned char *)json,
      strlen(json),
      &offset,
      &outBuf,
      &outLen
    );
  } while (!yajl_validator_tok_is_terminal(tok));
}

