#include <curses.h>
#include <term.h>
#include <form.h>
#include <unistd.h>

SCREEN *scr = nullptr;

int main()
{
  printf("Enter terminal type (or just enter for 'xterm'):\n");
  char term[200];
  scanf("%s",term);

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
   
  // initscr();
  cbreak();
  noecho();
  keypad(stdscr, true);

  FIELD *f1 = new_field(1, 10, 4, 18, 0, 0);
  FIELD *f2 = new_field(1, 10, 6, 18, 0, 0);
  FIELD *f3 = new_field(10, 70, 8, 2, 0, 0);
  FIELD *fields[4] = {f1, f2, f3,NULL};
  set_field_back(f1, A_UNDERLINE);
  field_opts_off(f1, O_AUTOSKIP);
  set_field_back(f2, A_UNDERLINE);
  field_opts_off(f2, O_AUTOSKIP);
  set_field_back(f3, A_UNDERLINE);
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
      else
	{
	  form_driver(frm, ch);
	  ch = getch();
	}
    }
  
  sleep(5);
  
  endwin();
}
