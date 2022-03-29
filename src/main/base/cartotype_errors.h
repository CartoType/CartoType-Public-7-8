/*
cartotype_errors.h
Copyright (C) 2004-2020 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_ERRORS_H__
#define CARTOTYPE_ERRORS_H__

#include <cartotype_types.h>
#include <assert.h>
#include <stdlib.h>
#include <string>

namespace CartoType
{

/**
The result and error code type. It is a class, to enforce initialization to zero, and
to allow a breakpoint to be placed in the constructor or assignment operator.
In release builds on modern compilers the use of a class rather than a plain integer has no cost.
*/
class TResult
    {
    public:
    constexpr TResult() noexcept { }
    /** Creates a TResult object containing a specified code. */
    constexpr TResult(uint32_t aCode) noexcept
        {
        *this = aCode;
        }
    /** Returns the integer code of this TResult object. */
    constexpr operator uint32_t() const noexcept { return iCode; }
    /** Assigns an integer code to this TResult object. */
    constexpr void operator=(uint32_t aCode) noexcept
        {
        iCode = aCode;
        }

    private:
    uint32_t iCode = 0;
    };

/** No error; a successful result. */
constexpr TResult KErrorNone = 0;

/**
Use KErrorGeneral where an error has occurred but no other CartoType
error is applicable. For example, this error code can be used
when FreeType returns an error code for illegal TrueType hinting instructions.
*/
constexpr TResult KErrorGeneral = 1;

/** Use KErrorNoMemory when an attempted memory allocation fails. */
constexpr TResult KErrorNoMemory = 2;

/**
KErrorEndOfData is returned by iterators and streams when no more data
is available. It may be treated as an error or not depending on the
circumstances.
*/
constexpr TResult KErrorEndOfData = 3;

/**
KErrorTextUnmodifiable is returned when an attempt is made to
modify an unmodifiable string represented by a class derived
from MString.
*/
constexpr TResult KErrorTextUnmodifiable = 4;

/**
KErrorTextTruncated is returned when text inserted into
a string derived from MString is too long for a fixed-size
buffer.
*/
constexpr TResult KErrorTextTruncated = 5;

/**
KErrorNotFound is returned whenever a resource like a file
is not found, or a search fails.
It may be treated as an error or not depending on the
circumstances.
*/
constexpr TResult KErrorNotFound = 6;

/**
The code KErrorUnimplemented is used for functions that have
not yet been implemented, or for circumstances or options
within functions that remain unimplemented because they
are deemed to be either rarely or never useful.
*/
constexpr TResult KErrorUnimplemented = 7;

/**
If the graphics system detects an invalid outline, such as one
that starts with a cubic control point, it returns
KErrorInvalidOutline.
*/
constexpr TResult KErrorInvalidOutline = 8;

/**
The code KErrorIo should be used for unexpected read or write
errors on files or other data streams. This is for where you
attempt to read data that should be there, but the OS says
it's unavailable (e.g., someone has taken the CD out of the drive
or the network is down).
*/
constexpr TResult KErrorIo = 9;

/**
Use KErrorCorrupt when illegal values are found when reading data
from a file or other serialized form.
*/
constexpr TResult KErrorCorrupt = 10;

/**
This error is returned by database accessors, typeface accessors, etc.,
to indicate that this data format is unknown to a particular accessor.
It is not really an error but tells the caller to try the next
accessor in the list.
*/
constexpr TResult KErrorUnknownDataFormat = 11;

/**
This error is returned by transformation inversion if the transformation
has no inverse.
*/
constexpr TResult KErrorNoInverse = 12;

/**
This error is returned by the projection system if the projection
causes overflow.
*/
constexpr TResult KErrorProjectionOverflow = 13;

/**
The code KErrorCancel is returned to cancel an asynchronous
operation that uses callbacks. The callback or virtual function
returns this code. the caller must terminate further processing
on receipt of any error, but this one indicates that there has been
no actual error, but the handler has canceled the operation.
*/
constexpr TResult KErrorCancel = 14;

/**
This error is returned when an invalid argument has been given to a function.
*/
constexpr TResult KErrorInvalidArgument = 15;

/**
This error is returned by a data reader when it cannot deal with the data version.
*/
constexpr TResult KErrorUnknownVersion = 16;

/**
This error is returned by the base library when reading data or calculations result
in overflow.
*/
constexpr TResult KErrorOverflow = 17;

// ABOLISHED: constexpr TResult KErrorOverlap = 18;

/**
The error code returned by line intersection algorithms when the lines are parallel.
*/
constexpr TResult KErrorParallelLines = 19;

//  ABOLISHED: constexpr TResult KErrorPathLengthExceeded = 20;

//  ABOLISHED: constexpr TResult KErrorMaxTurnExceeded = 21;

/**
An attempt was made to draw a bitmap needing a color palette
but no palette was provided.
*/
constexpr TResult KErrorNoPalette = 22;

/**
An attempt was made to insert a duplicate object into a collection
class that does not allow duplicates.
*/
constexpr TResult KErrorDuplicate = 23;

/**
The projection for converting latitude and longitude to map coordinates
is unknown or unavailable.
*/
constexpr TResult KErrorNoProjection = 24;

/**
A palette is full and no new entries can be added.
*/
constexpr TResult KErrorPaletteFull = 25;

/**
The dash array for drawing strokes is invalid.
*/
constexpr TResult KErrorInvalidDashArray = 26;

// ABOLISHED: constexpr TResult KErrorCentralPath = 27;

/**
A route was needed by the navigation system
but no route was available.
*/
constexpr TResult KErrorNoRoute = 28;

