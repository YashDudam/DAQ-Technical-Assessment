#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

#include "dbc.h"

struct packet {
  std::string timestamp;
  std::string interface;
  uint32_t can_id;
  uint64_t payload;

  packet(std::string line);
  // For debug sake, please remove it yash
  void print() {
    std::cout << "Timestamp: " << timestamp << '\n'
              << "Interface: " << interface << '\n'
              << "can_id: " << can_id << '\n'
              << "payload: " << payload << '\n';
  }
};

packet::packet(std::string line) {
  std::string timestamp, interface, data;
  std::istringstream(line) >> timestamp >> interface >> data;

  this->timestamp = timestamp;
  this->interface = interface;

  auto stream = std::istringstream(data);
  std::string field;

  std::getline(stream, field, '#');
  this->can_id = std::strtoul(field.c_str(), nullptr, 16);

  std::getline(stream, field, '#');
  this->payload = std::strtoull(field.c_str(), nullptr, 16);
}

static void print(dbc::format dbc, std::string signal_name) {
  std::cout << "Name: " << signal_name << '\n'
            << "Start bit: " << dbc.start_bit(signal_name) << '\n'
            << "Length: " << dbc.bit_length(signal_name) << '\n'
            << "Big Endian: " << dbc.is_big_endian(signal_name) << '\n'
            << "Signed: " << dbc.is_signed(signal_name) << '\n'
            << "Scale: " << dbc.scale(signal_name) << '\n'
            << "Offset: " << dbc.offset(signal_name) << '\n'
            << "Min: " << dbc.min(signal_name) << '\n'
            << "Max: " << dbc.max(signal_name) << '\n'
            << "Unit: " << dbc.unit(signal_name) << '\n';
}

static double extract_value(dbc::format dbc, uint64_t payload);

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "USAGE: " << argv[0]
              << " <path to dbc file> <path to can dump>\n";
    return 1;
  }
  const auto dbc_filename = argv[1];
  const auto can_filename = argv[2];

  auto dbc = dbc::format(dbc_filename);
  std::cout << "=========================================\n";
  print(dbc, "WheelSpeedFR");
  std::cout << "=========================================\n";
  print(dbc, "WheelSpeedFL");
  std::cout << "=========================================\n";

  auto can_file = std::ifstream(can_filename);
  if (!can_file.is_open()) {
    std::cerr << "Couldn't open file: " << can_filename << '\n';
    return 1;
  }

  std::string line;
  while (std::getline(can_file, line)) {
    auto pkt = packet(line);
    // if (pkt.can_id != dbc.get_can_id()) {
    //   continue;
    // }

    extract_value(dbc, pkt.payload);
  }

  return 0;
}

static double extract_value(dbc::format dbc, uint64_t payload) { return 0.0; }
