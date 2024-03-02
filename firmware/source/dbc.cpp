#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "dbc.h"

static std::string trim_leading_whitespace(const std::string line);
static const std::string extract_name(const std::string details);
static const int extract_start_bit(const std::string details);
static const int extract_length(const std::string details);
static const bool extract_big_endian(const std::string details);
static const bool extract_sign(const std::string details);
static const double extract_scale(const std::string details);
static const int extract_offset(const std::string details);
static const int extract_min(const std::string details);
static const int extract_max(const std::string details);
static const std::string extract_unit(const std::string details);

struct dbc::signal {
  const std::string name;
  const int start_bit;
  const int length;
  const bool big_endian;
  const bool sign;
  const double scale;
  const int offset;
  const int min;
  const int max;
  const std::string unit;

  signal(std::string details);
  // remove after debugging yash.
  void print();
};

dbc::signal::signal(const std::string details)
    : name(extract_name(details)), start_bit(extract_start_bit(details)),
      length(extract_length(details)), big_endian(extract_big_endian(details)),
      sign(extract_sign(details)), scale(extract_scale(details)),
      offset(extract_offset(details)), min(extract_min(details)),
      max(extract_max(details)), unit(extract_unit(details)) {}

void dbc::signal::print() {
  std::cout << "Name: " << this->name << '\n'
            << "Start bit: " << this->start_bit << '\n'
            << "Length: " << this->length << '\n'
            << "Big Endian: " << this->big_endian << '\n'
            << "Signed: " << this->sign << '\n'
            << "Scale: " << this->scale << '\n'
            << "Offset: " << this->offset << '\n'
            << "Min: " << this->min << '\n'
            << "Max: " << this->max << '\n'
            << "Unit: " << this->unit << '\n';
}

void dbc::format::print() {
  std::cout << "CAN id: " << this->can_id << '\n'
            << "name: " << this->name << '\n'
            << "Bytes: " << this->num_bytes << '\n';

  std::cout << "=========================================\n";
  for (auto &pair : this->signals) {
    pair.second->print();
    std::cout << "=========================================\n";
  }
}

static std::string trim_leading_whitespace(const std::string line) {
  return line.substr(line.find_first_not_of(' '));
}

static const std::string extract_name(const std::string details) {
  std::string _, name;

  auto stream = std::istringstream(details);
  stream >> _ >> name;

  return name;
}

static const int extract_start_bit(const std::string details) {
  std::string _, bit_details;

  auto stream = std::istringstream(details);
  stream >> _ >> _ >> _ >> bit_details;

  auto bit_details_stream = std::istringstream(bit_details);
  std::string start_bit;
  std::getline(bit_details_stream, start_bit, '|');

  return std::stoi(start_bit);
}

static const int extract_length(const std::string details) {
  std::string _, bit_details;

  auto stream = std::istringstream(details);
  stream >> _ >> _ >> _ >> bit_details;

  auto bit_details_stream = std::istringstream(bit_details);
  std::string bit_len_endian_sign;
  std::getline(bit_details_stream, _, '|');
  std::getline(bit_details_stream, bit_len_endian_sign, '|');

  auto bit_len_endian_sign_stream = std::istringstream(bit_len_endian_sign);
  std::string bit_len;
  std::getline(bit_len_endian_sign_stream, bit_len, '@');

  return std::stoi(bit_len);
}

static const bool extract_big_endian(const std::string details) {
  std::string _, bit_details;

  auto stream = std::istringstream(details);
  stream >> _ >> _ >> _ >> bit_details;

  return bit_details.at(bit_details.length() - 2) == '0';
}

static const bool extract_sign(const std::string details) {
  std::string _, bit_details;

  auto stream = std::istringstream(details);
  stream >> _ >> _ >> _ >> bit_details;

  return bit_details.back() == '-';
}

static const double extract_scale(const std::string details) {
  std::string _, scale_offset;

  auto stream = std::istringstream(details);
  stream >> _ >> _ >> _ >> _ >> scale_offset;

  scale_offset = scale_offset.substr(1, scale_offset.length() - 1);
  std::string scale;
  auto scale_offset_stream = std::istringstream(scale_offset);
  std::getline(scale_offset_stream, scale, ',');

  return std::stod(scale);
}

