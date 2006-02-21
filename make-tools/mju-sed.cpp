#include <time.h>
#include <vector>
#include <string>
#include <iostream>

using std::string;
using std::vector;

void subst(string &work, const string &from, const string &to)
{
  string::size_type i;
  i = work.find(from);
  while(i != string::npos)
  {
    work = work.replace(i, from.length(), to);
    i = work.find(from);
  }
}

int main()
{
  char _line[4096];
  vector<string> from;
  vector<string> to;

#define add_map(_from,_to) \
  from.insert(from.end(), string(_from)); \
  to.insert(to.end(), string(_to));
  add_map("&&", "\1\1"); // first

  time_t ltime;
  struct tm *today;
  time( &ltime );
#define add_time(fmt,tmpl) \
  strftime(_line, sizeof(_line), fmt, today); \
  add_map(tmpl, _line);
  today = localtime( &ltime );
  add_time("%Y%m%d", "&TIMESTAMP");
  add_time("%Y", "&Y"); // Year with century, as decimal number
  add_time("%m", "&m"); // Month as decimal number (01 - 12)
  add_time("%d", "&d"); // Day of month as decimal number (01 - 31)
  add_time("%j", "&j"); // Day of year as decimal number (001 - 366)
  add_time("%H", "&H"); // Hour in 24-hour format (00 - 23)

  add_map("\1\1", "&"); // last

  while (std::cin)
  {
    std::cin.getline(_line, sizeof(_line));
    if (!std::cin) break;
    string line(_line);
    for (vector<string>::const_iterator i=from.begin(),j=to.begin();
         i!=from.end(); i++, j++)
    {
      subst(line, *i, *j);
    }
    std::cout << line << std::endl;
  }
  return 0;
}