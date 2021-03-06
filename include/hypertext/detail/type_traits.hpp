#ifndef CPP_HT_DETAIL_TYPE_TRAITS
#define CPP_HT_DETAIL_TYPE_TRAITS

#include <string>
#include <type_traits>
#include "hypertext/types.hpp"

namespace beast = boost::beast;

namespace hypertext {
namespace detail {

template <typename... T>
struct make_void
{
  using type = void;
};

template <typename... T>
using void_t = typename make_void<T...>::type;


template <bool... B>
struct bool_pack {};

template <bool... B>
using all_true = std::is_same<bool_pack<true, B...>, bool_pack<B..., true>>;

//-------------------------------------------------

/*
 */
template <typename T, typename=void>
struct is_parameter: std::false_type
{
};

template <typename T>
struct is_parameter<T, void_t<
                          decltype(
                              std::declval<T&>().get(),
                              (void)0)
                       >
                   >: std::true_type
{
};

/*
 */
template <bool B, typename... T>
struct are_all_parameters;

template <typename... T>
struct are_all_parameters<true, T...>: std::true_type
{
};


/*
 */
template <typename T, typename=void>
struct is_header_compatible: std::false_type
{
};

template <typename T>
struct is_header_compatible<T, void_t<
                                typename std::enable_if<
                                  std::is_constructible<beast::string_view, typename T::key_type>::value,
                                  void
                                >::type,

                                typename std::enable_if<
                                  std::is_constructible<beast::string_view, typename T::mapped_type>::value,
                                  void
                                >::type,

                                decltype(
                                    //TODO: Improve the traits check
                                    std::declval<T&>().operator[](std::declval<typename std::remove_reference_t<T>::key_type>()),
                                    std::declval<T&>().begin(),
                                    std::declval<T&>().end(),
                                    (void)0)
                               >
                           >: std::true_type
{
};


/*
 */
template <typename T, typename=void>
struct is_authorization: std::false_type
{
};

//TODO: This is very poor
template <typename T>
struct is_authorization<T, void_t<
                            decltype(
                              std::declval<std::string>() = std::declval<T&>(),
                              (void)0)
                           >
                       >: std::true_type
{
};


/*
 */

template <typename T, typename=void>
struct is_transport_adapter: std::false_type
{
};

template <typename T>
struct is_transport_adapter<T, void_t<
                                decltype(
                                    // Send bytes without ssl
                                    std::declval<T&>().send(
                                      std::declval<const types::request&>(),
                                      std::declval<beast::string_view>(),
                                      std::declval<uint16_t>(),
                                      std::declval<bool>()),

                                    // Send bytes with SSL
                                    std::declval<T&>().send_secure(
                                      std::declval<const types::request&>(),
                                      std::declval<beast::string_view>(),
                                      std::declval<uint16_t>(),
                                      std::declval<bool>(),
                                      std::declval<const boost::optional<boost::variant<std::string, bool>>&>(),
                                      std::declval<const boost::optional<std::string>&>()),

                                    std::declval<T&>().close(),
                                    (void)0)
                               >
                           >: std::true_type
{
};


/*
 */
template <typename T, typename=void>
struct is_named_arg: std::false_type
{
};

template <typename T>
struct is_named_arg<T, void_t<
                        typename std::enable_if<
                            std::is_same<decltype(T::value()), char const*>::value,
                            void>::type
                       >
                   >: std::true_type
{
};


/*
 */
template <typename T, typename=void>
struct is_auth_concept: std::false_type
{
};

template <typename T>
struct is_auth_concept<T, void_t<
                            decltype(
                            std::declval<std::string>() = 
                                std::declval<T&>().encoded_str(std::declval<types::request&>()),
                            (void)0)
                          >
                   >: std::true_type
{
};

} // END namespace detail
} // END namespace hypertext

#endif
