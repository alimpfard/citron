#include <iostream>
#include <stdio.h>
#include <vector>

#include <windows.h>

#include <processthreadsapi.h>
#include <psapi.h>

int print_modules(std::vector<std::string> matching);
bool print_address = false;

DWORD ErrorNoExit(LPSTR lpszFunction) {
  // Retrieve the system error message for the last-error code

  char *lpMsgBuf;
  DWORD dw = GetLastError();

  FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                     FORMAT_MESSAGE_IGNORE_INSERTS,
                 NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                 (LPSTR)&lpMsgBuf, 0, NULL);

  printf("%s failed with error %d: %s", lpszFunction, dw, lpMsgBuf);

  LocalFree(lpMsgBuf);
  return dw;
}

void ErrorExit(LPSTR lpszFunction) { ExitProcess(ErrorNoExit(lpszFunction)); }

auto main(int argc, char *argv[]) -> int {
  std::vector<std::string> modnames;
  bool ignore_failing = false;
  int modid = 0;
  HMODULE modules[1024];

  for (int i = 1; i < argc; ++i) {
    std::string s{argv[i]};
    if (s == "-h") {
      puts(
          "fakeldconfig [-a] [-x] {-p <filter>} {library}\n"
          "\n"
          "\tLoads all the given libraries and prints the paths to their DLLs\n"
          "OPTIONS\n"
          "\t-a\n\t\tprint addresses\n"
          "\t-x\n\t\tignore loading failures\n"
          "\t-p <filter>\n\t\tfilter results (substring)\n"
          "\t-h\n\t\tshow this help and exit\n"
          "\nAnotherTest (2019)");
      exit(0);
    }
    if (s == "-a") {
      print_address = true;
      continue;
    }
    if (s == "-p") {
      modnames.push_back(std::string{argv[++i]});
      continue;
    }
    if (s == "-x") {
      ignore_failing = true;
      continue;
    }
    HMODULE mod = LoadLibraryA((LPCSTR)s.c_str());
    if (mod != nullptr) {
      modules[modid++] = mod;
    } else {
      std::cerr << "module " << s << " failed to load" << std::endl;
      if (!ignore_failing)
        ErrorExit((LPSTR) "LoadLibrary");
    }
  }

  print_modules(modnames);
  return 0;
}

int print_modules(std::vector<std::string> matching) {
  HMODULE hMods[1024];
  DWORD cbNeeded;
  unsigned int i;
  HANDLE process = GetCurrentProcess();
  if (EnumProcessModules(process, hMods, sizeof(hMods), &cbNeeded)) {
    for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
      char szModName[MAX_PATH];
      if (GetModuleFileNameExA(process, hMods[i], (LPSTR)szModName,
                               sizeof(szModName) / sizeof(TCHAR))) {
        for (std::string m : matching)
          if (std::string{szModName}.find(m) != std::string::npos) {
            if (print_address)
              printf("\t%s (0x%08p)\n", szModName, hMods[i]);
            else
              puts(szModName);
            break;
          }
        if (matching.size() == 0) {
          if (print_address)
            printf("\t%s (0x%08p)\n", szModName, hMods[i]);
          else
            puts(szModName);
        }
      } else {
        ErrorNoExit((LPSTR) "GetModuleFileNameEx");
      }
    }
  } else {
    ErrorExit((LPSTR) "EnumProcessModules");
  }
  return 0;
}
