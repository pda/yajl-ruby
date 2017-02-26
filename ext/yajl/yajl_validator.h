#ifndef __YAJL_VALIDATOR_H__
#define __YAJL_VALIDATOR_H__

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "yajl_lex.h"

// types

typedef enum {
  yajl_validator_context_root,
  yajl_validator_context_array,
  yajl_validator_context_object,
} yajl_validator_context;

typedef enum {
  yajl_validator_state_root,
  yajl_validator_state_root_done,
  yajl_validator_state_array,
  yajl_validator_state_array_have_value,
  yajl_validator_state_object,
  yajl_validator_state_object_have_key,
  yajl_validator_state_object_have_sep,
  yajl_validator_state_object_have_value,
} yajl_validator_state;

struct yajl_validator {
  yajl_lexer lexer;
  yajl_validator_context ctx_stack[256]; // TODO: dynamic stack
  uint8_t ctx_stack_depth;               // TODO: dynamic stack
  yajl_validator_state state;
};

// functions

void yajl_validator_init(struct yajl_validator * val, yajl_lexer lexer);

yajl_tok yajl_validator_lex(
  struct yajl_validator * val,
  const unsigned char * jsonText,
  unsigned int jsonTextLen,
  unsigned int * offset,
  const unsigned char ** outBuf,
  unsigned int * outLen
);

int yajl_validator_tok_is_terminal(yajl_tok);

#endif
