/*
cartotype_stream.h
Copyright (C) 2004-2020 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_STREAM_H__
#define CARTOTYPE_STREAM_H__

#include <cartotype_types.h>
#include <cartotype_arithmetic.h>
#include <cartotype_errors.h>
#include <cartotype_list.h>
#include <cartotype_string.h>
#include <string.h>
#include <stdio.h>

#ifdef __unix__
    #include <unistd.h> // to define _POSIX_VERSION
#endif

// Use low-level file i/o on Windows, but not Windows CE, for a small speed improvement (about 5%).
#if (defined(_MSC_VER) && !defined(_WIN32_WCE))
    #define CARTOTYPE_LOW_LEVEL_FILE_IO
#endif

#ifdef ANDROID
    #define CARTOTYPE_LOW_LEVEL_FILE_IO
#endif

#ifdef CARTOTYPE_LOW_LEVEL_FILE_IO
    #if defined(ANDROID)
        #include <unistd.h>
        #include <fcntl.h>
        #include <errno.h>
    #else
        #include <io.h>
    #endif
#endif

#undef COLLECT_STATISTICS

namespace CartoType
{

// Forward declarations.
class MString;
class CString;

/**
The input stream interface.
Streams that do not support random access always throw exceptions for Seek
and may throw exceptions for Position and Length.

Exceptions that are not caused by lack or memory are of the type TResult.
Common values are KErrorEndOfData and KErrorIo.
*/
class MInputStream
    {
    public:
    /**
    Virtual destructor: strictly unneeded since pointers to MInputStream are not owned
    and should not be deleted.
    */
    virtual ~MInputStream() { }
    /**
    Read some data into a buffer owned by the MInputStream object and return
    a pointer to it in aPointer. Return the number of bytes of data in aLength.
    This function will return at least one byte if there are bytes remaining in the
    stream. The pointer is valid until the next call to Read.
    */
    virtual void Read(const uint8_t*& aPointer,size_t& aLength) = 0;
    /** Return whether the end of the stream has been reached. */
    virtual bool EndOfStream() const = 0;
    /** Seek to the specified position. */
    virtual void Seek(int64_t aPosition) = 0;
    /** Return the current position. */
    virtual int64_t Position() = 0;
    /** Return the number of bytes in the stream. */
    virtual int64_t Length() = 0;
    /** Return the file name or URI associated with the stream if any. Return the empty string is there is no file name or URI. */
    virtual std::string Name() { return std::string(); }
    };

/** The output stream interface. */
class MOutputStream
    {
    public:
    /**
    Virtual destructor: strictly unneeded since pointers to MOutputStream are not owned
    and should not be deleted.
    */
    virtual ~MOutputStream() { }
    /** Writes aBytes bytes from aBuffer to the stream. */
    virtual void Write(const uint8_t* aBuffer,size_t aBytes) = 0;
    /** Writes a null-terminated string to the stream. Does not write the final null. */
    void WriteString(const char* aString) { Write((const uint8_t*)aString,strlen(aString)); }
    void WriteString(const MString& aString);
    void WriteXmlText(const MString& aString);
    };

/** The encoding for reading or writing strings. */
enum class TStreamEncoding
    {
    Utf16,
    Utf8
    };

/** The endianness for data streams. */
enum class TStreamEndianness
    {
    Big,
    Little
    };

/**
The data stream base class, providing nothing but the ability
to set and get endianness and string encoding.
*/
class TDataStream
    {
    public:
    TDataStream():
        iEncoding(TStreamEncoding::Utf8),
        iEndianness(TStreamEndianness::Big) { }
    /** Returns the encoding used for streams. */
    TStreamEncoding Encoding() const
        { return iEncoding; }
    /** Sets the encoding used for streams. */
    void SetEncoding(TStreamEncoding aEncoding)
        { iEncoding = aEncoding; }
    /** Returns the endianness used for streams. */
    TStreamEndianness Endianness() const
        { return iEndianness; }
    /** Sets the endianness used for streams. */
    void SetEndianness(TStreamEndianness aEndianness)
        { iEndianness = aEndianness; }
    protected:
    /** The encoding: UTF-16 or UTF-8. */
    TStreamEncoding iEncoding;
    /** The endianness: big-endian or little-endian. */
    TStreamEndianness iEndianness;
    };

