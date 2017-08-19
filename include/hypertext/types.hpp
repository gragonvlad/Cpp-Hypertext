#ifndef CPP_HT_TYPES_HPP
#define CPP_HT_TYPES_HPP

#include <chrono>
#include <string>
#include <iterator>
#include <functional>

#include "beast/http/error.hpp"
#include "beast/http/parser.hpp"
#include "beast/http/message.hpp"
#include "beast/http/empty_body.hpp"
#include "beast/http/string_body.hpp"
#include "beast/http/dynamic_body.hpp"
#include "beast/core/flat_buffer.hpp"

namespace hypertext {
namespace types {

using request_header   = beast::http::request_header<>;
using response_header  = beast::http::request_header<>;
using emptybody_parser = beast::http::parser<false, beast::http::empty_body>;

struct in_place_construct_t {};

/*
 */
class request:
  public beast::http::request<beast::http::string_body>
{
public: // 'tors
  request() = default;
  request(const request&) = default;
  request(request&&) = default;
  request& operator=(const request&) = default;
  request& operator=(request&&) = default;
  ~request() = default;

public: // Exposed APIs
  /*
   */
  const request_header& header() const noexcept
  {
    return (*this);
  }

  /*
   */
  request_header& header() noexcept
  {
    return (*this);
  }
  
};

/*
 */
class response: 
  public beast::http::response<beast::http::dynamic_body>
{
public: // 'tors
  response() = default;
  response(const response&) = delete;
  response(response&&) = default;
  response& operator=(const response&) = delete;
  response& operator=(response&&) = default;

public:
  /*
   */
  template <typename ChunkBodyStorage = std::string>
  class chunk_response_block
  {
  public: //typedefs
    using storage_type = ChunkBodyStorage;

  public: // 'tors
    chunk_response_block(in_place_construct_t);
    chunk_response_block(const chunk_response_block&) = delete;
    chunk_response_block(chunk_response_block&&) = default;
    chunk_response_block& operator=(const chunk_response_block&) = delete;
    chunk_response_block& operator=(chunk_response_block&&) = default;

  public: // Exposed APIs
    /*
     */
    emptybody_parser&
    parser() noexcept { return parser_; }

    /*
     */
    beast::flat_buffer&
    pbuf() noexcept { return pbuf_; }

    /*
     */
    beast::error_code
    get_error() const noexcept { return ec_; }

    /*
     */
    beast::string_view
    get_chunk_body() noexcept { return {chunk_body_}; }

    /*
     */
    ChunkBodyStorage&
    get_storage() noexcept { return chunk_body_; }

    /*
     */
    template <typename TransportAdapter>
    bool fill_in_next_chunk(TransportAdapter& transport);

  private:
    /// Storage for storing the chunk body
    ChunkBodyStorage chunk_body_;
    /// The dynamic buffer used for parsing
    beast::flat_buffer pbuf_;
    /// HTTP parser
    emptybody_parser parser_;
    /// The error code
    beast::error_code ec_;

    //Chunk decoding callbacks
    std::function<
      void(uint64_t,
           beast::string_view,
           beast::error_code&)> on_chunk_header_cb_;

    std::function<
      size_t(uint64_t,
             beast::string_view,
             beast::error_code&)> on_chunk_body_cb_;
  };


  /*
   */
  template <typename CRB, typename Transport>
  class chunk_iterator
  {
  public: // iterator typedefs
    using value_type = beast::string_view;
    using pointer    = typename std::add_pointer<CRB>::type;
    using reference  = typename std::add_lvalue_reference<CRB>::type;
    using difference_type = size_t;
    using iterator_category = std::input_iterator_tag;

    /*
     * https://stackoverflow.com/questions/35165828/how-to-implement-dereference-and-post-increment-for-input-iterator-in-c
     */
    class post_increment_proxy
    {
    public:
      post_increment_proxy(const typename CRB::storage_type& s)
        : value_(s)
      {}
      post_increment_proxy(const post_increment_proxy&) = default;
      post_increment_proxy& operator=(const post_increment_proxy&) = default;

    public:
      typename CRB::storage_type&
      operator*() const 
      {
        return value_;
      }

    private:
      typename CRB::storage_type value_;
    };

  public: // 'tors
    chunk_iterator(CRB& crb, Transport& transport)
      : crb_ref_(crb)
      , transport_(transport)
    {}

    chunk_iterator(bool finish)
      : finished_(finish)
    {}

    chunk_iterator(const chunk_iterator&) = default;
    chunk_iterator& operator=(const chunk_iterator&) = default;

  public: // Input Iterator Requirements
    /*
     */
    value_type operator*() const noexcept
    {
      return crb_ref_.get().get_chunk_body();
    }

    /*
     */
    pointer operator->() const noexcept
    {
      return &(crb_ref_.get());
    }

    /*
     */
    chunk_iterator& operator++() noexcept
    {
      finished_ = !(crb_ref_.get().fill_in_next_chunk(transport_.get()));
      return *this;
    }

    /*
     */
    post_increment_proxy operator++(int) noexcept
    {
      post_increment_proxy result{crb_ref_.get().get_storage()};
      finished_ = !(crb_ref_.get().fill_in_next_chunk(transport_.get()));
      return result;
    }

    /*
     */
    bool finished() const noexcept { return finished_; }

  private:
    ///
    bool finished_ = false;
    ///
    boost::optional<CRB&> crb_ref_;
    ///
    boost::optional<Transport&> transport_;
  };

public: // Exposed APIs


  /*
   */
  template <typename Transport>
  chunk_iterator<chunk_response_block<>, Transport>
  chunk_iter(Transport& t)
  {
    return chunk_iterator<chunk_response_block<>, Transport>{chunk_resp_.get(), t};
  }
 
  /*
   */
  template <typename Transport>
  chunk_iterator<chunk_response_block<>, Transport>
  chunk_end(Transport& t) const
  {
    return chunk_iterator<chunk_response_block<>, Transport>{true};
  }

  /*
   */
  void set_chunked_response()
  {
    chunk_resp_.emplace(in_place_construct_t{});
  }

  chunk_response_block<>&
  get_chunk_response_block()
  {
    return chunk_resp_.get();
  }

  /*
   */
  bool has_chunked_response() const noexcept
  {
    return !(!chunk_resp_);
  }

  /*
   */
  std::chrono::milliseconds elapsed_time() const noexcept
  {
    return elapsed_time_;
  }

private:
  std::chrono::milliseconds elapsed_time_;
  boost::optional<chunk_response_block<>> chunk_resp_;
};


} // END namespace hypertext
} // END namespace hypertext

#include "hypertext/impl/response.ipp"

#endif
