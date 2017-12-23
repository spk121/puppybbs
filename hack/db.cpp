#include <mysql.h>
#include <stdio.h>

MYSQL mysql;

int main()
{
  mysql_init(&mysql);

  if (!mysql_real_connect(&mysql,
			  nullptr,	// null is localhost
			  "puppy",	// db username
			  "passw0rd",
			  "puppy",	// 'puppy' database
			  3306,	// port 3306 is default for MariaDB
			  nullptr, // the socket or named pipe
			  0ul))    // no flags used
    {
      printf("Failed to connect to database: %s\n",
	     mysql_error(&mysql));
      return 1;
    }

  mysql_query("USE puppy");
  if (mysql_query(con, "SELECT * FROM Cars")) 
  {
      finish_with_error(con);
  }
  
  MYSQL_RES *result = mysql_store_result(con);
  
  if (result == NULL) 
  {
      finish_with_error(con);
  }

  int num_fields = mysql_num_fields(result);

  MYSQL_ROW row;
  
  while ((row = mysql_fetch_row(result))) 
  { 
      for(int i = 0; i < num_fields; i++) 
      { 
          printf("%s ", row[i] ? row[i] : "NULL"); 
      } 
          printf("\n"); 
  }
  
  mysql_free_result(result);
  
  printf("ready\n");
  mysql_close(&mysql);
  return 0;
}
