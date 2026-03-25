#include <sparki.h>
#include "pitches.h" // include a list of pitches

// Written by: James Sheppard
// Age 4

// If wanting to test from console, just put the coordinates into desmos or something idk

// Declare repetitive values for readability and EOA
constexpr uint8_t BIT_VALUES[ 8 ]{ 0b10000000, 0b01000000, 0b00100000, 0b00010000, 0b00001000, 0b00000100, 0b00000010, 0b00000001 };

// These just represent where the Axis and Axis Signs are represented on the byte
constexpr uint8_t AXIS = 0b01000000;
constexpr uint8_t AXIS_SIGN = 0b10000000;

// These represent the X & Y axes and their Positive & Negative signs
constexpr uint8_t X_AXIS = 0x00 & AXIS;
constexpr uint8_t Y_AXIS = 0xFF & AXIS;
constexpr uint8_t POSITIVE_AXIS = 0xFF & AXIS_SIGN;
constexpr uint8_t NEGATIVE_AXIS = 0x00 & AXIS_SIGN;

// These bits are the length part of the Body Ray
constexpr uint8_t LENGTH_BITS = 0b00111111;

const uint8_t DEFAULT_DELAY_INTERVAL = 50;
const uint8_t APPLE_INCREASE = 4;

// Literally just the & operator
static uint8_t BinaryAnd( uint8_t inputByte, const uint8_t& binary )
{
  return inputByte & binary;
}

// Unsigned Vector (duh)
typedef struct UVector
{
  uint8_t X = 0;
  uint8_t Y = 0;

  UVector( uint8_t x, uint8_t y )
  {
    this->X = x;
    this->Y = y;
  }
} UVector;

// Signed Vector (duh)
typedef struct Vector
{
  int8_t X = 0;
  int8_t Y = 0;

  Vector( int8_t x, int8_t y )
  {
    this->X = x;
    this->Y = y;
  }
} Vector;

// Body struct declaration (notice how the byte size is 1 because I am awesome)
typedef struct Body
{
  uint8_t BodyDetails = 0;

  // Create body as a signed ray with magnitude and an axis
  static Body CreateBodyRay( const uint8_t& axis, const uint8_t& axisSign, uint8_t length )
  {
    return Body(( axis | axisSign ) | BinaryAnd( length, LENGTH_BITS ));
  }

  // Getters for Body Ray
  uint8_t GetLengthAsBodyRay() const { return BinaryAnd( BodyDetails, LENGTH_BITS ); }
  uint8_t GetAxisAsBodyRay() const { return BinaryAnd( BodyDetails, AXIS ); }
  uint8_t GetAxisSignAsBodyRay() const { return BinaryAnd( BodyDetails, AXIS_SIGN ); }

  // Dummy constructor
  Body() {}
  private:
    // REAL CONSTRUCTOR (obfuscated for factory)
    Body( uint8_t bodyDetails ) { BodyDetails = bodyDetails; }
} Body;

static void EnqueueBody( Body body );

// The movement of the head of the snake
static Vector MovementNormal( 1, 0 );

// The position of the head of the snake
static UVector HeadPosition( 64, 32 );
static UVector ApplePosition( 0, 0 );

// The "bodies" (a.k.a. tail)
static uint8_t BodiesLength = 1;
static Body* Bodies = new Body[ BodiesLength ]{ Body::CreateBodyRay( X_AXIS, NEGATIVE_AXIS, 3 ) };

// Score
static uint16_t Score = 1;

// Helpers to check if the snake is moving in a certain axis
static const bool IsMovingX() { return MovementNormal.X != 0; }
static const bool IsMovingY() { return MovementNormal.Y != 0; }

// Bodies Enqueue and Dequeue (works like queue)
static void EnqueueBody( Body body )
{
  BodiesLength++;
  Body* _newBodies = new Body[ BodiesLength ];

  for ( int i = 0; i < BodiesLength - 1; i++ )
    _newBodies[ i + 1 ] = Bodies[ i ];

  _newBodies[ 0 ] = body;

  delete Bodies;
  Bodies = _newBodies;
}
static void DequeueBody()
{
  if ( BodiesLength <= 0 ) return;

  BodiesLength--;
  Body* _newBodies = new Body[ BodiesLength ];

  for ( int i = 0; i < BodiesLength; i++ )
    _newBodies[ i ] = Bodies[ i ];

  delete Bodies;
  Bodies = _newBodies;
}

static bool DeadSnakeCondition()
{
    return sparki.readPixel( HeadPosition.X, HeadPosition.Y ) && ( HeadPosition.X != ApplePosition.X || HeadPosition.Y != ApplePosition.Y );
}

static void RandomApplePosition()
{
  ApplePosition.X = random( 33, 95 );
  ApplePosition.Y = random( 1, 63 );
}

void setup()
{
  randomSeed( static_cast< uint64_t >( sparki.lightCenter() ) );
  RandomApplePosition();

  Serial1.begin( 9600 );

  Serial1.print( "\u001b[2J" );
  Serial1.print( "Score: " );
  Serial1.print( Score );
}

