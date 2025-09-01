#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <random>
#include <thread>

#ifdef __APPLE__
#include <mach/mach_init.h>
#include <mach/task.h>
#else
#include <sys/resource.h>
#endif

long report1()
{
  ::rusage out = {};
  if (::getrusage(RUSAGE_SELF, &out) != 0) {
    return -1;
  }
  return out.ru_maxrss;
}

#ifdef __APPLE__
long usage()
{
  ::task_basic_info t_info = {};
  ::mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;
  if (KERN_SUCCESS != ::task_info(::mach_task_self(), TASK_BASIC_INFO, (task_info_t)&t_info, &t_info_count)) {
    return -1;
  }
  return t_info.resident_size;
}
#else
long usage() { return report1() * 1024; }
#endif

void leak(std::random_device &r, std::size_t volatile &state)
{
  constexpr std::size_t size = 1024 * 1024;
  char *p = static_cast<char *>(::malloc(size));
  auto ps = reinterpret_cast<std::size_t *>(p);

  std::ranlux48 e(r());
  auto o = state;
  // ensure the memory is paged into resident set and read to update state
  for (std::size_t i = 0; i < size / sizeof(std::size_t); ++i) {
    ps[i] = e();
  }
  for (std::size_t i = 0; i < size / sizeof(std::size_t); ++i) {
    o |= ps[i];
  }
  state = o;
}

int main(int argc, char **argv)
{
  constexpr int defsize = 10;
  int size = defsize;
  bool sleep = false;
  if (argc == 2) {
    int d = 0;
    char c = '\0';
    char s = '\0';
    auto const ret = std::sscanf(argv[1], "%i%c%c", &d, &c, &s);
    if (ret < 1 || ret > 2 || d < 1)
    {
      std::cerr << "expected integral number of MB to leak, optionally suffixed with S" << '\n';
      return 13;
    }
    if (c == 's' || c == 'S')
      sleep = true;
    size = d;

    std::cout << "will leak " << size << "MB";
    if (sleep)
      std::cout << " and will sleep 30s when done";
    std::cout << std::endl;
  }

  std::random_device r = {};
  std::size_t volatile s = 0;
  auto const start = usage();
  for (int i = 0; i < size; ++i) {
    std::cerr << report1() << '\t' << usage() << '\n';
    leak(r, s);
  }
  auto const finish = usage();
  std::cout << "total leak in KB: " << (finish - start) / 1024 << '\n';

  if (sleep)
    std::this_thread::sleep_for(std::chrono::seconds(30));
}
