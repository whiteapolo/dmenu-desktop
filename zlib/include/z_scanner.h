#ifndef Z_SCANNER_H
#define Z_SCANNER_H

#include <z_string.h>

typedef struct {
  Z_String_View source;
  size_t start;
  size_t current;
  size_t line;
  size_t column;
} Z_Scanner;

Z_Scanner z_scanner_new(Z_String_View source);
bool z_scanner_is_at_end(const Z_Scanner *scanner);
void z_scanner_advance(Z_Scanner *scanner, size_t n);
void z_scanner_advance_until(Z_Scanner *scanner, char expected);
void z_scanner_advance_until_string(Z_Scanner *scanner, Z_String_View expected);
char z_scanner_peek(const Z_Scanner *scanner);
char z_scanner_previous(const Z_Scanner *scanner);
bool z_scanner_check(const Z_Scanner *scanner, char expected);
bool z_scanner_match(Z_Scanner *scanner, char expected);
bool z_scanner_check_string(const Z_Scanner *scanner, Z_String_View expected);
bool z_scanner_match_string(Z_Scanner *scanner, Z_String_View expected);
Z_String_View z_scanner_capture(const Z_Scanner *scanner);
void z_scanner_reset_mark(Z_Scanner *scanner);
void z_scanner_skip_cset(Z_Scanner *scanner, Z_String_View cset);
void z_scanner_skip_spaces(Z_Scanner *scanner);

#endif
