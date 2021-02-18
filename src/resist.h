#include "adt.h"

#define DECLARE_CONSTRUCTOR( ADT, WHICH, NAME, FRESH ) \
  template<typename ... FRESH> \
  auto NAME( FRESH && ... BOOST_PP_CAT( FRESH, 0 ) ) { \
    return constructor<ADT, WHICH>( std::forward<FRESH>( BOOST_PP_CAT( FRESH, 0 ) ) ... ); \
  }

#define DECLARE_CONSTRUCTORS_AUX( r, data, which, name ) \
  DECLARE_CONSTRUCTOR( BOOST_PP_SEQ_ELEM( 0, data ), which, name, BOOST_PP_SEQ_ELEM( 1, data ) )

#define DECLARE_CONSTRUCTORS( ADT, NAMES, FRESH ) \
  BOOST_PP_SEQ_FOR_EACH_I_R( 1, DECLARE_CONSTRUCTORS_AUX, (ADT)(FRESH), BOOST_PP_TUPLE_TO_SEQ( NAMES ) )

#define DECLARE_ADT_AUX_0( data, elem ) BOOST_PP_TUPLE_ELEM( 0, elem )

#define DECLARE_ADT_AUX_1( data, elem ) \
  BOOST_PP_IF( BOOST_PP_EQUAL( BOOST_PP_TUPLE_SIZE( elem ), 1 ), (unit), \
    BOOST_PP_TUPLE_POP_FRONT( elem ) )

#define DECLARE_ADT_AUX_2( data, i, elem ) BOOST_PP_COMMA_IF( BOOST_PP_GREATER( i, 0 ) ) std::tuple<BOOST_PP_TUPLE_ENUM( elem )>

#define TUPLE_TRANSFORM_AUX( r, count, data ) \
  BOOST_PP_TUPLE_ELEM( 0, data )( BOOST_PP_TUPLE_ELEM( 1, data ), BOOST_PP_TUPLE_ELEM( count, BOOST_PP_TUPLE_ELEM( 2, data ) ) ),

#define TUPLE_TRANSFORM( op, data, tuple ) \
  ( BOOST_PP_REPEAT( BOOST_PP_DEC( BOOST_PP_TUPLE_SIZE( tuple ) ), TUPLE_TRANSFORM_AUX, ( op, data, tuple ) ) \
    op( data, BOOST_PP_TUPLE_ELEM( BOOST_PP_DEC( BOOST_PP_TUPLE_SIZE( tuple ) ), tuple ) ) )

#define TUPLE_FOREACH_I_AUX( r, count, data ) \
  BOOST_PP_TUPLE_ELEM( 0, data )( BOOST_PP_TUPLE_ELEM( 1, data ), count, BOOST_PP_TUPLE_ELEM( count, BOOST_PP_TUPLE_ELEM( 2, data ) ) )

#define TUPLE_FOREACH_I( op, data, tuple ) \
  BOOST_PP_REPEAT( BOOST_PP_TUPLE_SIZE( tuple ), TUPLE_FOREACH_I_AUX, ( op, data, tuple ) )

#define DECLARE_ADT( ADT, CONSTRUCTORS, FRESH ) \
  typedef typename \
    make_algebraic_data_type<TUPLE_FOREACH_I( DECLARE_ADT_AUX_2, _, TUPLE_TRANSFORM( DECLARE_ADT_AUX_1, _, CONSTRUCTORS ) )>::type ADT; \
  DECLARE_CONSTRUCTORS( ADT, TUPLE_TRANSFORM( DECLARE_ADT_AUX_0, _, CONSTRUCTORS ), FRESH )

typedef recursive_indicator ri;

DECLARE_ADT( expr,
     ( (Unit),
       (True),
       (False),
       (Print,  ri),
       (Seq,    ri, ri),
       (If,     ri, ri, ri),
       (String, std::string),
       (Show,   ri),
       (Concat, ri, ri),
       (Set,    ri, ri),
       (Get,    ri),
       (IsDefined,  ri),
       (Define, ri, ri),
       (Scope,  ri),
       (While,  ri, ri)
     ), X )

typedef std::map<std::string, expr> symbol_table;

using std::optional;
using std::reference_wrapper;

template<typename T>
using orw = optional<reference_wrapper<T>>;

struct environment {
  symbol_table st;
  orw<environment> parent;

