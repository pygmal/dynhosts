#include <Windows.h>
#include <WinDNS.h>
#include <fstream>
#include <string>

std::string IpStrFromU32(uint32_t ip) {
  uint8_t a = ip >> 24;
  uint8_t b = ip >> 16;
  uint8_t c = ip >> 8;
  uint8_t d = ip;
  return std::to_string(d) + "." + std::to_string(c) + "." + std::to_string(b) +
         "." + std::to_string(a);
}

int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
  while (true) {
    CopyFileA("stahosts.txt", "newhosts.txt", FALSE);
    int wait_secs = 60;
    {
      std::ofstream newhosts("newhosts.txt", std::ios::app);
      std::ifstream dynhosts("dynhosts.txt");
      std::string to_resolve, dynhost;
      while (dynhosts >> to_resolve >> dynhost) {
        DNS_RECORD *record_list = nullptr;
        DnsQuery_A(to_resolve.c_str(), DNS_TYPE_A, DNS_QUERY_STANDARD, nullptr,
                   &record_list, nullptr);
        if (!record_list) {
          OutputDebugStringA((to_resolve + ": unresolved\n").c_str());
          continue;
        }
        for (DNS_RECORD *record = record_list; record; record = record->pNext) {
          if (record->wType != DNS_TYPE_A) {
            continue;
          }
          std::string ipstr = IpStrFromU32(record->Data.A.IpAddress);
          OutputDebugStringA((dynhost + ": " + ipstr + "\n").c_str());
          newhosts << ipstr << "\t" << dynhost << "\n";
          if (wait_secs > static_cast<int>(record->dwTtl)) {
            wait_secs = record->dwTtl;
          }
        }
        DnsFree(record_list, DnsFreeRecordList);
      }
    }
    CopyFileA("newhosts.txt", "C:\\Windows\\System32\\drivers\\etc\\hosts",
              FALSE);
    // TODO(xjia): Do we need to `ipconfig /flushdns`?
    Sleep(wait_secs * 1000);
  }
  return 0;
}
