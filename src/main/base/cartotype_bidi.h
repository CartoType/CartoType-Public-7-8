/*
cartotype_bidi.h
Copyright (C) 2008-2020 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_BIDI_H__
#define CARTOTYPE_BIDI_H__

#include <cartotype_char.h>
#include <cartotype_errors.h>

namespace CartoType
{

/** The override status during bidirectional reordering. */
enum class TBidiOverride
    {
    Neutral,
    LeftToRight,
    RightToLeft
    };

/** A run of text in a certain birectional type. */
class TBidiRun
    {
    public:
    /** The length of the run in bytes. */
    int32_t iLength { 0 };
    /** The bidirectional type of the run. */
    TBidiType iType { TBidiType::ON };
    /** The level of the run as usded in the Unicode Bidirectional Algorithm. */
    int32_t iLevel { 0 };
    };

/** The paragraph direction used during bidirectional reordering. */
enum class TBidiParDir
    {
    /**
    Set the paragraph direction from the first strongly directional character.
    If there are none, use left-to-right.
    */
    LeftToRightPreferred,
    /**
    Set the paragraph direction from the first strongly directional character.
    If there are none, use right-to-left.
    */
    RightToLeftPreferred,
    /** Set the paragraph direction to left-to-right whatever its content. */
    LeftToRightForced,
    /** Set the paragraph direction to right-to-left whatever its content. */
    RightToLeftForced
    };

/**
An engine for doing bidirectional reordering and contextual shaping, and storing state
when reordering and shaping a series of lines.
*/
class CBidiEngine
    {
    public:
    CBidiEngine();

    void Order(uint16_t* aText,size_t aLength,size_t& aNewLength,TBidiParDir aParDir,bool aParStart,int32_t* aUserData = nullptr);
    static int32_t ShapeArabic(uint16_t* aText,size_t aLength);
    
    /** Return the resolved direction; meaningful only after Order has been called at least once. */
    bool RightToLeft() { return (iStack[iStackLevel].iLevel & 1) != 0; }

    private:
    void operator=(const CBidiEngine&) = delete;
    void operator=(CBidiEngine&&) = delete;
    CBidiEngine(const CBidiEngine&) = delete;
    CBidiEngine(CBidiEngine&&) = delete;

    void Push(bool aRightToLeft,TBidiOverride aOverride);
    static void ShapeAndMirror(uint16_t* aText,size_t& aNewLength,bool aHaveArabic,int32_t* aUserData,TBidiRun* aRunStart,TBidiRun* aRunEnd);
    
    static constexpr int KRunArraySize = 8;
    static constexpr int KLevels = 63; // legal levels are 0...62 

    class TStackItem
        {
        public:
        uint8_t iLevel;
        uint8_t iOverride;
        };

    TStackItem iStack[KLevels];
    int32_t iStackLevel { 0 };
    };

}

#endif