/**
A data output stream. It writes integers, strings and blocks of
data to a data sink provided by a class derived from MOutputStream.
*/
class TDataOutputStream: public TDataStream
    {
    public:
    /** Creates a data output stream to write to aOutputStream. */
    TDataOutputStream(MOutputStream& aOutputStream):
        iOutputStream(aOutputStream) {}
    void WriteUint8(uint8_t aValue);
    void WriteUint16(uint16_t aValue);
    void WriteUint32(uint32_t aValue);
    void WriteUint(uint32_t aValue,int32_t aSize);
    void WriteUint(uint64_t aValue);
    void WriteInt(int64_t aValue);
    void WriteFloat(float aValue);
    void WriteDouble(double aValue);
    void WriteNullTerminatedString(const MString& aString);
    void WriteUtf8StringWithLength(const MString& aString);
    void WriteBytes(const uint8_t* aBuffer,size_t aBytes);
    void WriteNullTerminatedUtf8String(const MString& aString);
    void WriteNullTerminatedUtf16String(const MString& aString);
    void WriteString(const MString& aString);
    void WriteXmlText(const MString& aString);
    /** Writes a null-terminated 8-bit string. */
    void WriteString(const char* aString) { return WriteBytes((const uint8_t*)aString,strlen(aString)); }

    private:
    MOutputStream& iOutputStream;
    };

