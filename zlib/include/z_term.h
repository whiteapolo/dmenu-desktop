#ifndef Z_TERM_H
#define Z_TERM_H

void z_clear_screen(void);
void z_clear_line(void);

void z_enable_line_wrap(void);
void z_disable_line_wrap(void);

void z_enter_alternative_screen(void);
void z_exit_alternative_screen(void);

void z_hide_cursor(void);
void z_show_cursor(void);

void z_set_cursor_position(int x, int y);
void z_set_cursor_horizontal_position(int x);

void z_move_cursor_up(int n);
void z_move_cursor_down(int n);
void z_move_cursor_right(int n);
void z_move_cursor_left(int n);

#endif
