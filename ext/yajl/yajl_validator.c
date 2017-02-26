#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "yajl_alloc.h"
#include "yajl_lex.h"
#include "yajl_validator.h"

// private headers

static const char * yajl_tok_name(yajl_tok);

static const char * yajl_validator_context_name(yajl_validator_context);

static const char * yajl_validator_state_name(yajl_validator_state);

int yajl_validator_token(struct yajl_validator *, yajl_tok);

yajl_validator_context yajl_validator_ctx(struct yajl_validator *);

void yajl_validator_ctx_push(struct yajl_validator *, yajl_validator_context);

void yajl_validator_ctx_pop(struct yajl_validator *);

void yajl_validator_state_set(struct yajl_validator *, yajl_validator_state);

// functions

yajl_tok yajl_validator_lex(
  struct yajl_validator * val,
  const unsigned char * jsonText,
  unsigned int jsonTextLen,
  unsigned int * offset,
  const unsigned char ** outBuf,
  unsigned int * outLen
) {
  yajl_tok tok = yajl_lex_lex(
    val->lexer,
    jsonText,
    jsonTextLen,
    offset,
    outBuf,
    outLen
  );

  yajl_validator_token(val, tok);

  return tok;
}

static const char *
yajl_tok_name(yajl_tok tok) 
{
    switch (tok) {
        case yajl_tok_bool: return "bool";
        case yajl_tok_colon: return "colon";
        case yajl_tok_comma: return "comma";
        case yajl_tok_comment: return "comment";
        case yajl_tok_eof: return "eof";
        case yajl_tok_error: return "error";
        case yajl_tok_left_brace: return "open_array"; // [
        case yajl_tok_left_bracket: return "open_object"; // {
        case yajl_tok_null: return "null";
        case yajl_tok_integer: return "integer";
        case yajl_tok_double: return "double";
        case yajl_tok_right_brace: return "close_array"; // ]
        case yajl_tok_right_bracket: return "close_object"; // }
        case yajl_tok_string: return "string";
        case yajl_tok_string_with_escapes: return "string_with_escapes";
        default: assert(0);
    }
}

static const char *
yajl_validator_context_name(yajl_validator_context ctx) 
{
    switch (ctx) {
      case yajl_validator_context_root: return "root";
      case yajl_validator_context_array: return "array";
      case yajl_validator_context_object: return "object";
      default: assert(0);
    }
}

static const char *
yajl_validator_state_name(yajl_validator_state state) 
{
    switch (state) {
      case yajl_validator_state_root: return "root";
      case yajl_validator_state_root_done: return "root_done";
      case yajl_validator_state_array: return "array";
      case yajl_validator_state_array_have_value: return "array_have_value";
      case yajl_validator_state_object: return "object";
      case yajl_validator_state_object_have_key: return "object_have_key";
      case yajl_validator_state_object_have_sep: return "object_have_sep";
      case yajl_validator_state_object_have_value: return "object_have_value";
      default: assert(0);
    }
}

int yajl_validator_tok_is_terminal(yajl_tok t) {
  return t == yajl_tok_eof || t == yajl_tok_error;
}