/**
A data input stream. It reads integers, strings and blocks of data from
a data source provided by a class derived from MInputStream.
*/
class TDataInputStream: public TDataStream
    {
    public:
    /** Construct a data input stream, specifying the data source. */
    TDataInputStream(MInputStream& aInputStream):
        iInputStream(&aInputStream)
        {
        }

    /** Sets the data source. */
    void Set(MInputStream& aInputStream)
        {
        iInputStream = &aInputStream;
        iData = nullptr;
        iDataBytes = 0;
        iDataPosition = 0;
        iDataStart = nullptr;
        }

    void Seek(int64_t aPosition);
    /** Returns the current position as a byte offset from the start of the stream. */
    int64_t Position() const { return iDataPosition + (int64_t)(iData - iDataStart); }
    /** Returns true if this stream is at the end of the data. */
    bool EndOfData() const { return !iDataBytes && iInputStream->EndOfStream(); }
    /** Reads an 8-bit unsigned integer. */
    uint8_t ReadUint8()
        {
        if (iDataBytes >= 1)
            {
            iDataBytes--;
            return *iData++;
            }
        return ReadUint8Helper();
        }
    uint16_t ReadUint16();
    uint32_t ReadUint32();
    /** Reads a 16-bit unsigned integer in big-endian form. */
    uint16_t ReadUint16BigEndian()
        {
        if (iDataBytes >= 2)
            {
            iDataBytes -= 2;
            iData += 2;
            return uint16_t(iData[-2] << 8 | iData[-1]);
            }
        return ReadUint16BigEndianHelper();
        }
    /** Reads a 32-bit unsigned integer in big-endian form. */
    uint32_t ReadUint32BigEndian()
        {
        if (iDataBytes >= 4)
            {
            iDataBytes -= 4;
            iData += 4;
            return uint32_t((uint32_t)iData[-4] << 24 | (uint32_t)iData[-3] << 16 | iData[-2] << 8 | iData[-1]);
            }
        return ReadUint32BigEndianHelper();
        }
    /** Reads a 40-bit unsigned integer in big-endian form. */
    uint64_t ReadUint40BigEndian()
        {
        if (iDataBytes >= 5)
            {
            iDataBytes -= 5;
            iData += 5;
            return uint64_t((uint64_t)iData[-5] << 32 | (uint64_t)iData[-4] << 24 | (uint64_t)iData[-3] << 16 | (uint64_t)iData[-2] << 8 | iData[-1]);
            }
        return ReadUint40BigEndianHelper();
        }
    /** Reads a file position: that is, an unsigned integer stored in the number of bytes returned by FilePosBytes. */
    virtual int64_t ReadFilePos() { return ReadUint32BigEndian(); }
    /** Reads a file position combined with a degree square code: that is, an unsigned integer stored in two more bytes than FilePosBytes. */
    virtual int64_t ReadFilePosWithDegreeSquare() { return ReadUint48BigEndian(); }
    /** A virtual function to return the number of bytes storing a file position. The base class returns 4. */
    virtual int32_t FilePosBytes() const { return 4; }
    uint32_t ReadUintOfSize(int32_t aSize);
    uint64_t ReadUint();
    int64_t ReadInt();
    uint32_t ReadUintMax32();
    int32_t ReadIntMax32();
    float ReadFloatFP();
    double ReadDoubleFP();
    int32_t ReadFloatRounded();
    int32_t ReadDoubleRounded();
    void ReadLine(uint8_t* aBuffer,size_t aMaxBytes,size_t& aActualBytes);
    void ReadNullTerminatedBytes(const uint8_t*& aBuffer,size_t& aLength,bool& aNullFound);
    void ReadBytes(uint8_t* aBuffer,size_t aMaxBytes,size_t& aActualBytes);
    CString ReadNullTerminatedString();
    CString ReadUtf8StringWithLength();

    /**
    Reads a string preceded by its length. The length is a single byte for lengths 0...254.
    Greater lengths are encoded as the byte value 255 followed by a four-byte length.
    The current encoding and endianness are used. If aBytesRead is non-null the number of
    bytes read from the stream is returned there.
    */
    CString ReadString(size_t* aBytesRead = 0)
        { if (iEncoding == TStreamEncoding::Utf8) return ReadUtf8String(aBytesRead); else return ReadUtf16String(aBytesRead); }
    std::string ReadUtf8StringToStdString();
    CString ReadUtf8String(size_t* aBytesRead = 0);
    CString ReadUtf16String(size_t* aBytesRead = 0);
    void Skip(int64_t aBytes);

    /**
    Reads the next aBytes bytes, returning a pointer to them, or return nullptr if
    fewer than that number of bytes is cached.
    */
    const uint8_t* Read(size_t aBytes)
        {
        if (iDataBytes >= aBytes)
            {
            iData += aBytes;
            iDataBytes -= aBytes;
            return iData - aBytes;
            }
        return nullptr;
        }

    protected:
    /** Reads an unsigned big-endian 48-bit number. */
    uint64_t ReadUint48BigEndian()
        {
        if (iDataBytes >= 6)
            {
            iDataBytes -= 6;
            iData += 6;
            return uint64_t((uint64_t)iData[-6] << 40 | (uint64_t)iData[-5] << 32 | (uint64_t)iData[-4] << 24 | (uint64_t)iData[-3] << 16 | (uint64_t)iData[-2] << 8 | iData[-1]);
            }
        return ReadUint48BigEndianHelper();
        }
    /** Reads an unsigned big-endian 56-bit number. */
    uint64_t ReadUint56BigEndian()
        {
        if (iDataBytes >= 7)
            {
            iDataBytes -= 7;
            iData += 7;
            return uint64_t((uint64_t)iData[-7] << 48 | (uint64_t)iData[-6] << 40 | (uint64_t)iData[-5] << 32 |
                            (uint64_t)iData[-4] << 24 | (uint64_t)iData[-3] << 16 | (uint64_t)iData[-2] << 8 | iData[-1]);
            }
        return ReadUint56BigEndianHelper();
        }

    private:
    uint8_t ReadUint8Helper();
    uint16_t ReadUint16BigEndianHelper();
    uint32_t ReadUint32BigEndianHelper();
    uint64_t ReadUint40BigEndianHelper();
    uint64_t ReadUint48BigEndianHelper();
    uint64_t ReadUint56BigEndianHelper();
    void ReadAdditionalBytes(size_t aBytesRequired);
    inline void GetFloatComponents(bool& aSign,int32_t& aRawValue,int& aShift);
    inline void GetDoubleComponents(bool& aSign,int32_t& aRawValue,int& aShift);
    inline void ApplyShift(int32_t& aValue,int aShift,int aLowerBound,int aUpperBound) const;
    void GetUtf8String(CString& aString,size_t& aStringBytes,bool& aEndFound,size_t& aIncompleteSequenceBytes);
    void GetUtf16String(CString& aString,size_t& aStringBytes,bool& aEndFound,size_t& aIncompleteSequenceBytes);
    void ReadData()
        {
        iDataPosition = iInputStream->Position();
        iInputStream->Read(iDataStart,iDataBytes);
        iData = iDataStart;
        }

    /** The data source. */
    MInputStream* iInputStream;
    /**
    Internal buffer for reading ints and floats, and for holding incomplete UTF
    sequences
    */
    uint8_t iBuffer[8] = { };
    /** The current data pointer. */
    const uint8_t* iData = nullptr;
    /** The number of data bytes remaining. */
    size_t iDataBytes = 0;
    /** The position of iDataStart within the whole of the data. */
    int64_t iDataPosition = 0;
    /** Start of data returned by the last call to MInputStream::Read. */
    const uint8_t* iDataStart = nullptr;
    };

