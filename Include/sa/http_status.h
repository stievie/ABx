/**
 * Copyright 2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

namespace sa {
namespace http {

#define ENUMERATE_HTTP_STATUS_CODES      \
    ENUM_HTTP_STATUS_CODE(100, "Continue") \
    ENUM_HTTP_STATUS_CODE(101, "Switching Protocol") \
    ENUM_HTTP_STATUS_CODE(102, "Processing") \
    ENUM_HTTP_STATUS_CODE(103, "Early Hints") \
    ENUM_HTTP_STATUS_CODE(200, "OK") \
    ENUM_HTTP_STATUS_CODE(201, "Created") \
    ENUM_HTTP_STATUS_CODE(202, "Accepted") \
    ENUM_HTTP_STATUS_CODE(203, "Non-Authoritative Information") \
    ENUM_HTTP_STATUS_CODE(204, "No Content") \
    ENUM_HTTP_STATUS_CODE(205, "Reset Content") \
    ENUM_HTTP_STATUS_CODE(206, "Partial Content") \
    ENUM_HTTP_STATUS_CODE(207, "Multi-Status") \
    ENUM_HTTP_STATUS_CODE(208, "Already Reported") \
    ENUM_HTTP_STATUS_CODE(226, "IM Used") \
    ENUM_HTTP_STATUS_CODE(300, "Multiple Choice") \
    ENUM_HTTP_STATUS_CODE(301, "Moved Permanently") \
    ENUM_HTTP_STATUS_CODE(302, "Found") \
    ENUM_HTTP_STATUS_CODE(303, "See Other") \
    ENUM_HTTP_STATUS_CODE(304, "Not Modified") \
    ENUM_HTTP_STATUS_CODE(305, "Use Proxy") \
    ENUM_HTTP_STATUS_CODE(306, "unused") \
    ENUM_HTTP_STATUS_CODE(307, "Temporary Redirect") \
    ENUM_HTTP_STATUS_CODE(308, "Permanent Redirect") \
    ENUM_HTTP_STATUS_CODE(400, "Bad Request") \
    ENUM_HTTP_STATUS_CODE(401, "Unauthorized") \
    ENUM_HTTP_STATUS_CODE(402, "Payment Required") \
    ENUM_HTTP_STATUS_CODE(403, "Forbidden") \
    ENUM_HTTP_STATUS_CODE(404, "Not Found") \
    ENUM_HTTP_STATUS_CODE(405, "Method Not Allowed") \
    ENUM_HTTP_STATUS_CODE(406, "Not Acceptable") \
    ENUM_HTTP_STATUS_CODE(407, "Proxy Authentication Required") \
    ENUM_HTTP_STATUS_CODE(408, "Request Timeout") \
    ENUM_HTTP_STATUS_CODE(409, "Conflict") \
    ENUM_HTTP_STATUS_CODE(410, "Gone") \
    ENUM_HTTP_STATUS_CODE(411, "Length Required") \
    ENUM_HTTP_STATUS_CODE(412, "Precondition Failed") \
    ENUM_HTTP_STATUS_CODE(413, "Payload Too Large") \
    ENUM_HTTP_STATUS_CODE(414, "URI Too Long") \
    ENUM_HTTP_STATUS_CODE(415, "Unsupported Media Type") \
    ENUM_HTTP_STATUS_CODE(416, "Range Not Satisfiable") \
    ENUM_HTTP_STATUS_CODE(417, "Expectation Failed") \
    ENUM_HTTP_STATUS_CODE(418, "I'm a teapot") \
    ENUM_HTTP_STATUS_CODE(421, "Misdirected Request") \
    ENUM_HTTP_STATUS_CODE(422, "Unprocessable Entity") \
    ENUM_HTTP_STATUS_CODE(423, "Locked") \
    ENUM_HTTP_STATUS_CODE(424, "Failed Dependency") \
    ENUM_HTTP_STATUS_CODE(425, "Too Early") \
    ENUM_HTTP_STATUS_CODE(426, "Upgrade Required") \
    ENUM_HTTP_STATUS_CODE(428, "Precondition Required") \
    ENUM_HTTP_STATUS_CODE(429, "Too Many Requests") \
    ENUM_HTTP_STATUS_CODE(431, "Request Header Fields Too Large") \
    ENUM_HTTP_STATUS_CODE(451, "Unavailable For Legal Reasons") \
    ENUM_HTTP_STATUS_CODE(500, "Internal Server Error") \
    ENUM_HTTP_STATUS_CODE(501, "Not Implemented") \
    ENUM_HTTP_STATUS_CODE(502, "Bad Gateway") \
    ENUM_HTTP_STATUS_CODE(503, "Service Unavailable") \
    ENUM_HTTP_STATUS_CODE(504, "Gateway Timeout") \
    ENUM_HTTP_STATUS_CODE(505, "HTTP Version Not Supported") \
    ENUM_HTTP_STATUS_CODE(506, "Variant Also Negotiates") \
    ENUM_HTTP_STATUS_CODE(507, "Insufficient Storage") \
    ENUM_HTTP_STATUS_CODE(508, "Loop Detected") \
    ENUM_HTTP_STATUS_CODE(510, "Not Extended") \
    ENUM_HTTP_STATUS_CODE(511, "Network Authentication Required")

inline const char* status_message(int status)
{
    switch (status)
    {
#define ENUM_HTTP_STATUS_CODE(s, m) case s: return m;
        ENUMERATE_HTTP_STATUS_CODES
#undef ENUM_HTTP_STATUS_CODE
    default:
      return "Unknown Error";
    }
}

}
}
