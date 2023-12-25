// products.hpp
//
// Purpose: 1. Defines Bond and Interest Rate Swap products(IRSWAP).
// 2. Update some codes for efficiency and add some comments.
// 
// @author Breman Thuraisingham
// @coauthor Yuanting Li
// @version 2.0 2023/12/22 

#ifndef PRODUCTS_HPP
#define PRODUCTS_HPP

#include <iostream>
#include <string>

#include "boost/date_time/gregorian/gregorian.hpp"

using namespace std;
using namespace boost::gregorian;

// Product types in the trading system
enum ProductType { IRSWAP, BOND };

/**
 * Base class for a product.
 */
class Product
{

public:
	// Default ctor
	Product();

	// ctor for a prduct
	Product(string _productId, ProductType _productType);

	// dtor(we need to use virtual in base product class!)
	virtual ~Product() = default;

	// Get the product identifier
	const string& GetProductId() const;

	// Ge the product type
	ProductType GetProductType() const;

private:
	// Product ID
	string productId;

	// Product Type
	ProductType productType;

};

// Bond ID types: CUSIP(numbers for US bonds identification), ISIN(numbers for international bonds identification)
enum BondIdType { CUSIP, ISIN };

/**
 * Bond product class: Derived from Product base class
 */
class Bond : public Product
{

public:
	// Default ctor for a bond
	Bond();

	// ctor for a bond
	Bond(string _productId, BondIdType _bondIdType, string _ticker, float _coupon, date _maturityDate);

	// dtor for a bond
	~Bond();

	// Get the ticker
	const string& GetTicker() const;

	// Get the coupon
	float GetCoupon() const;

	// Get the maturity date
	const date& GetMaturityDate() const;

	// Get the bond identifier type
	BondIdType GetBondIdType() const;

	// Print the bond
	friend ostream& operator<<(ostream& output, const Bond& bond);

private:
	// Product ID(to identify a certain bond)
	string productId;

	// Bond ID Type(CUISP or ININ?)
	BondIdType bondIdType;

	// Ticker
	string ticker;

	// Coupon rate
	float coupon;

	// Maturity Date
	date maturityDate;

};

/**
 * Interest Rate Swap enums
 */

 // Day Count Convention enumeration(30 days in a month/360 days a year, Actual days in a month or year, 360 days a year)
enum DayCountConvention { THIRTY_THREE_SIXTY, ACT_THREE_SIXTY };

//Payment (of bond) Frequency enumeration(quarterly, half a year, or annually)
enum PaymentFrequency { QUARTERLY, SEMI_ANNUAL, ANNUAL };

// Float Index enumeration: Identifies the reference rate index for floating-rate instruments
enum FloatingIndex { LIBOR, EURIBOR };

// Floating Index Tenor enumeration: Defines the tenor (duration) of the floating rate index
enum FloatingIndexTenor { TENOR_1M, TENOR_3M, TENOR_6M, TENOR_12M };

// Currency enumeration: the major currencies used for the bonds
enum Currency { USD, EUR, GBP };

// Swap type enumeration: Defines the types of swaps in interest rate markets
enum SwapType { STANDARD, FORWARD, IMM, MAC, BASIS };

// Swap leg type enumeration: Specifies the leg type in a swap strategy.
enum SwapLegType { OUTRIGHT, CURVE, FLY };

/**
 * Interest Rate Swap product
 */
class IRSwap : public Product
{

public:
	// Default ctor
	IRSwap();

	// ctor for a swap
	IRSwap(string productId, DayCountConvention _fixedLegDayCountConvention, DayCountConvention _floatingLegDayCountConvention, PaymentFrequency _fixedLegPaymentFrequency, FloatingIndex _floatingIndex, FloatingIndexTenor _floatingIndexTenor, date _effectiveDate, date _terminationDate, Currency _currency, int termYears, SwapType _swapType, SwapLegType _swapLegType);

	// dtor
	~IRSwap();

	// Get the fixed leg daycount convention
	DayCountConvention GetFixedLegDayCountConvention() const;

	// Get the floating leg daycount convention
	DayCountConvention GetFloatingLegDayCountConvention() const;