/** An input stream for a contiguous piece of memory. */
class TMemoryInputStream: public MInputStream
    {
    public:
    /** Creates a memory input stream to read from data of aLength bytes starting at aData. */
    TMemoryInputStream(const uint8_t* aData,size_t aLength):
        iData(aData),
        iLength(aLength)
        {
        }

    /** Creates a memory input stream to read from a std::string. */
    explicit TMemoryInputStream(const std::string& aString):
        iData((const uint8_t*)aString.data()),
        iLength(aString.length())
        {
        }

    /** Resets this memory input stream to read from data of aLength bytes starting at aData. */
    void Set(const uint8_t* aData,size_t aLength)
        {
        iData = aData;
        iLength = aLength;
        iPosition = 0;
        }

    // from MInputStream
    void Read(const uint8_t*& aPointer,size_t& aLength) override;
    bool EndOfStream() const override { return iPosition >= iLength; }
    void Seek(int64_t aPosition) override;
    int64_t Position() override
        {
        return iPosition;
        }
    int64_t Length() override
        {
        return iLength;
        }

    private:
    const uint8_t* iData = nullptr;
    int64_t iLength = 0;
    int64_t iPosition = 0;
    };

/** A file input class for reading binary data from file which may be greater than 4Gb in size. */
#ifdef CARTOTYPE_LOW_LEVEL_FILE_IO
class CBinaryInputFile
    {
    public:
    CBinaryInputFile():
        iFile(-1)
        {
        }

    /** Opens a file. */
    TResult Open(const char* aFileName);

    /** Opens standard input. */
    void OpenStandardInput()
        {
        iFile = 0;
        }

    ~CBinaryInputFile()
        {
        if (iFile != -1)
#if (defined(_MSC_VER))
            _close(iFile);
#else
            close(iFile);
#endif
        }

    /** Seeks to a byte position aOffset in the file; aOrigin is the same as for fseek(). */
    TResult Seek(int64_t aOffset,int aOrigin)
        {
#if (defined(_MSC_VER) && !defined(_WIN32_WCE))
        int64_t pos = _lseeki64(iFile,aOffset,aOrigin);
#elif defined(__APPLE__)
        int64_t pos = lseek(iFile,aOffset,aOrigin);
#elif (defined(_POSIX_VERSION) || defined(__MINGW32__))
        int64_t pos = lseek64(iFile,aOffset,aOrigin);
#else
        int64_t pos = -1;
        if (aOffset >= INT32_MIN && aOffset <= INT32_MAX)
            pos = lseek(iFile,long(aOffset),aOrigin);
#endif
        return pos > -1 ? KErrorNone : KErrorIo;
        }

    /** Returns the current byte position in the file. */
    int64_t Tell() const
        {
#if (defined(_MSC_VER) && !defined(_WIN32_WCE))
        return _telli64(iFile);
#elif defined (__APPLE__)
        return lseek(iFile,0,SEEK_CUR);
#elif (defined(_POSIX_VERSION) || defined(__MINGW32__))
        return lseek64(iFile,0,SEEK_CUR);
#else
        return lseek(iFile,0,SEEK_CUR);
#endif
        }

    /** Reads up to aBufferSize bytes into aBuffer and returns the number of bytes actually read. */
    size_t Read(uint8_t* aBuffer,size_t aBufferSize)
        {
#if (defined(_MSC_VER))
        return _read(iFile,aBuffer,(unsigned int)aBufferSize);
#else
        return read(iFile,aBuffer,aBufferSize);
#endif
        }

    CBinaryInputFile(const CBinaryInputFile&) = delete;
    CBinaryInputFile(CBinaryInputFile&&) = delete;
    void operator=(const CBinaryInputFile&) = delete;
    void operator=(CBinaryInputFile&&) = delete;

    private:
    int iFile;
    };
