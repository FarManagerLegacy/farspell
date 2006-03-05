#include <time.h>
#if defined(__CYGWIN__) || defined(__MINGW32__)  
#include <map>
#define hash_map map
#else
#include <hash_map>
#endif
#include <cerrno>
#include <string>
#include <iostream>
#include <fstream>
#include <windows.h>

using std::string;

unsigned long long get_filesize(string &sFilename)
{
  WIN32_FIND_DATA fd;
  HANDLE h;
  h =  FindFirstFile(sFilename.c_str(), &fd);
  if (h != INVALID_HANDLE_VALUE) 
  {
    FindClose(h);
    return fd.nFileSizeLow | (fd.nFileSizeHigh<<32);
  }
  else
  {
    std::cerr << sFilename << ": " << strerror(GetLastError()) << "\n";
    exit(1);
  }
  return 0;
}

void put_file(string &sFilename, std::ostream &dest)
{
  std::ifstream src(sFilename.c_str());
  if (!src.good()) {
    std::cerr << sFilename << ": " << strerror(errno) << "\n";
    exit(1);
  }
  std::copy(std::istreambuf_iterator<char>(src),
            std::istreambuf_iterator<char>(),
            std::ostreambuf_iterator<char>(dest));
}

int main(int argc, char *argv[])
{
  enum { 
    passthru, 
    escape,
    brackets,
  } state = passthru;
  char cEscape = '&';
  char cQuote = '"';
  char cLB = '[';
  char cRB = ']';
  char ch;  string sch("0");
  string accumulator;
  //vector<string> from;
  //vector<string> to;
  typedef std::pair<string,string> string_pair;
  std::hash_map<string, string> env;
  std::hash_map<string, string>::iterator found;
  char _line[1024];
  env.insert(string_pair("aaa", "bbb"));
#define add_map(_from, _to) env.insert(string_pair(_from, _to))
  time_t ltime;
  struct tm *today;
  time( &ltime );
#define add_time(_from, fmt) \
  strftime(_line, sizeof(_line), fmt, today); \
  add_map(_from, std::string(_line));
  today = localtime( &ltime );
  add_time("TIMESTAMP", "%Y%m%d");
  add_time("Y", "%Y"); // Year with century, as decimal number
  add_time("m", "%m"); // Month as decimal number (01 - 12)
  add_time("d", "%d"); // Day of month as decimal number (01 - 31)
  add_time("j", "%j"); // Day of year as decimal number (001 - 366)
  add_time("H", "%H"); // Hour in 24-hour format (00 - 23)

  if (argc>1)
  { 
    int l = strlen(argv[1]);
    if (l>0) cEscape = argv[1][0];
    if (l>2)
    {
      cLB = argv[1][1];
      cRB = argv[1][2];
    }
    std::cerr << "Using escape " << cEscape << cLB << cRB << "\n";
  }

  for (std::istreambuf_iterator<char> in_end, in(std::cin); 
       in != in_end; in++)
  {
    ch = *in;
    switch (state)
    {
      case passthru:
        if (ch == cEscape) {
          state = escape;
          accumulator.erase();
        }
        else 
          std::cout << ch;
        break;
      case escape:
        if (ch == cEscape) {
          std::cout << cEscape;
          state = passthru;
        } else  if (ch == cLB) {
          state = brackets;
          accumulator.erase();
        } else if ((sch[0] = ch) && (found = env.find(sch)) != env.end()) {
            std::cout << found->second;
            state = passthru;
        } else 
        {
          std::cerr << "unknown escape: " << ch << "\n";
          exit(1);
        }
        break;
      case brackets:
        if (ch == cRB)
        {
          string::size_type i = accumulator.find(' ');
          if (i != string::npos)
          {
            string cmd(accumulator.substr(0, i));
            string arg(accumulator.substr(i+1));
            if (cmd == "filesize")
              std::cout << get_filesize(arg);
            else if (cmd == "file")
              put_file(arg, std::cout);
            else 
            {
              std::cerr << "Invalid escape command ["  << cmd <<  " " << arg <<"]\n";
              exit(1);
            }
            state = passthru;
          } else if ((found = env.find(accumulator)) != env.end()) {
            std::cout << found->second;
            state = passthru;
          } else if (accumulator.length()>10)
          {
            std::cerr << "Invalid escape near "  
              << cEscape <<  cLB << accumulator << cRB <<"\n";
            exit(1);
          }
          accumulator.erase();
        } else
          accumulator += ch;
        break;
    }
  }
  if (state != passthru)
  {
    std::cerr << "Unclosed escape or quote\n";
    exit(1);
  }
  return 0;
}
