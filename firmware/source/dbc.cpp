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

// so many extract_XXX functions... unfortunately they're necessary to keep the
// signal fields read-only. There's a bit of a performance overhead here because
// we're forced to repeat some steps in parsing the string but I believe the
// code safety is worth the trade-off
dbc::signal::signal(const std::string details)
    : name(extract_name(details)), start_bit(extract_start_bit(details)),
      length(extract_length(details)), big_endian(extract_big_endian(details)),
      sign(extract_sign(details)), scale(extract_scale(details)),
      offset(extract_offset(details)), min(extract_min(details)),
      max(extract_max(details)), unit(extract_unit(details)) {}

dbc::format::format(std::string dbc_filename) {
  auto dbc_file = std::ifstream(dbc_filename);
  if (!dbc_file.is_open()) {
    std::cerr << "Unable to open file: " << dbc_filename << '\n';
    exit(1);
  }

  // move through the file until we find our signal definition
  std::string line;
  while (std::getline(dbc_file, line)) {
    if (line.substr(0, 3) == "BO_")
      break;
  }

  auto stream = std::istringstream(line);
  std::string _, can_id, name, num_bytes;
  stream >> _ >> can_id >> name >> num_bytes;
  this->can_id = std::stoi(can_id);
  this->name = name.substr(
      0, name.length() - 1); // trim the trailing ':' from 'ECU_WheelSpeed:'
  this->num_bytes = std::stoi(num_bytes);

  while (std::getline(dbc_file, line)) {
    if (line.empty())
      break;
    auto signal_details = trim_leading_whitespace(line);

    std::string _, signal_name;
    std::istringstream(line) >> _ >> signal_name;
    // this is a bit goofy and where i wish i could get the unique_ptr
    // implementation working. when the pair is inserted into the map, it copies
    // the dbc::signal into the map which is why i need to delete it immediately
    // after insertion. I tried emplace which would insert it into the map in
    // place, but couldn't get it working.
    auto pair = std::pair(signal_name, new dbc::signal(signal_details));
    this->signals.insert(pair);
    delete pair.second;
  }
}

const int dbc::format::get_can_id() { return this->can_id; }

const std::string dbc::format::get_frame_name() { return this->name; }

const std::vector<std::string> dbc::format::get_signal_names() {
  auto names = std::vector<std::string>();
  for (auto &signal : this->signals) {
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
  // bit_details = 32|16@0+
  auto bit_details_stream = std::istringstream(bit_details);
  std::string start_bit;
  std::getline(bit_details_stream, start_bit, '|');
  // start_bit = 32
  return std::stoi(start_bit);
}

static const int extract_length(const std::string details) {
  std::string _, bit_details;

  auto stream = std::istringstream(details);
  stream >> _ >> _ >> _ >> bit_details;
  // bit_details = 32|16@0+
  auto bit_details_stream = std::istringstream(bit_details);
  std::string bit_len_endian_sign;
  std::getline(bit_details_stream, _, '|');
  // bit_len_endian_sign = 16@0+
  std::getline(bit_details_stream, bit_len_endian_sign, '|');

  auto bit_len_endian_sign_stream = std::istringstream(bit_len_endian_sign);
  std::string bit_len;
  std::getline(bit_len_endian_sign_stream, bit_len, '@');
  // bit_len = 16
  return std::stoi(bit_len);
}

static const bool extract_big_endian(const std::string details) {
  std::string _, bit_details;

  // bit_details = 32|16@0+
  auto stream = std::istringstream(details);
  stream >> _ >> _ >> _ >> bit_details;

  // if the second last char of bit_details is '0', then it is big endian
  return bit_details.at(bit_details.length() - 2) == '0';
}

static const bool extract_sign(const std::string details) {
  std::string _, bit_details;

  // bit_details = 32|16@0+
  auto stream = std::istringstream(details);
  stream >> _ >> _ >> _ >> bit_details;

  // if the last char of bit_details is '-', then it is signed
  return bit_details.back() == '-';
}

static const double extract_scale(const std::string details) {
  std::string _, scale_offset;

  // scale_offset = (0.1,0)
  auto stream = std::istringstream(details);
  stream >> _ >> _ >> _ >> _ >> scale_offset;

  // trim the brackets
  scale_offset = scale_offset.substr(1, scale_offset.length() - 1);
  std::string scale;
  auto scale_offset_stream = std::istringstream(scale_offset);
  // get the first value
  std::getline(scale_offset_stream, scale, ',');

  return std::stod(scale);
}

static const int extract_offset(const std::string details) {
  std::string _, scale_offset;

  // scale_offset = (0.1,0)
  auto stream = std::istringstream(details);
  stream >> _ >> _ >> _ >> _ >> scale_offset;

  // trim the brackets
  scale_offset = scale_offset.substr(1, scale_offset.length() - 1);
  std::string offset;
  auto scale_offset_stream = std::istringstream(scale_offset);
  std::getline(scale_offset_stream, _, ',');
  // get the second value
  std::getline(scale_offset_stream, offset, ',');

  return std::stod(offset);
}

static const int extract_min(const std::string details) {
  std::string _, ranges;

  // ranges = [0|0]
  auto stream = std::istringstream(details);
  stream >> _ >> _ >> _ >> _ >> _ >> ranges;

  // trim the square brackets
  ranges = ranges.substr(1, ranges.length() - 1);
  std::string min;
  auto ranges_stream = std::istringstream(ranges);
  // get the first value
  std::getline(ranges_stream, min, '|');

  return std::stoi(min);
}

static const int extract_max(const std::string details) {
  std::string _, ranges;

  // ranges = [0|0]
  auto stream = std::istringstream(details);
  stream >> _ >> _ >> _ >> _ >> _ >> ranges;

  // trim the square brackets
  ranges = ranges.substr(1, ranges.length() - 1);
  std::string max;
  auto ranges_stream = std::istringstream(ranges);
  std::getline(ranges_stream, _, '|');
  // get the second value
  std::getline(ranges_stream, max, '|');

  return std::stoi(max);
}

static const std::string extract_unit(const std::string details) {
  // If we treat the '"' as the delimiter, then the second field will be our
  // unit
  auto unit_stream = std::istringstream(details);
  std::string _, unit;
  std::getline(unit_stream, _, '"');
  std::getline(unit_stream, unit, '"');

  return unit;
}