#else
class CBinaryInputFile
    {
    public:
    CBinaryInputFile() = default;

    /** Opens a file. */
    TResult Open(const char* aFileName);

    /** Opens standard input. */
    void OpenStandardInput()
        {
        iFile = stdin;
        }

    ~CBinaryInputFile()
        {
        if (iFile)
            fclose(iFile);
        }

    /** Seeks to a byte position aOffset in the file; aOrigin is the same as for fseek(). */
    TResult Seek(int64_t aOffset,int aOrigin)
        {
        int e;
#if (defined(_MSC_VER) && !defined(_WIN32_WCE))
        e = _fseeki64(iFile,aOffset,aOrigin);
#elif defined(__APPLE__)
        e = fseeko(iFile,aOffset,aOrigin);
#elif ((defined(_POSIX_VERSION) || defined(__MINGW32__)) && !defined(ANDROID) && !defined(__ANDROID__))
        e = fseeko64(iFile,aOffset,aOrigin);
#else
        if (aOffset < INT32_MIN)
            return KErrorIo;
        else if (aOffset > INT32_MAX)
            return KErrorIo;
        e = fseek(iFile,long(aOffset),aOrigin);
#endif
        return e ? KErrorIo : KErrorNone;
        }

    /** Returns the current byte position in the file. */
    int64_t Tell() const
        {
#if (defined(_MSC_VER) && !defined(_WIN32_WCE))
        return _ftelli64(iFile);
#elif defined (__APPLE__)
        return ftello(iFile);
#elif ((defined(_POSIX_VERSION) || defined(__MINGW32__)) && !defined(ANDROID) && !defined(__ANDROID__))
        return ftello64(iFile);
#else
        return ftell(iFile);
#endif
        }

    /** Reads up to aBufferSize bytes into aBuffer and returns the number of bytes actually read. */
    size_t Read(uint8_t* aBuffer,size_t aBufferSize)
        {
        return fread(aBuffer,1,aBufferSize,iFile);
        }

    CBinaryInputFile(const CBinaryInputFile&) = delete;
    CBinaryInputFile(CBinaryInputFile&&) = delete;
    void operator=(const CBinaryInputFile&) = delete;
    void operator=(CBinaryInputFile&&) = delete;

    private:
    FILE* iFile = nullptr;
    };
#endif

