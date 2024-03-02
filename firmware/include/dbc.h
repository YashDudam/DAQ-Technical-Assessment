#ifndef DBC_FORMAT_H
#define DBC_FORMAT_H

#include <string>
#include <unordered_map>
#include <vector>

namespace dbc {
typedef struct signal *Signal;

class format {
private:
  int can_id;
  std::string name;
  int num_bytes;
  std::unordered_map<std::string, Signal> signals;

public:
  void print();
  format(std::string dbc_filename);
  const int get_can_id();
  const std::string get_frame_name();
  const std::vector<std::string> get_signal_names();
  const int start_bit(std::string signal_name);
  const int bit_length(std::string signal_name);
  const bool is_big_endian(std::string signal_name);
  const bool is_signed(std::string signal_name);
  const double scale(std::string signal_name);
  const int offset(std::string signal_name);
  const int min(std::string signal_name);
  const int max(std::string signal_name);
  const std::string unit(std::string signal_name);
};
} // namespace dbc
#endif
