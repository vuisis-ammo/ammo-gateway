/* Copyright (c) 2010-2015 Vanderbilt University
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef CONTACTS_QUERY_PARAM_PARSER_H
#define CONTACTS_QUERY_PARAM_PARSER_H

#include <string>

#include "StringParser.h"

class ContactsQueryParamParser : public StringParser
{
public:
  ContactsQueryParamParser (void);

  void parse (const std::string &params);

  std::string contact_owner_;
  std::string uri_;
  std::string first_name_;
  std::string middle_initial_;
  std::string last_name_;
  std::string rank_;
  std::string call_sign_;
  std::string branch_;
  std::string unit_;
  std::string email_;
  std::string phone_;
};

#endif // CONTACTS_QUERY_PARAM_PARSER_H
