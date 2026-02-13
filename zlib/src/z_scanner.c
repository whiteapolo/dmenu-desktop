#include <z_scanner.h>

Z_Scanner z_scanner_new(Z_String_View source)
{
  Z_Scanner scanner = {
    .source = source,
    .start = 0,
    .current = 0,
    .line = 1,
    .column = 1,
  };

  return scanner;
}

bool z_scanner_is_at_end(const Z_Scanner *scanner)
{
  return scanner->current >= scanner->source.length;
}

void z_scanner_advance(Z_Scanner *scanner, size_t n)
{
  for (size_t i = 0; i < n; i++) {
    char current = scanner->source.ptr[scanner->current++];

    if (current == '\n') {
      scanner->line++;
      scanner->column = 1;
    }
  }
}

char z_scanner_peek(const Z_Scanner *scanner)
{
  return scanner->source.ptr[scanner->current];
}

char z_scanner_previous(const Z_Scanner *scanner)
{
  return scanner->source.ptr[scanner->current - 1];
}

bool z_scanner_check(const Z_Scanner *scanner, char expected)
{
  return !z_scanner_is_at_end(scanner) && z_scanner_peek(scanner) == expected;
}

bool z_scanner_match(Z_Scanner *scanner, char expected)
{
  if (z_scanner_check(scanner, expected)) {
    z_scanner_advance(scanner, 1);
    return true;
  }

  return false;
}

bool z_scanner_check_string(const Z_Scanner *scanner, Z_String_View expected)
{
  if (expected.length > scanner->source.length - scanner->current) {
    return false;
  }

  Z_String_View slice = z_sv_substring(scanner->source, scanner->current, scanner->current + expected.length);
  return z_sv_equal(slice, expected);
}

bool z_scanner_match_string(Z_Scanner *scanner, Z_String_View expected)
{
  if (z_scanner_check_string(scanner, expected)) {
    scanner->current += expected.length;
    return true;
  }

  return false;
}

void z_scanner_advance_until(Z_Scanner *scanner, char expected)
{
  while (!z_scanner_is_at_end(scanner) && !z_scanner_check(scanner, expected)) {
    z_scanner_advance(scanner, 1);
  }
}

void z_scanner_advance_until_string(Z_Scanner *scanner, Z_String_View expected)
{
  while (!z_scanner_is_at_end(scanner) && !z_scanner_check_string(scanner, expected)) {
    z_scanner_advance(scanner, expected.length);
  }
}

Z_String_View z_scanner_capture(const Z_Scanner *scanner)
{
  return z_sv_substring(scanner->source, scanner->start, scanner->current);
}

void z_scanner_reset_mark(Z_Scanner *scanner)
{
  scanner->start = scanner->current;
}

void z_scanner_skip_cset(Z_Scanner *scanner, Z_String_View cset)
{
  while (!z_scanner_is_at_end(scanner) && z_sv_contain_char(cset, z_scanner_peek(scanner))) {
    z_scanner_advance(scanner, 1);
  }
}

void z_scanner_skip_spaces(Z_Scanner *scanner)
{
  z_scanner_skip_cset(scanner, z_sv_from_cstr(" \f\n\r\t\v"));
}
