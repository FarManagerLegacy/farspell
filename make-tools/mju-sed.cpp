#include <time.h>
#if defined(_MSC_VER) && (_MSC_VER >= 1310) && !defined(__BORLANDC__)
#include <hash_map>
#define stl_hash_map stdext::hash_map
#else
#include <map>
#define stl_hash_map std::map
#endif
#include <cerrno>
#include <string>
#include <iostream>
#include <fstream>
#include <strstream>
#include <windows.h>

using std::string;

string get_filesize(const string &sFilename)
{
  WIN32_FIND_DATA fd;
  HANDLE h;
  h =  FindFirstFile(sFilename.c_str(), &fd);
  if (h != INVALID_HANDLE_VALUE) 
  {
    char str[128];
    _ui64toa(fd.nFileSizeLow | (fd.nFileSizeHigh<<32), str, 10);
    FindClose(h);
    return str;
  }
  else
  {
    std::cerr << sFilename << ": " << strerror(GetLastError()) << "\n";
    exit(1);
  }
  return "";
}

void put_file(const string &sFilename, std::ostreambuf_iterator<char> dest)
{
  std::ifstream src(sFilename.c_str());
  if (!src.good()) {
    std::cerr << sFilename << ": " << strerror(errno) << "\n";
    exit(1);
  }
  std::copy(std::istreambuf_iterator<char>(src),
            std::istreambuf_iterator<char>(),
            dest);
}

class Environment
{
public:
  typedef std::pair<const string, string> string_pair;
  typedef stl_hash_map<const string, string> env_map;
  env_map env;
  time_t ltime;
  struct tm *today;
  void add(const string &var, const string &val)
  {
    env.insert(string_pair(var, val));
  }
  void add_time(const string &var, const char* fmt)
  {
    char _line[32];
    strftime(_line, sizeof(_line), fmt, today); 
    add(var, std::string(_line));
  }
  bool has(const string &var, string &val) const
  {
    env_map::const_iterator found;
    found = env.find(var);
    if (found == env.end()) return false;
    val = found->second;
    return true;
  }
  Environment()
  {
    time( &ltime );
    today = localtime( &ltime );
    add_time("TIMESTAMP", "%Y%m%d");
    add_time("Y", "%Y"); // Year with century, as decimal number
    add_time("m", "%m"); // Month as decimal number (01 - 12)
    add_time("d", "%d"); // Day of month as decimal number (01 - 31)
    add_time("j", "%j"); // Day of year as decimal number (001 - 366)
    add_time("H", "%H"); // Hour in 24-hour format (00 - 23)
  }
};

char escape_end(char begin_quote)
{
  switch (begin_quote)
  {
    case '(': return ')';
    case '[': return ']';
    case '{': return '}';
    case '|': case '!': 
    case '+': case '#': 
    case '-': case '=': 
    case '@': case '~': 
    case '*': case '$': 
    case '%': case '^': 
    case '\\': case '/': 
      return begin_quote;
    default:
      return 0;
  }
}
class TemplateEngine
{
  enum State { 
    passthrough, 
    escape,
    escape_region,
  };
  string sch;
public:
  char cEscape;
  TemplateEngine()
  : sch("0")
  {
    cEscape = '&';
  }

  string Process(const Environment &env, const string &src)
  {
    std::istrstream str_src(src.c_str(), src.length());
    std::ostrstream str_dest;
    Process(env, std::istreambuf_iterator<char>(str_src),
                 std::istreambuf_iterator<char>(),
                 std::ostreambuf_iterator<char>(str_dest));
    str_dest << '\0';
    return str_dest.str();
  }

  void Process(
    const Environment &env,
    std::istreambuf_iterator<char> src_begin,
    std::istreambuf_iterator<char> src_end,
    std::ostreambuf_iterator<char> dest
  )
  {
    State state = passthrough;
    string accumulator;
    string value;
    char cEscapeEnd;

    for (std::istreambuf_iterator<char> &src(src_begin); 
         src != src_end; src++)
    {
      char ch = *src;
      switch (state)
      {
        case passthrough:
          if (ch == cEscape) {
            state = escape;
            accumulator.erase();
          }
          else 
            *dest = ch;
          break;
        case escape:
          if (ch == cEscape) {
            *dest = cEscape;
            state = passthrough;
          } else  if (cEscapeEnd = escape_end(ch)) {
            state = escape_region;
            accumulator.erase();
          } else if ((sch[0] = ch) && (env.has(sch, value))) {
              std::copy(value.begin(), value.end(), dest);
              state = passthrough;
          } else {
            std::cerr << "unknown escape `" << ch << "'\n";
            exit(1);
          }
          break;
        case escape_region:
          if (ch == cEscapeEnd)
          {
            string::size_type i = accumulator.find(' ');
            if (i != string::npos)
            {
              const string cmd(accumulator.substr(0, i));
              const string arg(accumulator.substr(i+1));
              if (cmd == "filesize") {
                value = get_filesize(Process(env, arg));
                std::copy(value.begin(), value.end(), dest);
              } else if (cmd == "file")
                put_file(Process(env, arg), dest);
              else {
                std::cerr << "Invalid escape command `"  << cmd << ":" << arg <<"'\n";
                exit(1);
              }
            } else if ((env.has(accumulator, value))) {
              std::copy(value.begin(), value.end(), dest);
            } else if (accumulator.length()>10) {
              std::cerr << "Invalid escape near "  
                << "`" << accumulator << "'" << "\n";
              exit(1);
            }
            state = passthrough;
            accumulator.erase();
          } else
            accumulator += ch;
          break;
      }
    }
    if (state != passthrough) {
      std::cerr << "Unclosed escape or quote\n";
      exit(1);
    }
  }
};

int main(int argc, char *argv[])
{
  Environment env;
  TemplateEngine engine;
  
  if (argc>1)
  { 
    int l = strlen(argv[1]);
    if (l>0) engine.cEscape = argv[1][0];
    std::cerr << "Using escape " << engine.cEscape << "\n";
  }

  engine.Process(env, std::istreambuf_iterator<char>(std::cin),
                      std::istreambuf_iterator<char>(),
                      std::ostreambuf_iterator<char>(std::cout));
  return 0;
}
