/*
cartotype_expression.h
Copyright (C) 2009-2019 CartoType Ltd.
See www.cartotype.com for more information.
*/

#ifndef CARTOTYPE_EXPRESSION_H__
#define CARTOTYPE_EXPRESSION_H__

#include <cartotype_base.h>
#include <cartotype_errors.h>
#include <cartotype_string.h>
#include <cartotype_road_type.h>

#include <float.h>

namespace CartoType
{

#ifndef NAN
static const double infinity { CartoType::CT_DBL_MAX * CartoType::CT_DBL_MAX };
static const double NAN { infinity * 0.0 };
#endif

/** Construct a 15-bit type code (to go in in bits 17...31) from 3 lower-case letters. */
inline int32_t TypeCode(char A,char B,char C) { return (((A-'a') << 27) | ((B-'a') << 22) | ((C-'a') << 17)); }

/**
A class for expression values or the values of variables used in expressions.
If iString is non-null it is a string.
If iString is null it is a number unless iNumber is NAN (not a number),
in which case it is undefined.
*/
class TExpressionValue
    {
    public:
    TExpressionValue():
        iNumber(NAN)
        {
        }

    TExpressionValue(double aNumber):
        iNumber(aNumber)
        {
        }

    TExpressionValue(const MString& aString):
        iNumber(NAN),
        iString(aString)
        {
        if (iString.Length() == 0)
            iString = TText(); // treat empty strings as null values so that '' compares equal to an undefined variable
        else
            iNumber = ValueOfStringLiteral(iString,true);
        }

    operator double() const { return iNumber; }
    int ToInt() const
        {
        return IsNan() ? 0 : int(iNumber);
        }

    // Overload comparison operators for string values.
    int operator<(const TExpressionValue& aOther) const
        {
        if (iString.Text() && aOther.iString.Text()) return iString.Compare(aOther.iString) < 0;
        return iNumber < aOther.iNumber;
        }
    int operator<=(const TExpressionValue& aOther) const
        {
        if (iString.Text() && aOther.iString.Text()) return iString.Compare(aOther.iString) <= 0;
        return iNumber <= aOther.iNumber;
        }
    int operator==(const TExpressionValue& aOther) const
        {
        if (iString.Text() && aOther.iString.Text()) return iString == aOther.iString;
        return iNumber == aOther.iNumber ||
               (iNumber != iNumber && aOther.iNumber != aOther.iNumber); // if both numbers are NAN, they compare equal: both are undefined
        }
    int operator!=(const TExpressionValue& aOther) const
        {
        return int(!(*this == aOther));
        }
    int operator>=(const TExpressionValue& aOther) const
        {
        if (iString.Text() && aOther.iString.Text()) return iString.Compare(aOther.iString) >= 0;
        return iNumber >= aOther.iNumber;
        }
    int operator>(const TExpressionValue& aOther) const
        {
        if (iString.Text() && aOther.iString.Text()) return iString.Compare(aOther.iString) > 0;
        return iNumber > aOther.iNumber;
        }

    /** Case-folded, accent-folded or fuzzy comparison; reverts to equality for numbers. */
    int Compare(const TExpressionValue& aOther,TStringMatchMethod aMethod) const
        {
        if (iString.Text() && aOther.iString.Text())
            return iString.Compare(aOther.iString,aMethod) == 0;
        return iNumber == aOther.iNumber ||
            (iNumber != iNumber && aOther.iNumber != aOther.iNumber); // if both numbers are NAN, they compare equal: both are undefined
        }

    /** Wild-card match: aOther can contain wild cards; reverts to equality for numbers. */
    int WildMatch(const TExpressionValue& aOther) const
        {
        if (iString.Text() && aOther.iString.Text())
            return iString.WildMatch(aOther.iString);
        return iNumber == aOther.iNumber ||
            (iNumber != iNumber && aOther.iNumber != aOther.iNumber); // if both numbers are NAN, they compare equal: both are undefined
        }

    bool IsTrue() const
        {
        return iString.Length() > 0 || (iNumber != 0 && iNumber == iNumber);
        }

    bool IsNan() const
        {
        return iNumber != iNumber;
        }

    const MString* StringValue() const { return iString.Text() ? &iString : nullptr; }
    static double ValueOfStringLiteral(const MString& aText,bool aTolerateSyntaxError);

    private:
    double iNumber;
    TText iString;
    };

enum class TExpressionOpType
    {
    Number,
    String,
    Variable,
    
    UnaryMinus,
    BitwiseNot,
    LogicalNot,

    Multiply,
    Divide,
    Mod,

    Plus,
    Minus,

    LeftShift,
    RightShift,

    LessThan,
    LessThanOrEqual,
    Equal,
    NotEqual,
    GreaterThanOrEqual,
    GreaterThan,