  orw<expr> get( const std::string & str ) {
    auto it = st.find( str );
    return
      it != st.end( ) ?
        reference_wrapper<expr>{
          it->second
        }
      : ( parent ? parent->get( ).get( str ) : orw<expr>{ } );
  }

  void define( const std::string & str, const expr & e ) {
    auto res = st.insert( std::make_pair( str ,e ) );
    assert( res.second );
  }

  bool assign( const std::string & str, const expr & e ) {
    auto it = st.find( str );
    return it != st.end( ) ? ( it->second = e, true ) : ( parent ? parent->get( ).assign( str, e ) : false );
  }

  void set( const std::string & str, const expr & e ) {
    if ( ! assign( str, e ) ) {
      st.insert( std::make_pair( str, e ) );
    }
  }
};

expr And( const expr & l, const expr & r ) {
  return If( l, r, False( ) );
}
expr Or( const expr & l, const expr & r ) {
  return If( l, True( ), r );
}
expr Neg( const expr & e ) {
  return If( e, False( ), True( ) );
}
expr When( const expr & con, const expr & act ) {
  return If( con, act, Unit( ) );
}
expr Unless( const expr & con, const expr & act ) {
  return If( con, Unit( ), act );
}

bool value_to_bool( const expr & e ) {
  return e.match(
    with( True( uim ), []( ){
        return true;
      } ),
    with( False( uim ), []( ){
        return false;
      } ) );
}

std::string value_to_string( const expr & e ) {
  return e.match( with( String( arg ), []( const std::string & str ){
    return str;
  } ) );
}

std::string show( const expr & e ) {
  return e.match(
    with( Unit( uim ),   []( ) { return "tt"; } ),
    with( True( uim ),   []( ) { return "true"; } ),
    with( False( uim ),  []( ) { return "false"; } ),
    with( String( arg ), []( const std::string & str ) { return str; } ) );
}

expr execute( std::tuple<environment> st, const expr & e );
expr execute( environment & env, const expr & e ) {
  return
    e.match(
      with( Print( arg ), [&]( const expr & exp ) { std::cout << show( execute( env, exp ) ); return Unit( ); } ),
      with( Seq( arg, arg ), [&]( const expr & l, const expr & r ) { execute( env, l ); return execute( env,r ); } ),
      with( Unit( uim ), []( ){ return Unit( ); } ),
      with( True( uim ), []( ){ return True( ); } ),
      with( False( uim ), []( ){ return False( ); } ),
      with(
        If( arg, arg, arg ),
        [&]( const expr & i, const expr & t, const expr & e )
        { return value_to_bool( execute( env, i ) ) ? execute( env, t ) : execute( env, e ); } ),
      with( String( arg ), []( const std::string & str ) { return String( str ); } ),
      with( Show( arg ), [&]( const expr & e ) { return String( show( execute( env, e ) ) ); } ),
      with(
        Concat( arg, arg ),
        [&]( const expr & l, const expr & r )
        { return String( value_to_string( execute( env, l ) ) + value_to_string( execute( env, r ) ) ); } ),
      with(
        IsDefined( arg ),
        [&]( const expr & exp ) { return env.get( value_to_string( execute( env, exp ) ) ) ? True( ) : False( ); } ),
      with(
        Define( arg, arg ),
        [&]( const expr & l, const expr & r ) { env.define( value_to_string( execute( env, l ) ), r ); return Unit( ); } ),
      with( Get( arg ), [&]( const expr & e ) { return *env.get( value_to_string( execute( env, e ) ) ); } ),
      with(
        Set( arg, arg ),
        [&]( const expr & l, const expr & r )
        {
          env.set( value_to_string( execute( env, l ) ), r );
          return Unit( );
        } ),
      with( Scope( arg ), [&]( const expr & e ) { return execute( std::make_tuple( environment{ symbol_table{ }, { env } } ), e ); } ),
      with(
        While( arg, arg ),
        [&]( const expr & b, const expr & act )
        {
          while ( value_to_bool( execute( env, b ) ) ) { execute( env, act ); }
          return Unit( );
        } ) );
}

expr execute( std::tuple<environment> st, const expr & e ) {
  return execute( std::get< 0 >( st ), e );
}

expr execute( const expr & e ) {
  return execute( std::make_tuple( environment{ } ), e );
}