/**
Input stream for a file. The user of this stream determines the buffer size that
is used to read from the file.
*/
class CFileInputStream: public MInputStream
    {
    public:
    /** Creates a CFileInputStream to read from the file aFileName. Returns the result in aError. */
    static std::unique_ptr<CFileInputStream> New(TResult& aError,const std::string& aFileName,size_t aBufferSize = KDefaultBufferSize,size_t aMaxBuffers = KDefaultMaxBuffers);
    /** Creates a CFileInputStream to read from the file aFileName. Throws an exception if the file cannot be opened. */
    CFileInputStream(const std::string& aFileName,size_t aBufferSize = KDefaultBufferSize,size_t aMaxBuffers = KDefaultMaxBuffers);

    /** Returns a copy of this CFileInputStream. */
    virtual std::unique_ptr<CFileInputStream> Copy();

    // from MInputStream
    void Read(const uint8_t*& aPointer,size_t& aLength) override;
    bool EndOfStream() const override;
    void Seek(int64_t aPosition) override;
    int64_t Position() override
        {
        return iLogicalPosition;
        }
    int64_t Length() override
        {
        return iLength;
        }
    std::string Name() override { return iName; }

    /** The default size of each buffer in bytes. */
    static constexpr size_t KDefaultBufferSize = 64 * 1024;

    /** The default maximum number of buffers. */
    static constexpr size_t KDefaultMaxBuffers = 32;

#ifdef COLLECT_STATISTICS
    void ResetStatistics()
        {
        iSeekCount = 0;
        iReadCount = 0;
        }
    int32_t SeekCount() const
        { return iSeekCount; }
    int32_t ReadCount() const
        { return iReadCount; }
#endif

    CFileInputStream(const CFileInputStream&) = delete;
    CFileInputStream(CFileInputStream&&) = delete;
    void operator=(const CFileInputStream&) = delete;
    void operator=(CFileInputStream&&) = delete;

    protected:
    /** A buffer storing some data from the file. */
    class CBuffer
        {
        public:
        CBuffer(): iPosition(-1), iSize(0), iData(0) { }
        ~CBuffer()
            { delete[] iData; }

        /** The byte offset in the file of the data in this buffer. */
        int64_t iPosition;
        /** The number of bytes stored in this buffer. */
        size_t iSize;
        /** A pointer to the data stored in this buffer. */
        uint8_t* iData;
        };

    /** Override this function to read a buffer at a certain position in the file. */
    virtual void ReadBuffer(CBuffer& aBuffer,int64_t aPos);

    /** The file. */
    CBinaryInputFile iFile;
    /** A type for the data cache. */
    using CBufferList = CList<CBuffer>;
    /** Cached data from the file. */
    CBufferList iBuffers;
    /** The size of a buffer in bytes. */
    size_t iBufferSize = KDefaultBufferSize;
    /** The physical position in the file. */
    int64_t iPositionInFile = 0;
    /** The position in the file from the user's point of view. */
    int64_t iLogicalPosition = 0;
    /** the length of the file in bytes. */
    int64_t iLength = 0;
    /** The name of the file. */
    std::string iName;
#ifdef COLLECT_STATISTICS
    int32_t iSeekCount = 0;
    int32_t iReadCount = 0;
#endif
    };

/**
A simple file input stream that does not use seek when reading sequentially.
If the first part of the filename, before any extensions, is '-', it reads from standard input.
*/
class CSimpleFileInputStream: public MInputStream
    {
    public:
    /** Creates a CSimpleFileInputStream to read from the file aFileName. Returns the result in aError. */
    static std::unique_ptr<CSimpleFileInputStream> New(TResult& aError,const std::string& aFileName,size_t aBufferSize = 64 * 1024);
    /** Creates a CSimpleFileInputStream to read from the file aFileName. Throws an exception if the file cannot be opened. */
    CSimpleFileInputStream(const std::string& aFileName,size_t aBufferSize = 64 * 1024);

    void Read(const uint8_t*& aPointer,size_t& aLength) override;
    bool EndOfStream() const override;
    void Seek(int64_t aPosition) override;
    int64_t Position() override;
    int64_t Length() override;
    std::string Name() override { return iName; }

    private:
    CBinaryInputFile iFile;
    std::vector<uint8_t> iBuffer;
    std::string iName;
    int64_t iLength = -1;
    bool iStandardInput = false;
    bool iEndOfStream = false;
    };

