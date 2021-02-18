#include "resist.h"

int main() {

  execute( Seq(
    If( True( ), Print( String( "hello " ) ), Print( True( ) ) ),
    If( Or( False( ), Neg( True( ) ) ), Print( Unit( ) ), Print( String( "world!\n" ) ) ) ) );

  execute( Seq(
    When( True( ), Print( String( "hello " ) ) ),
    Unless( False( ), Print( String( "world!\n" ) ) ) ) );

  execute( Print( Print( Seq( Print( True( ) ), Print( False( ) ) ) ) ) );

  execute( Print( Concat( String( "4" ), Show( True( ) ) ) ) );

  assert( value_to_bool( execute( IsDefined( String( "not defined" ) ) ) ) == false );

  execute(
    Seq( Set( String( "SomeVar" ), String( "12" ) ),
      Seq( Print( Get( String( "SomeVar" ) ) ),
        Seq( Set( Concat( String( "S" ), String( "omeVar" ) ), String( "345\n" ) ),
          Print( Get( Concat( String( "Some" ), String( "Var" ) ) ) ) ) ) ) );

  execute( Seq(
    If( True( ), Set( String( "hello " ), String( "hellos \n" ) )
      , Print( True( ) ) )
    , Print( Get( String( "hello " ) ) )
    )
  );

  execute( Seq( Scope( Set( String( "hi" ), True( ) ) ), Print( IsDefined( String( "hi" ) ) ) ) );

  assert(
    value_to_bool( execute( Seq( Seq( Set( String( "var" ), False( ) ), Scope( Set( String( "var" ), True( ) ) ) )
                               , Get( String( "var" ) ) ) ) ) );

  execute(
    Seq(
      Seq( Set( String( "b1" ), True( ) ), Set( String( "b2" ), True( ) ) ),
      While(
        Or( Get( String( "b1" ) ), Get( String( "b2" ) ) ),
        If( Get( String( "b1" ) ),
          Seq( Set( String( "b1" ), False( ) ), Print( String( "Hello" ) ) ),
          Seq( Set( String( "b2" ), False( ) ), Print( String( "World" ) ) ) ) ) ) );

  execute( Print( String( "\n" ) ) );

  return 0;
}
