#include <curses.h>
#include <ctype.h>
#include <term.h>
#include <form.h>
#include <unistd.h>
#include <string.h>

SCREEN *scr = nullptr;

int main()
{
  printf("Enter terminal type (or just enter for 'xterm'):\n");
  char term[200];
  fgets(term, 199, stdin);
  for (int i = 0; i < strlen(term); i ++)
    {
      if (!isgraph(term[i]))
	{
	  term[i] = '\0';
	  break;
	}
    }
  if (strlen(term) == 0)
    strcpy(term, "xterm");
  int erret;
  int ret = setupterm(term, 0, &erret);
  if (ret == ERR && erret == 1)
    {
      printf("your terminal '%s' is too dumb to work\n", term);
      return 1;
    }
  else if (ret == ERR && erret == 0)
    {
      printf("your terminal '%s' is unknown to me\n", term);
      return 1;
    }
   
  scr = newterm(term, stdin, stdout);
  start_color();
  cbreak();
  noecho();
  keypad(stdscr, true);
  move(0,0);
  addstr("terminal: ");
  addstr(longname());
  char buf[200];
  if (has_colors())
    sprintf(buf, "color terminal with %d colors and %d color pairs", COLORS, COLOR_PAIRS);
  else
    sprintf(buf, "monochrome terminal");
  move(1, 0);
  addstr(buf);
  refresh();
  getch();

  FIELD *f1 = new_field(1, 10, 4, 18, 0, 0);
  FIELD *f2 = new_field(1, 10, 6, 18, 0, 0);
  FIELD *f3 = new_field(10, 70, 8, 2, 0, 0);
  FIELD *fields[4] = {f1, f2, f3,NULL};
  init_pair(1, COLOR_BLACK, COLOR_CYAN);
  set_field_back(f1, COLOR_PAIR(1) | A_UNDERLINE);
  field_opts_off(f1, O_AUTOSKIP);
  set_field_back(f2, COLOR_PAIR(1) | A_UNDERLINE);
  field_opts_off(f2, O_AUTOSKIP);
  set_field_fore(f2, A_BLINK);
  set_field_back(f3, COLOR_PAIR(1));
  set_field_fore(f3, A_ITALIC);
  field_opts_off(f3, O_AUTOSKIP);

  FORM *frm = new_form(fields);
  post_form (frm);
  refresh();

  mvaddstr(4, 10, "Value 1:");
  mvaddstr(6, 10, "Value 2:");
  refresh();

  int ch = getch();
  while (ch != KEY_F(1))
    {
      if (ch == KEY_DOWN)
	{
	  form_driver(frm, REQ_NEXT_FIELD);
	  form_driver(frm, REQ_END_LINE);
	  ch = getch();
	}
      else if (ch == KEY_UP)
	{
	  form_driver(frm, REQ_PREV_FIELD);
	  form_driver(frm, REQ_END_LINE);
	  ch = getch();
	}
      else if (ch == KEY_LEFT)
	{
	  form_driver(frm, REQ_PREV_CHAR);
	  ch = getch();
	}
      else if (ch == KEY_RIGHT)
	{
	  form_driver(frm, REQ_NEXT_CHAR);
	  ch = getch();
	}
      else if (ch == KEY_HOME)
	{
	  form_driver(frm, REQ_BEG_LINE);
	  ch = getch();
	}
      else if (ch == KEY_BACKSPACE)
	{
	  form_driver(frm, REQ_DEL_PREV);
	  ch = getch();
	}
      else if (ch == KEY_DC)
	{
	  form_driver(frm, REQ_DEL_CHAR);
	  ch = getch();
	}
      else
	{
	  form_driver(frm, ch);
	  ch = getch();
	}
    }
  
  sleep(5);
  
  endwin();
}