void loop()
{ 
  uint8_t _currentKey = sparki.readIR();

  sparki.clearLCD();

  sparki.drawRectFilled( 0, 0, 32, 64 );
  sparki.drawRectFilled( 96, 0, 128, 64 );
  sparki.drawLine( 32, 0, 95, 0 );
  sparki.drawLine( 32, 63, 95, 63 );

  if ( HeadPosition.X == ApplePosition.X && HeadPosition.Y == ApplePosition.Y )
  {
    Bodies[ BodiesLength - 1 ].BodyDetails += APPLE_INCREASE;
    Score++;
    randomSeed( static_cast< uint64_t >( sparki.lightCenter() ) );

    sparki.beep( 1000 + ( Score - 1 ) * 100 );

    Serial1.print( "\u001b[2J" );
    Serial1.print( "Score: " );
    Serial1.print( Score );
    
    RandomApplePosition();
  }

  sparki.drawPixel( ApplePosition.X, ApplePosition.Y );

  // Change direction based on the current key
  switch ( _currentKey )
  {
    case 70:
      if ( IsMovingY() ) break; // If it's already moving in the axis specified, then don't do it, stupid. Snakes can't just 180.
      // We enqueue a body in the opposite direction to mimic a tail
      MovementNormal = Vector( 0, -1 );
      EnqueueBody( Body::CreateBodyRay( Y_AXIS, POSITIVE_AXIS, 0 ) ); // Set to 0 to only imply the direction (the head already acts as a point so inserting one as the length would just duplicate the movement)
      break;

    // All below are basically ditto to above
    case 68:
      if ( IsMovingX() ) break;
      MovementNormal = Vector( -1, 0 );
      EnqueueBody( Body::CreateBodyRay( X_AXIS, POSITIVE_AXIS, 0 ) );
      break;

    case 21:
      if ( IsMovingY() ) break;
      MovementNormal = Vector( 0, 1 );
      EnqueueBody( Body::CreateBodyRay( Y_AXIS, NEGATIVE_AXIS, 0 ) );
      break;

    case 67:
      if ( IsMovingX() ) break;
      MovementNormal = Vector( 1, 0 );
      EnqueueBody( Body::CreateBodyRay( X_AXIS, NEGATIVE_AXIS, 0 ) );
      break;
  }

  // Save the last position before we modify it
  UVector _lastPos( HeadPosition.X, HeadPosition.Y );
  
  // Modify that position
  HeadPosition.X += MovementNormal.X;
  HeadPosition.Y += MovementNormal.Y;

  for ( uint8_t _bodyIndex = 0; _bodyIndex < BodiesLength; _bodyIndex++ )
  {
    // Get the end position as separate to allow comparison to the original last position
    UVector _endPos( _lastPos.X, _lastPos.Y );

    // Just getting the Ray information
    uint8_t _axis = Bodies[ _bodyIndex ].GetAxisAsBodyRay();
    uint8_t _axisSign = Bodies[ _bodyIndex ].GetAxisSignAsBodyRay();
    uint8_t _length = Bodies[ _bodyIndex ].GetLengthAsBodyRay();

    // Add or subtract on the axis based on... the axis... and the sign of said axis.
    switch ( _axis | _axisSign )
    {
      case X_AXIS | POSITIVE_AXIS:
        _endPos.X += _length;
        break;
      case Y_AXIS | POSITIVE_AXIS:
        _endPos.Y += _length;
        break;
      case X_AXIS | NEGATIVE_AXIS:
        _endPos.X -= _length;
        break;
      case Y_AXIS | NEGATIVE_AXIS:
        _endPos.Y -= _length;
        break;
    }

    sparki.drawLine( _lastPos.X, _lastPos.Y, _endPos.X, _endPos.Y );

    // Set the last position to the end one so we can link the tail together
    _lastPos = _endPos;
  }

  // If our tail is being a tail that isn't straight, then let's mimic the tail straightening out by decreasing the end of the tail and adding onto the start to make it seem like the tail is being straightened out
  if ( BodiesLength > 1 )
  {
    Bodies[ 0 ].BodyDetails++;
    Bodies[ BodiesLength - 1 ].BodyDetails--;

    // Hey, if the length is 0, there is no tail.
    if ( Bodies[ BodiesLength - 1 ].GetLengthAsBodyRay() <= 0 ) DequeueBody();
  }

  // Long ass if statement that kills my snake
  if ( DeadSnakeCondition() )
  {
    Serial1.println( "\nYou are dead." );
    delay( 13 );
    
    uint16_t melody[] = { NOTE_C4, 0, NOTE_CS3, 0, NOTE_C3, 0, NOTE_B2 };
    // Make this death music!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    uint16_t noteDurations[] = { 8, 16, 8, 16, 8, 16, 1 };

    for (int thisNote = 0; thisNote < 8; thisNote++)
    {
      // calculate the note duration as 1 second divided by note type.
      //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
      int noteDuration = 1000/noteDurations[thisNote];
      sparki.beep(melody[thisNote],noteDuration);
      // to distinguish the notes, set a minimum time between them.
      // the note's duration + 30% seems to work well:
      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);
      // stop the tone playing:
      sparki.noBeep();
    }
    
    exit( 0 );
  }

  sparki.drawPixel( HeadPosition.X, HeadPosition.Y );

  sparki.updateLCD();
  delay( DEFAULT_DELAY_INTERVAL / Score );
}
