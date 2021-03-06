/*
Copyright (c) 2017, Vadim Malyshev, lboss75@gmail.com
All rights reserved
*/
#include "stdafx.h"
#include <locale>
#include <codecvt>
#include "encoding.h"

std::wstring vds::utf16::from_utf8(const std::string & original)
{
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
  return convert.from_bytes(original);
}

std::string vds::utf16::to_utf8(const std::wstring & original)
{
  std::string result;
  result.reserve(original.length());

  for (auto ch : original) {
    utf8::add(result, ch);
  }
  return result;
}

size_t vds::utf8::char_size(char first_utf8_symbol)
{
  if (0 == (0b10000000 & (uint8_t)first_utf8_symbol)) {
    return 1;
  }
  else if (0b11000000 == (0b11100000 & (uint8_t)first_utf8_symbol)) {
    return 2;
  }
  else if (0b11100000 == (0b11110000 & (uint8_t)first_utf8_symbol)) {
    return 3;
  }
  else if (0b11110000 == (0b11111000 & (uint8_t)first_utf8_symbol)) {
    return 4;
  }
  else {
    throw std::runtime_error("Invalid UTF8 string");
  }
}


wchar_t vds::utf8::next_char(const char *& utf8string, size_t & len)
{
  if (len == 0) {
    return 0;
  }

  wchar_t result;
  if (0 == (0b10000000 & (uint8_t)*utf8string)) {
    result = (wchar_t)*utf8string++;
    --len;
  }
  else if (0b11000000 == (0b11100000 & (uint8_t)*utf8string)) {
    if (len < 2 || (0b10000000 != (0b11000000 & (uint8_t)utf8string[1]))) {
      throw std::runtime_error("Invalid UTF8 string");
    }

    result = (wchar_t)(0x80 + (((0b00011111 & (uint8_t)utf8string[0]) << 6) | (0b00111111 & (uint8_t)utf8string[1])));
    utf8string += 2;
    len += 2;
  }
  else if (0b11100000 == (0b11110000 & (uint8_t)*utf8string)) {
    if (len < 3
      || (0b10000000 != (0b11000000 & (uint8_t)utf8string[1]))
      || (0b10000000 != (0b11000000 & (uint8_t)utf8string[2]))) {
      throw std::runtime_error("Invalid UTF8 string");
    }

    result = (wchar_t)(0x800
      + (((0b00011111 & (uint8_t)utf8string[0]) << 12)
        | ((0b00111111 & (uint8_t)utf8string[1]) << 6)
        | (0b00111111 & (uint8_t)utf8string[2])
      ));
    utf8string += 3;
    len += 3;
  }
  else if (0b11110000 == (0b11111000 & (uint8_t)*utf8string)) {
    if (len < 4
      || (0b10000000 != (0b11000000 & (uint8_t)utf8string[1]))
      || (0b10000000 != (0b11000000 & (uint8_t)utf8string[2]))
      || (0b10000000 != (0b11000000 & (uint8_t)utf8string[3]))) {
      throw std::runtime_error("Invalid UTF8 string");
    }

    result = (wchar_t)(0x10000
      + (((0b00011111 & (uint8_t)utf8string[0]) << 18)
      | ((0b00111111 & (uint8_t)utf8string[1]) << 12)
      | ((0b00111111 & (uint8_t)utf8string[2]) << 6)
      | (0b00111111 & (uint8_t)utf8string[3])
      ));
    utf8string += 4;
    len += 4;
  }
  else {
    throw std::runtime_error("Invalid UTF8 string");
  }

  return result;
}

void vds::utf8::add(std::string& result, wchar_t ch)
{
  if (ch < 0x80) {
    result += (char)ch;
  }
  else if (ch < 0x800) {
    result += (char)(0b11000000 | (0b00011111 & ((ch - 0x80) >> 6)));
    result += (char)(0b10000000 | (0b00111111 & (ch - 0x80)));
  }
  else if (ch < 0x10000) {
    result += (char)(0b11000000 | (0b00011111 & ((ch - 0x800) >> 12)));
    result += (char)(0b10000000 | (0b00111111 & ((ch - 0x800) >> 6)));
    result += (char)(0b10000000 | (0b00111111 & (ch - 0x800)));
  }
  else if (ch < 0x110000) {
    result += (char)(0b11000000 | (0b00011111 & ((ch - 0x10000) >> 18)));
    result += (char)(0b10000000 | (0b00111111 & ((ch - 0x10000) >> 12)));
    result += (char)(0b10000000 | (0b00111111 & ((ch - 0x10000) >> 6)));
    result += (char)(0b10000000 | (0b00111111 & (ch - 0x10000)));
  }
  else {
    throw std::runtime_error("Invalid UTF16 string");
  }
}


