/*
cartotype_style_sheet_data.h
Copyright (C) 2016-2020 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_STYLE_SHEET_DATA_H__
#define CARTOTYPE_STYLE_SHEET_DATA_H__

#include <cartotype_stream.h>
#include <string>

namespace CartoType
{

/** Style sheet data stored in XML format as text or in a file. */
class CStyleSheetData
    {
    public:
    CStyleSheetData() = default;
    /** Creates a CStyleSheetData object by copying another one. */
    CStyleSheetData(const CStyleSheetData& aOther) = default;
    /** Creates a CStyleSheetData object by moving data from another one. */
    CStyleSheetData(CStyleSheetData&& aOther) = default;
    /** The assignment operator. */
    CStyleSheetData& operator=(const CStyleSheetData& aOther) = default;
    /** The move assignment operator. */
    CStyleSheetData& operator=(CStyleSheetData&& aOther) = default;
    /** Creates a CStyleSheetData object representing data from a named file. */
    explicit CStyleSheetData(const char* aFileName);
    /** Creates a CStyleSheetData object representing data stored in memory. Copies the data. Throws KErrorInvalidArgument if aData is null or aLength is zero. */
    CStyleSheetData(const uint8_t* aData,size_t aLength);
    /** Reloads the data in a CStyleSheetData object from the file if any. */
    void Reload();
    /** Creates a memory input stream representing the style sheet data. */
    TMemoryInputStream Stream() const { return TMemoryInputStream((const uint8_t*)iText.data(),iText.length()); }
    /** Returns the filename associated with this data, or the empty string if there is none. */
    const std::string& FileName() const { return iFileName; }
    /** Returns the text of this style sheet data. */
    const std::string& Text() const { return iText; }

    private:
    std::string iText;
    std::string iFileName;
    };

/** A set of style sheet data that may consist of more than one style sheet. */
using CStyleSheetDataArray = std::vector<CStyleSheetData>;

}

#endif // CARTOTYPE_STYLE_SHEET_DATA_H__