int yajl_validator_token(struct yajl_validator * val, yajl_tok tok) {
  yajl_validator_context ctx = yajl_validator_ctx(val);
  yajl_validator_state state = val->state;
  printf(
    "ctx:%s (%d)  state:%s\n",
    yajl_validator_context_name(ctx),
    val->ctx_stack_depth,
    yajl_validator_state_name(state)
  );
  printf("  token: %s\n", yajl_tok_name(tok));
  yajl_validator_state_name(state);
  yajl_validator_context_name(ctx);

  switch (state) {
    case yajl_validator_state_root:
      switch (tok) {
        case yajl_tok_left_brace: // [
          yajl_validator_ctx_push(val, yajl_validator_context_array);
          yajl_validator_state_set(val, yajl_validator_state_array);
          break;
        case yajl_tok_left_bracket: // {
          yajl_validator_ctx_push(val, yajl_validator_context_object);
          yajl_validator_state_set(val, yajl_validator_state_object);
          break;
        default:
          assert(0);
          break;
      }
      break;
    case yajl_validator_state_root_done:
      switch (tok) {
        case yajl_tok_eof:
          break;
        default:
          assert(0);
          break;
      }
      break;
    case yajl_validator_state_array:
      switch (tok) {
        case yajl_tok_bool:
        case yajl_tok_null:
        case yajl_tok_integer:
        case yajl_tok_double:
        case yajl_tok_right_brace: // ]
        case yajl_tok_right_bracket: // }
        case yajl_tok_string:
        case yajl_tok_string_with_escapes:
          yajl_validator_state_set(val, yajl_validator_state_array_have_value);
          break;
        default:
          assert(0);
          break;
      }
      break;
    case yajl_validator_state_array_have_value:
      switch (tok) {
        case yajl_tok_comma:
          yajl_validator_state_set(val, yajl_validator_state_array);
          break;
        case yajl_tok_right_brace: // ]
          yajl_validator_ctx_pop(val);
          break;
        default:
          assert(0);
          break;
      }
      break;
    case yajl_validator_state_object:
      switch (tok) {
        case yajl_tok_right_bracket: // }
          yajl_validator_ctx_pop(val);
          break;
        case yajl_tok_string:
        case yajl_tok_string_with_escapes:
          yajl_validator_state_set(val, yajl_validator_state_object_have_key);
          break;
        default:
          assert(0);
          break;
      }
      break;
    case yajl_validator_state_object_have_key:
      switch (tok) {
        case yajl_tok_colon:
          yajl_validator_state_set(val, yajl_validator_state_object_have_sep);
          break;
        default:
          assert(0);
          break;
      }
      break;
    case yajl_validator_state_object_have_sep:
      switch (tok) {
        case yajl_tok_bool:
          yajl_validator_state_set(val, yajl_validator_state_object_have_value);
          break;
        case yajl_tok_comment:
          assert(0);
          break;
        case yajl_tok_left_brace: // [
          yajl_validator_ctx_push(val, yajl_validator_context_array);
          yajl_validator_state_set(val, yajl_validator_state_array);
          break;
        case yajl_tok_left_bracket: // {
          yajl_validator_ctx_push(val, yajl_validator_context_object);
          yajl_validator_state_set(val, yajl_validator_state_object);
          break;
        case yajl_tok_null:
        case yajl_tok_integer:
        case yajl_tok_double:
        case yajl_tok_string:
        case yajl_tok_string_with_escapes:
          yajl_validator_state_set(val, yajl_validator_state_object_have_value);
          break;
        default:
          assert(0);
          break;
      }
      break;
    case yajl_validator_state_object_have_value:
      switch (tok) {
        case yajl_tok_comma:
          yajl_validator_state_set(val, yajl_validator_state_object);
          break;
        case yajl_tok_right_bracket: // }
          yajl_validator_ctx_pop(val);
          break;
        default:
          assert(0);
          break;
      }
      break;
    default:
      assert(0);
      break;
  }

  return 1;
}

yajl_validator_context yajl_validator_ctx(struct yajl_validator * val) {
  return val->ctx_stack[val->ctx_stack_depth];
}

void yajl_validator_ctx_push(struct yajl_validator * val, yajl_validator_context ctx) {
  printf("    pushing context: %s\n", yajl_validator_context_name(ctx));
  val->ctx_stack_depth++;
  val->ctx_stack[val->ctx_stack_depth] = ctx;
}

void yajl_validator_ctx_pop(struct yajl_validator * val) {
  assert(val->ctx_stack_depth > 0);
  val->ctx_stack_depth--;
  yajl_validator_context ctx = val->ctx_stack[val->ctx_stack_depth];
  printf("    popped into context: %s\n", yajl_validator_context_name(ctx));
  switch (ctx) {
    case yajl_validator_context_root:
      yajl_validator_state_set(val, yajl_validator_state_root_done);
      break;
    case yajl_validator_context_array:
      yajl_validator_state_set(val, yajl_validator_state_array_have_value);
      break;
    case yajl_validator_context_object:
      yajl_validator_state_set(val, yajl_validator_state_object_have_value);
      break;
    default:
      assert(0);
      break;
  }
}

void yajl_validator_state_set(struct yajl_validator * val, yajl_validator_state state) {
  printf("    setting state: %s\n", yajl_validator_state_name(state));
  val->state = state;
}