static const int extract_offset(const std::string details) {
  std::string _, scale_offset;

  auto stream = std::istringstream(details);
  stream >> _ >> _ >> _ >> _ >> scale_offset;

  scale_offset = scale_offset.substr(1, scale_offset.length() - 1);
  std::string offset;
  auto scale_offset_stream = std::istringstream(scale_offset);
  std::getline(scale_offset_stream, _, ',');
  std::getline(scale_offset_stream, offset, ',');

  return std::stod(offset);
}

static const int extract_min(const std::string details) {
  std::string _, ranges;

  auto stream = std::istringstream(details);
  stream >> _ >> _ >> _ >> _ >> _ >> ranges;

  ranges = ranges.substr(1, ranges.length() - 1);
  std::string min;
  auto ranges_stream = std::istringstream(ranges);
  std::getline(ranges_stream, min, '|');

  return std::stoi(min);
}

static const int extract_max(const std::string details) {
  std::string _, ranges;

  auto stream = std::istringstream(details);
  stream >> _ >> _ >> _ >> _ >> _ >> ranges;

  ranges = ranges.substr(1, ranges.length() - 1);
  std::string max;
  auto ranges_stream = std::istringstream(ranges);
  std::getline(ranges_stream, _, '|');
  std::getline(ranges_stream, max, '|');

  return std::stoi(max);
}

static const std::string extract_unit(const std::string details) {
  auto unit_stream = std::istringstream(details);
  std::string _, unit;
  std::getline(unit_stream, _, '"');
  std::getline(unit_stream, unit, '"');

  return unit;
}

dbc::format::format(std::string dbc_filename) {
  auto dbc_file = std::ifstream(dbc_filename);
  if (!dbc_file.is_open()) {
    std::cerr << "Unable to open file: " << dbc_filename << '\n';
    exit(1);
  }

  std::string line;
  while (std::getline(dbc_file, line)) {
    if (line.substr(0, 3) == "BO_")
      break;
  }

  auto stream = std::istringstream(line);
  std::string _, can_id, name, num_bytes;
  stream >> _ >> can_id >> name >> num_bytes;
  this->can_id = std::stoi(can_id);
  this->name = name.substr(0, name.length() - 1);
  this->num_bytes = std::stoi(num_bytes);

  while (std::getline(dbc_file, line)) {
    if (line.empty())
      break;
    auto signal_details = trim_leading_whitespace(line);

    std::string _, signal_name;
    std::istringstream(line) >> _ >> name;
    auto pair = std::pair(name, new dbc::signal(signal_details));
    this->signals.insert(pair);
  }
}

const int dbc::format::get_can_id() { return this->can_id; }

const std::string dbc::format::get_frame_name() { return this->name; }

const std::vector<std::string> dbc::format::get_signal_names() {
  auto names = std::vector<std::string>();
  for (auto signal : this->signals) {
    names.push_back(signal.first);
  }
  return names;
}

const int dbc::format::start_bit(std::string signal_name) {
  return this->signals.at(signal_name)->start_bit;
}

const int dbc::format::bit_length(std::string signal_name) {
  return this->signals.at(signal_name)->length;
}

const bool dbc::format::is_big_endian(std::string signal_name) {
  return this->signals.at(signal_name)->big_endian;
}

const bool dbc::format::is_signed(std::string signal_name) {
  return this->signals.at(signal_name)->sign;
}

const double dbc::format::scale(std::string signal_name) {
  return this->signals.at(signal_name)->scale;
}

const int dbc::format::offset(std::string signal_name) {
  return this->signals.at(signal_name)->offset;
}

const int dbc::format::min(std::string signal_name) {
  return this->signals.at(signal_name)->min;
}

const int dbc::format::max(std::string signal_name) {
  return this->signals.at(signal_name)->max;
}

const std::string dbc::format::unit(std::string signal_name) {
  return this->signals.at(signal_name)->unit;
}
