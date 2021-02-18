#pragma once

#include <utility>

#include <boost/mpl/bool.hpp>

template<typename T> struct PrintType;
template<typename T, typename ... R>
struct call {
  template<typename t>
  static constexpr boost::mpl::true_ SFINAE(
      t, std::remove_reference_t
      <decltype( std::declval<t>( )( std::declval<R>( )... ) )> * = nullptr )
  {
    return boost::mpl::true_( );
  }

  static constexpr boost::mpl::false_ SFINAE( ... ) {
    return boost::mpl::false_( );
  }

  static constexpr bool value = decltype( SFINAE( std::declval<T>( ) ) )::value;
};

template<typename T1, typename T2>
struct expansion {
  T1 first;
  T2 second;

  template<typename ... T, std::enable_if_t<call<const T1, T && ...>::value, bool> = true>
  auto operator ( )( T && ... arg ) const {
    return first( std::forward<T>( arg ) ... );
  }

  template<typename ... T, typename = std::enable_if_t<!call<const T1, T && ...>::value>>
  auto operator ( )( T && ... arg ) const {
    return second( std::forward< T >( arg ) ... );
  }

  template<typename ... T, std::enable_if_t<call<T1, T && ...>::value, bool> = true>
  auto operator ( )( T && ... arg ) {
    return first( std::forward<T>( arg ) ... );
  }

  template<typename ... T, typename = std::enable_if_t<!call<T1, T && ...>::value>>
  auto operator ( )( T && ... arg ) {
    return second( std::forward< T >( arg ) ... );
  }

  expansion( const T1 & t1, const T2 & t2 ) : first( t1 ), second( t2 ) { }
};

template<typename T1, typename T2>
expansion<T1, T2> make_expansion( const T1 & t1, const T2 & t2 ) {
  return expansion< T1, T2 >( t1, t2 );
}