/**
There was an attempt to zoom beyond the legal zoom limits for a map.
*/
constexpr TResult KErrorZoomLimitReached = 29;

/**
There was an attempt to project a map object that had already been projected.
*/
constexpr TResult KErrorAlreadyProjected = 30;

/**
Conditions are too deeply nested in the style sheet.
*/
constexpr TResult KErrorConditionsTooDeeplyNested = 31;

/**
There was an attempt to use a null font for drawing text.
*/
constexpr TResult KErrorNullFont = 32;

/**
An attempt to read data from the internet failed.
*/
constexpr TResult KErrorInternetIo = 33;

/**
Division by zero in an interpreted expression.
*/
constexpr TResult KErrorDivideByZero = 34;

/**
A transform failed because an argument or result was out of range.
*/
constexpr TResult KErrorTransformFailed = 35;

/**
Reading a bitmap from PNG format failed.
*/
constexpr TResult KErrorPngRead = 36;

/**
Reading a bitmap from JPG format failed.
*/
constexpr TResult KErrorJpgRead = 37;

/**
An object did not intersect a specified region.
*/
constexpr TResult KErrorNoIntersection = 38;

/**
An operation was interrupted, for example by another thread writing to a shared flag.
*/
constexpr TResult KErrorInterrupt = 39;

/**
There was an attempt to use map databases of incompatible formats
(TMapGrid values containing point format, datum and axis orientations)
to draw a map or find objects in a map.
*/
constexpr TResult KErrorMapDataBaseFormatMismatch = 40;

/** A key supplied for encryption was too short. */
constexpr TResult KErrorEncryptionKeyTooShort = 41;

/** No encryption key has been set. */
constexpr TResult KErrorNoEncryptionKey = 42;

/** A code for standard emergency messages. */
constexpr TResult KErrorEmergencyMessage = 43;

/** A code for standard alert messages. */
constexpr TResult KErrorAlertMessage = 44;

/** A code for standard critical messages. */
constexpr TResult KErrorCriticalMessage = 45;

/** A code for standard error messages. */
constexpr TResult KErrorErrorMessage = 46;

/** A code for standard warning messages. */
constexpr TResult KErrorWarningMessage = 47;

/** A code for standard notice messages. */
constexpr TResult KErrorNoticeMessage = 48;

/** A code for standard information messages. */
constexpr TResult KErrorInfoMessage = 49;

/** A code for standard debug messages. */
constexpr TResult KErrorDebugMessage = 50;

/** A function has been called which is available only when navigating. */
constexpr TResult KErrorNotNavigating = 51;

/** The global framework object does not exist. */
constexpr TResult KErrorNoFramework = 52;

/** The global framework object already exists. */
constexpr TResult KErrorFrameworkAlreadyExists = 53;

/** A string was not transliterable. */
constexpr TResult KErrorUntransliterable = 54;

/** Writing a bitmap to PNG format failed. */
constexpr TResult KErrorPngWrite = 55;

/** There was an attempt to write to a read-only map database. */
constexpr TResult KErrorReadOnlyMapDataBase = 56;

/** There was an error in the PROJ.4 projection library error other than a projection overflow. */
constexpr TResult KErrorProj4 = 57;

/** A function was called from the unlicensed version of CartoType that is available only in the licensed version. */
constexpr TResult KErrorUnlicensed = 58;

/** No route could be created because there were no roads near the start point of a route section. */
constexpr TResult KErrorNoRoadsNearStartOfRoute = 59;

/** No route could be created because there were no roads near the end point of a route section. */
constexpr TResult KErrorNoRoadsNearEndOfRoute = 60;

/** No route could be created because the start and end point were not connected. */
constexpr TResult KErrorNoRouteConnectivity = 61;

/** An unsupported feature was requested from the XML parser. */
constexpr TResult KErrorXmlFeatureNotSupported = 62;

/** A map file was not found. */
constexpr TResult KErrorMapNotFound = 63;

/** A font file was not found. */
constexpr TResult KErrorFontNotFound = 64;

/** A style sheet was not found.  */
constexpr TResult KErrorStyleSheetNotFound = 65;

/** The number of standard error codes. */
constexpr int32_t KStandardErrorCodeCount = 66;

/** Returns a short description of an error, given its code. */
std::string ErrorString(uint32_t aErrorCode);


/**
The start of the range of errors in XML parsing or in the style sheet format, such as a syntactically incorrect dimension or color.
The top byte is 0x10 for style sheet errors, or in the range 0x11 to 0x2A for Expat XML parser error codes.

The low three bytes give the error location: one byte for the column number (clamped to 0...255) two bytes for the line number (clamped to 0...65535).
*/
constexpr TResult KErrorXmlRangeStart = 0x10000000;

/** The end of the range of errors in XML parsing or in the style sheet format. */
constexpr TResult KErrorXmlRangeEnd = 0x2AFFFFFF;

/**
The base of error codes for returning SQLite errors.
The SQLite error code is placed in the lower two bytes.
*/
constexpr uint32_t KErrorSQLite = 0x30000000;

/** Result codes for drawing operations. */
enum class TDrawResult
    {
    Success,
    Overlap,
    MaxTurnExceeded,
    PathLengthExceeded,
    GlyphNotFound,
    TransformFailed
    };

// For unit tests
#ifdef CARTOTYPE_TEST
inline void Panic() { abort(); }
inline void Check(bool aExp) { if (!aExp) Panic(); }
inline void Check(int aExp) { if (!aExp) Panic(); }
inline void Check(const void* aPtr) { if (!aPtr) Panic(); }
#endif

} // namespace CartoType

#endif