static const char encodeLookup[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char padCharacter = '=';
std::string vds::base64::from_bytes(const void * _data, size_t len)
{
  const uint8_t * data = reinterpret_cast<const uint8_t *>(_data);
  std::string encodedString;
  encodedString.reserve(((len/3) + ((len % 3 > 0) ? 1 : 0)) * 4);
  uint32_t temp;
  
  for(; 2 < len; len -= 3) {
    temp  = (*data++) << 16;
    temp += (*data++) << 8;
    temp += (*data++);
    encodedString.append(1,encodeLookup[(temp & 0x00FC0000) >> 18]);
    encodedString.append(1,encodeLookup[(temp & 0x0003F000) >> 12]);
    encodedString.append(1,encodeLookup[(temp & 0x00000FC0) >> 6 ]);
    encodedString.append(1,encodeLookup[(temp & 0x0000003F)      ]);
  }
  
  switch(len)
  {
  case 1:
    temp  = (*data++) << 16;
    encodedString.append(1,encodeLookup[(temp & 0x00FC0000) >> 18]);
    encodedString.append(1,encodeLookup[(temp & 0x0003F000) >> 12]);
    encodedString.append(2,padCharacter);
    break;
  case 2:
    temp  = (*data++) << 16;
    temp += (*data++) << 8;
    encodedString.append(1,encodeLookup[(temp & 0x00FC0000) >> 18]);
    encodedString.append(1,encodeLookup[(temp & 0x0003F000) >> 12]);
    encodedString.append(1,encodeLookup[(temp & 0x00000FC0) >> 6 ]);
    encodedString.append(1,padCharacter);
    break;
  }
  
  return encodedString;
}

std::string vds::base64::from_bytes(const const_data_buffer & data)
{
  return from_bytes(data.data(), data.size());
}

vds::const_data_buffer vds::base64::to_bytes(const std::string& data)
{
  if (data.length() % 4){
    throw std::runtime_error("Non-Valid base64!");
  }
  
  size_t padding = 0;
  if (0 < data.length() && data[data.length()-1] == padCharacter){
    padding++;
    
    if (1 < data.length() && data[data.length()-2] == padCharacter){
      padding++;
    }
  }
  
  std::vector< uint8_t > result;
  result.reserve(((data.length()/4)*3) - padding);
  
  uint32_t temp=0;
  
  size_t quantumPosition = 0;
  for(auto ch : data) {
    temp <<= 6;
    if (ch >= 0x41 && ch <= 0x5A) {
      temp |= ch - 0x41;
    }
    else if  (ch >= 0x61 && ch <= 0x7A) {
      temp |= ch - 0x47;
    }
    else if  (ch >= 0x30 && ch <= 0x39) {
      temp |= ch + 0x04;
    }
    else if  (ch == 0x2B) {
      temp |= 0x3E; //change to 0x2D for URL alphabet
    }
    else if  (ch == 0x2F) {
      temp |= 0x3F; //change to 0x5F for URL alphabet
    }
    else if  (ch == padCharacter) {
      switch(padding) {
      case 1: //One pad character
        result.push_back((temp >> 16) & 0x000000FF);
        result.push_back((temp >> 8 ) & 0x000000FF);
        return const_data_buffer(result);
      case 2: //Two pad characters
        result.push_back((temp >> 10) & 0x000000FF);
        return const_data_buffer(result);
      default:
        throw std::runtime_error("Invalid Padding in Base 64!");
      }
    }
    else {
      throw std::runtime_error("Non-Valid Character in Base 64!");
    }
    
    if(4 == ++quantumPosition) {
      result.push_back((temp >> 16) & 0x000000FF);
      result.push_back((temp >> 8 ) & 0x000000FF);
      result.push_back((temp      ) & 0x000000FF);
      quantumPosition = 0;
    }
  }
  
  return const_data_buffer(result);
}