	// Get the payment frequency on the fixed leg
	PaymentFrequency GetFixedLegPaymentFrequency() const;

	// Get the flaotig leg index
	FloatingIndex GetFloatingIndex() const;

	// Get the floating leg index tenor
	FloatingIndexTenor GetFloatingIndexTenor() const;

	// Get the effective date
	const date& GetEffectiveDate() const;

	// Get the termination date
	const date& GetTerminationDate() const;

	// Get the currency
	Currency GetCurrency() const;

	// Get the term in years
	int GetTermYears() const;

	// Get the swap type
	SwapType GetSwapType() const;

	// Get the swap leg type
	SwapLegType GetSwapLegType() const;

	// Print the swap
	friend ostream& operator<<(ostream& output, const IRSwap& swap);

private:
	// Convention for counting days in the fixed leg of the swap
	DayCountConvention fixedLegDayCountConvention;

	// Convention for counting days in the floating leg of the swap
	DayCountConvention floatingLegDayCountConvention;

	// Frequency of payments in the fixed leg of the swap
	PaymentFrequency fixedLegPaymentFrequency;

	// Index used for the floating rate calculation
	FloatingIndex floatingIndex;

	// Tenor of the floating index
	FloatingIndexTenor floatingIndexTenor;

	// Start date of the swap contract
	date effectiveDate;

	// End date of the swap contract
	date terminationDate;

	// Currency in which the swap is denominated
	Currency currency;

	// Duration of the swap in years
	int termYears;

	// Type of the interest rate swap
	SwapType swapType;

	// Type of swap leg (outright, curve, or fly)
	SwapLegType swapLegType;

	// Convert day count convention to string
	string ToString(DayCountConvention dayCountConvention) const;

	// Convert payment frequency to string
	string ToString(PaymentFrequency paymentFrequency) const;

	// Convert floating index to string
	string ToString(FloatingIndex floatingIndex) const;

	// Convert floating index tenor to string
	string ToString(FloatingIndexTenor floatingIndexTenor) const;

	// Convert currency to string
	string ToString(Currency currency) const;

	// Convert swap type to string
	string ToString(SwapType swapType) const;

	// Convert swap leg type to string
	string ToString(SwapLegType swapLegType) const;

};

Product::Product() : Product(0, BOND)
{
}

// More efficient by initializer list
Product::Product(string _productId, ProductType _productType) : productId(_productId), productType(_productType)
{
}

const string& Product::GetProductId() const
{
	return productId;
}

ProductType Product::GetProductType() const
{
	return productType;
}

Bond::Bond() : Product(0, BOND)
{
}

// More efficient by initializer list
Bond::Bond(string _productId, BondIdType _bondIdType, string _ticker, float _coupon, date _maturityDate)
	: Product(_productId, BOND), bondIdType(_bondIdType), ticker(_ticker), coupon(_coupon), maturityDate(_maturityDate)
{
}

Bond::~Bond()
{
}

const string& Bond::GetTicker() const
{
	return ticker;
}

float Bond::GetCoupon() const
{
	return coupon;
}

const date& Bond::GetMaturityDate() const
{
	return maturityDate;
}

BondIdType Bond::GetBondIdType() const
{
	return bondIdType;
}

ostream& operator<<(ostream& output, const Bond& bond)
{
	output << bond.ticker << " " << bond.coupon << " " << bond.GetMaturityDate();
	return output;
}

IRSwap::IRSwap() : Product(0, IRSWAP)
{
}

// More efficient by initializer list
IRSwap::IRSwap(string _productId,
	DayCountConvention _fixedLegDayCountConvention,
	DayCountConvention _floatingLegDayCountConvention,
	PaymentFrequency _fixedLegPaymentFrequency,
	FloatingIndex _floatingIndex,
	FloatingIndexTenor _floatingIndexTenor,
	date _effectiveDate,
	date _terminationDate,
	Currency _currency,
	int _termYears,
	SwapType _swapType,
	SwapLegType _swapLegType)
	: Product(_productId, IRSWAP),
	fixedLegDayCountConvention(_fixedLegDayCountConvention),
	floatingLegDayCountConvention(_floatingLegDayCountConvention),
	fixedLegPaymentFrequency(_fixedLegPaymentFrequency),
	floatingIndex(_floatingIndex),
	floatingIndexTenor(_floatingIndexTenor),
	effectiveDate(_effectiveDate),
	terminationDate(_terminationDate),
	currency(_currency),
	termYears(_termYears),
	swapType(_swapType),
	swapLegType(_swapLegType)
{
}