    BitwiseAnd,
    BitwiseXor,
    BitwiseOr,

    LogicalAnd,
    LogicalOr,

    InSet,
    NotInSet,
    InRange,
    NotInRange,

    // A range set is a set of ranges, each a pair of values: lower and upper limit.
    InRangeSet,
    NotInRangeSet,

    EqualIgnoreCase,
    EqualIgnoreAccents,
    EqualFuzzy,
    EqualWild,

    Concat,
    Subscript
    };

/**
An expression operator is part of an expression encoded in reverse polish notation (RPN).
Numbers and strings cause values to be pushed to a stack. Other operators are evaluated
using one or more stack values, and the result is pushed to the stack.
*/
class CExpressionOp
    {
    public:
    CExpressionOp():
        iType(TExpressionOpType::Number),
        iNumber(0)
        {
        }

    explicit CExpressionOp(TExpressionOpType aOpType):
        iType(aOpType),
        iNumber(0)
        {
        }

    explicit CExpressionOp(TExpressionOpType aOpType,const MString& aString,double aNumber = 0):
        iType(aOpType),
        iNumber(aNumber),
        iString(aString)
        {
        }

    explicit CExpressionOp(double aNumber):
        iType(TExpressionOpType::Number),
        iNumber(aNumber)
        {
        }

    /** The operator type. */
    TExpressionOpType iType;

    /**
    For TExpressionOpType::Number, the number;
    for TExpressionOpType::Variable, a variable index if >= 0;
    otherwise ignored.
    */
    double iNumber;

    /**
    For EStringOp, the string;
    for EVariableOp, the variable name.
    */
    CString iString;
    };

class CRpnExpression
    {
    public:
    void Append(TExpressionOpType aOpType)
        {
        iExp.emplace_back(aOpType);
        }
    void Append(TExpressionOpType aOpType,const MString& aString,double aNumber = 0)
        {
        iExp.emplace_back(aOpType,aString,aNumber);
        }
    void Append(const TExpressionValue& aValue)
        {
        iExp.emplace_back(aValue);
        if (aValue.StringValue())
            {
            iExp.back().iType = TExpressionOpType::String;
            iExp.back().iString = *aValue.StringValue();
            }
        }

    std::vector<CExpressionOp> iExp;
    };

class MVariableDictionary
    {
    public:
    virtual ~MVariableDictionary() { }

    /** Get the value of a variable from its name. */
    virtual bool Find(const MString& aName,TExpressionValue& aValue,const size_t* aSubscriptArray,size_t aSubscriptCount) const = 0;
    /** Get the value of a variable from its index. */
    virtual bool Find(int32_t aIndex,TExpressionValue& aValue,const size_t* aSubscriptArray,size_t aSubscriptCount) const = 0;
    };

/** A simple implementation of a variable dictionary. */
class CVariableDictionary: public MVariableDictionary
    {
    public:
    bool Find(const MString& aKey,TExpressionValue& aValue,const size_t* /*aSubscriptArray*/,size_t /*aSubscriptCount*/) const override
        {
        auto p = iDictionary.find(aKey);
        if (p != iDictionary.end())
            {
            aValue = p->second;
            return true;
            }
        else
            return false;
        }
    bool Find(int32_t /*aIndex*/,TExpressionValue& /*aValue*/,const size_t* /*aSubscriptArray*/,size_t /*aSubscriptCount*/) const override
        {
        return false;
        }
    void Set(const CString& aVariableName,const CString& aValue)
        {
        if (aValue.Length())
            iDictionary[aVariableName] = aValue;
        else
            iDictionary.erase(aVariableName);
        }
    template<typename Functor> void Apply(Functor& aFunctor) { for (auto& p : iDictionary) { aFunctor(p.first,p.second); } }

    private:
    CStringDictionary iDictionary;
    };

/**
An evaluator for simple expressions.
String expressions can be compiled into reverse-polish form for efficient storage.
*/
class TExpressionEvaluator
    {
    public:
    TExpressionEvaluator(const MVariableDictionary* aVariableDictionary = nullptr);
    void Evaluate(const MString& aExpression,double* aNumericResult,MString* aStringResult,bool* aLogicalResult);
    void Evaluate(const CRpnExpression& aExpression,double* aNumericResult,MString* aStringResult,bool* aLogicalResult);
    bool EvaluateLogical(const MString& aExpression);
    bool EvaluateLogical(const CRpnExpression& aExpression);
    void Compile(const MString& aExpression,CRpnExpression& aDestExpression);

    private:
    const MVariableDictionary* iVariableDictionary;
    };

TRoadType RoadTypeFromKeyWord(const MString& aKeyWord);

}

#endif