/**
An output stream to write to a file that is already open for writing.
The destructor does not close the file.
*/
class COpenFileOutputStream: public MOutputStream
    {
    public:
    /**
    Creates a file output stream from a file descriptor (the value returned by fopen).
    The file must already have been opened for writing.
    */
    COpenFileOutputStream(void* aFile): iFD(aFile) { }
    void Write(const uint8_t* aBuffer,size_t aBytes) override;
    /** Returns the current position in the file as a byte offset relative to the start of the file. */
    int64_t Position();

    COpenFileOutputStream(const COpenFileOutputStream&) = delete;
    COpenFileOutputStream(COpenFileOutputStream&&) = delete;
    void operator=(const COpenFileOutputStream&) = delete;
    void operator=(COpenFileOutputStream&&) = delete;

    protected:
    COpenFileOutputStream(): iFD(nullptr) { }
    /** The file pointer. The actual type is FILE*. */
    void* iFD;
    };

/**
An output stream to write to a file. The New function opens the file and
the destructor closes it.
*/
class CFileOutputStream: public COpenFileOutputStream
    {
    public:
    /** Creates a CFileOutputStream to write to the file aFileName. Returns the result in aError. */
    static std::unique_ptr<CFileOutputStream> New(TResult& aError,const std::string& aFileName);
    /** Creates a CFileOutputStream to write to the file aFileName. Throws an exception if the file cannot be opened. */
    CFileOutputStream(const std::string& aFileName);
    ~CFileOutputStream();
    };

/**
Output stream for a buffer in memory. The caller specifies the initial size of the buffer,
which is automatically enlarged when necessary.
*/
class CMemoryOutputStream: public MOutputStream
    {
    public:
    /** Creates a CMemoryOutputStream object to write to a buffer owned by it, optionally specifying an initial buffer size in bytes. */
    CMemoryOutputStream(size_t aInitialBufferSize = 0) { iBuffer.reserve(aInitialBufferSize); }
    void Write(const uint8_t* aBuffer,size_t aBytes) override;

    /** Return a pointer to the memory buffer. */
    const uint8_t* Data() const { return iBuffer.data(); }
    /** Take ownership of the data. */
    std::vector<uint8_t> RemoveData() { std::vector<uint8_t> a; std::swap(a,iBuffer); return a; }
    /** Return the number of bytes written. */
    size_t Length() const { return iBuffer.size(); }

    private:
    std::vector<uint8_t> iBuffer;
    };

/**
An fseek-compatible function for moving to a position in a file, specifying
it using a 64-bit signed integer.
*/
inline int FileSeek(FILE* aFile,int64_t aOffset,int aOrigin)
    {
#if (defined(_MSC_VER) && !defined(_WIN32_WCE))
    return _fseeki64(aFile,aOffset,aOrigin);
#elif defined(__APPLE__)
    return fseeko(aFile,aOffset,aOrigin);
#elif ((defined(_POSIX_VERSION) || defined(__MINGW32__)) && !defined(ANDROID) && !defined(__ANDROID__))
    return fseeko64(aFile,aOffset,aOrigin);
#else
    if (aOffset < INT32_MIN)
        return -1;
    else if (aOffset > INT32_MAX)
        return -1;
    return fseek(aFile,long(aOffset),aOrigin);
#endif
    }

/**
An ftell-compatible function for getting the current position in a file,
returning a 64-bit signed integer.
*/
inline int64_t FileTell(FILE* aFile)
    {
#if (defined(_MSC_VER) && !defined(_WIN32_WCE))
    return _ftelli64(aFile);
#elif defined (__APPLE__)
    return ftello(aFile);
#elif ((defined(_POSIX_VERSION) || defined(__MINGW32__)) && !defined(ANDROID) && !defined(__ANDROID__))
    return ftello64(aFile);
#else
    return ftell(aFile);
#endif
    }

} // namespace CartoType

#endif