IRSwap::~IRSwap()
{
}

DayCountConvention IRSwap::GetFixedLegDayCountConvention() const
{
	return fixedLegDayCountConvention;
}

DayCountConvention IRSwap::GetFloatingLegDayCountConvention() const
{
	return floatingLegDayCountConvention;
}

PaymentFrequency IRSwap::GetFixedLegPaymentFrequency() const
{
	return fixedLegPaymentFrequency;
}

FloatingIndex IRSwap::GetFloatingIndex() const
{
	return floatingIndex;
}

FloatingIndexTenor IRSwap::GetFloatingIndexTenor() const
{
	return floatingIndexTenor;
}

const date& IRSwap::GetEffectiveDate() const
{
	return effectiveDate;
}

const date& IRSwap::GetTerminationDate() const
{
	return terminationDate;
}

Currency IRSwap::GetCurrency() const
{
	return currency;
}

int IRSwap::GetTermYears() const
{
	return termYears;
}

SwapType IRSwap::GetSwapType() const
{
	return swapType;
}

SwapLegType IRSwap::GetSwapLegType() const
{
	return swapLegType;
}


ostream& operator<<(ostream& output, const IRSwap& swap)
{
	output << "fixedDayCount:" << swap.ToString(swap.GetFixedLegDayCountConvention()) << " floatingDayCount:" << swap.ToString(swap.GetFloatingLegDayCountConvention()) << " paymentFreq:" << swap.ToString(swap.GetFixedLegPaymentFrequency()) << " " << swap.ToString(swap.GetFloatingIndexTenor()) << swap.ToString(swap.GetFloatingIndex()) << " effective:" << swap.GetEffectiveDate() << " termination:" << swap.GetTerminationDate() << " " << swap.ToString(swap.GetCurrency()) << " " << swap.GetTermYears() << "yrs " << swap.ToString(swap.GetSwapType()) << " " << swap.ToString(swap.GetSwapLegType());
	return output;
}

string IRSwap::ToString(DayCountConvention dayCountConvention) const
{
	switch (dayCountConvention)
	{
	case THIRTY_THREE_SIXTY: return "30/360";
	case ACT_THREE_SIXTY: return "Act/360";
	default: return "";
	}
}

string IRSwap::ToString(PaymentFrequency paymentFrequency) const
{
	switch (paymentFrequency)
	{
	case QUARTERLY: return "Quarterly";
	case SEMI_ANNUAL: return "Semi-Annual";
	case ANNUAL: return "Annual";
	default: return "";
	}
}

string IRSwap::ToString(FloatingIndex floatingIndex) const
{
	switch (floatingIndex)
	{
	case LIBOR: return "LIBOR";
	case EURIBOR: return "EURIBOR";
	default: return "";
	}
}

string IRSwap::ToString(FloatingIndexTenor floatingIndexTenor) const
{
	switch (floatingIndexTenor)
	{
	case TENOR_1M: return "1m";
	case TENOR_3M: return "3m";
	case TENOR_6M: return "6m";
	case TENOR_12M: return "12m";
	default: return "";
	}
}

string IRSwap::ToString(Currency currency) const
{
	switch (currency)
	{
	case USD: return "USD";
	case EUR: return "EUR";
	case GBP: return "GBP";
	default: return "";
	}
}

string IRSwap::ToString(SwapType swapType) const
{
	switch (swapType)
	{
	case STANDARD: return "Standard";
	case FORWARD: return "Forward";
	case IMM: return "IMM";
	case MAC: return "MAC";
	case BASIS: return "Basis";
	default: return "";
	}
}

string IRSwap::ToString(SwapLegType swapLegType) const
{
	switch (swapLegType)
	{
	case OUTRIGHT: return "Outright";
	case CURVE: return "Curve";
	case FLY: return "Fly";
	default: return "";
	}
}

#endif
