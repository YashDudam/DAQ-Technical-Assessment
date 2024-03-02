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

static uint64_t extract_value(uint64_t payload, int start, int length);
static uint64_t little_endian_to_big_endian(uint64_t little_endian);
static double extract_value(dbc::format dbc, uint64_t payload,
                            std::string signal_name);

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "USAGE: " << argv[0]
              << " <path to dbc file> <path to can dump>\n";
    return 1;
  }
  const auto dbc_filename = argv[1];
  const auto can_filename = argv[2];

  auto dbc = dbc::format(dbc_filename);
  // std::cout << "=========================================\n";
  // print(dbc, "WheelSpeedFR");
  // std::cout << "=========================================\n";
  // print(dbc, "WheelSpeedFL");
  // std::cout << "=========================================\n";

  auto can_file = std::ifstream(can_filename);
  if (!can_file.is_open()) {
    std::cerr << "Couldn't open file: " << can_filename << '\n';
    return 1;
  }

  std::string line;
  while (std::getline(can_file, line)) {
    auto pkt = packet(line);
    if (pkt.can_id != dbc.get_can_id()) {
      continue;
    }
    // std::cout << pkt.payload << std::endl;

    std::cout << pkt.timestamp << ": "
              << "WheelSpeedFR: "
              << extract_value(dbc, pkt.payload, "WheelSpeedFR") << '\n';
    std::cout << pkt.timestamp << ": "
              << "WheelSpeedRR: "
              << extract_value(dbc, pkt.payload, "WheelSpeedRR") << '\n';
  }

  return 0;
}

static double extract_value(dbc::format dbc, uint64_t payload,
                            std::string signal_name) {
  if (!dbc.is_big_endian(signal_name)) {
    payload = little_endian_to_big_endian(payload);
  }

  auto length = dbc.bit_length(signal_name);
  auto start = dbc.start_bit(signal_name);

  uint64_t mask = ((1 << length) - 1);
  mask = mask << start;
  uint64_t result = 0;
  result = payload & mask;
  result = result >> start;

  return result * dbc.scale(signal_name) + dbc.offset(signal_name);
}

static uint64_t little_endian_to_big_endian(uint64_t little_endian) {
  uint64_t big_endian = 0;

  // Iterate through each byte of the little endian number
  for (int i = 0; i < 8; ++i) {
    // Extract the i-th byte from the little endian number
    uint8_t byte = (little_endian >> (i * 8)) & 0xFF;

    // Append the byte to the big endian number in reverse order
    big_endian |= static_cast<uint64_t>(byte)
                  << ((sizeof(uint64_t) - 1 - i) * 8);
  }

  return big_endian;
}
