#ifndef CPP_HT_PARAMTERS_HPP
#define CPP_HT_PARAMTERS_HPP

#include <chrono>

#include "beast/http/verb.hpp"
#include "beast/core/string.hpp"
#include "hypertext/types.hpp"

namespace hypertext {
namespace parameters {

// Fwd. decl all the available paremters

/// The HTTP method for the request.
struct method_param;

/// The dictionary/map of HTTP headers to be sent.
struct headers_param;

/// The Authorization field.
template <typename AuthConcept>
struct auth_param;

/// How many milli-seconds to wait for the server to send response.
struct timeout_param;

/// Option to enable streaming download
struct stream;

/// Method Parameter Definitions
//------------------------------

struct method_param
{
  beast::http::verb get() const noexcept;

  beast::http::verb method_;
};

struct headers_param
{
  headers_param(types::request_header);
  types::request_header get();

  types::request_header headers_;
};

template <typename AuthConceptT>
struct auth_param
{
  auth_param(AuthConceptT&&);
  AuthConceptT get();

  AuthConceptT auth_;
};

struct timeout_param
{
  timeout_param(std::chrono::milliseconds);
  std::chrono::milliseconds get() const noexcept;
  std::chrono::milliseconds timeout_;
};

struct stream_param
{
  stream_param(bool);
  bool get() const noexcept;
  bool stream_ = false;
};

/// Parameter creators
//-------------------------------

/*
 */
method_param 
method(beast::http::verb);

/*
 */
headers_param 
headers(
    const std::initializer_list<
                std::pair<beast::string_view, beast::string_view>
          >&);

/*
 */
template <typename HeaderConceptT>
headers_param
headers(HeaderConceptT&&);

/*
 */
template <typename AuthConceptT>
auth_param<typename std::decay<AuthConceptT>::type> 
auth(AuthConceptT&&);

//TODO: Provide a default for HTTPBasicAuth

/*
 */
timeout_param 
timeout(std::chrono::milliseconds);

/*
 */
timeout_param 
timeout(std::chrono::seconds);

/*
 */
stream_param
stream(bool);

} // END namespace parameters
} // END namespace hypertext

#include "hypertext/impl/parameters.ipp"

#endif
