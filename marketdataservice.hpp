// marketdataservice.hpp
//
// Purpose: 1.  Defines the data types and Service for order book market data.
//
// 
// @author Breman Thuraisingham
// @coauthor Yuanting Li
// @version 2.0 2023/12/22 

#ifndef MARKET_DATA_SERVICE_HPP
#define MARKET_DATA_SERVICE_HPP

#include <string>
#include <vector>
#include <algorithm>
#include "soa.hpp"
#include "utilities.hpp"

using namespace std;

// Side for market data
enum PricingSide { BID, OFFER };

/**
 * A market data order with price, quantity, and side.
 */
class Order
{

public:
	// default ctor, need by the map data structure
	Order() = default;

	// ctor for an order
	Order(double _price, long _quantity, PricingSide _side);

	// Get the price on the order
	double GetPrice() const;

	// Get the quantity on the order
	long GetQuantity() const;

	// Get the side on the order
	PricingSide GetSide() const;

private:
	double price;
	long quantity;
	PricingSide side;

};

Order::Order(double _price, long _quantity, PricingSide _side) :
  price(_price), quantity(_quantity), side(_side)
{
}

double Order::GetPrice() const
{
	return price;
}

long Order::GetQuantity() const
{
	return quantity;
}

PricingSide Order::GetSide() const
{
	return side;
}


/**
 * Class representing a bid and offer order
 */
class BidOffer
{

public:

	// ctor for bid/offer
	BidOffer(const Order &_bidOrder, const Order &_offerOrder);

	// Get the bid order
	const Order& GetBidOrder() const;

	// Get the offer order
	const Order& GetOfferOrder() const;

private:
	Order bidOrder;
	Order offerOrder;

};

BidOffer::BidOffer(const Order &_bidOrder, const Order &_offerOrder) :
  bidOrder(_bidOrder), offerOrder(_offerOrder)
{
}

const Order& BidOffer::GetBidOrder() const
{
	return bidOrder;
}

const Order& BidOffer::GetOfferOrder() const
{
	return offerOrder;
}


/**
 * Order book with a bid and offer stack.
 * Type T is the product type.
 */
template<typename T>
class OrderBook
{

public:

	// default ctor, needed for the map data structure
	OrderBook() = default;

	// ctor for the order book
	OrderBook(const T &_product, const vector<Order> &_bidStack, const vector<Order> &_offerStack);

	// Get the product
	const T& GetProduct() const;

	// Get the bid stack
	vector<Order>& GetBidStack();

	// Get the offer stack
	vector<Order>& GetOfferStack();

	// Get the best bid/offer order
	BidOffer BestBidOffer() const;


private:
	T product;
	vector<Order> bidStack;
	vector<Order> offerStack;

};

template<typename T>
OrderBook<T>::OrderBook(const T &_product, const vector<Order> &_bidStack, const vector<Order> &_offerStack) :
  product(_product), bidStack(_bidStack), offerStack(_offerStack)
{
}

template<typename T>
const T& OrderBook<T>::GetProduct() const
{
	return product;
}

template<typename T>
vector<Order>& OrderBook<T>::GetBidStack()
{
	return bidStack;
}

template<typename T>
vector<Order>& OrderBook<T>::GetOfferStack()
{
	return offerStack;
}

template<typename T>
BidOffer OrderBook<T>::BestBidOffer() const
{
	// iterate bid stack and offer stack to find the best bid/offer order
	auto bestBid = std::max_element(bidStack.begin(), bidStack.end(), [](const Order& a, const Order& b) {return a.GetPrice() < b.GetPrice(); });
	auto bestOffer = std::min_element(offerStack.begin(), offerStack.end(), [](const Order& a, const Order& b) {return a.GetPrice() < b.GetPrice(); });
	return BidOffer(*bestBid, *bestOffer);
  
}


// forward declaration of MarketDataConnector
template<typename T>
class MarketDataConnector;


/**
 * Market Data Service which distributes market data
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class MarketDataService : public Service<string,OrderBook <T> >
{
private:
	MarketDataConnector<T>* connector;
	map<string, OrderBook<T>> orderBookMap;
	vector<ServiceListener<OrderBook<T>>*> listeners;
	int bookDepth;
public:
	// ctor and dtor
	MarketDataService();
	~MarketDataService()=default;

	// Get data on our service given a key
	OrderBook<T>& GetData(string key) override;

	// The callback that a Connector should invoke for any new or updated data
	void OnMessage(OrderBook<T>& data) override;

	// Add a listener to the Service for callbacks on add, remove, and update events
	void AddListener(ServiceListener<OrderBook<T>>* listener) override;

	// Get all listeners on the Service
	const vector<ServiceListener<OrderBook<T>>*>& GetListeners() const override;

	// Get the connector
	MarketDataConnector<T>* GetConnector();

	// Get the book depth
	int GetBookDepth() const;


	// Get the best bid/offer order
	const BidOffer& BestBidOffer(const string &productId);

	// Aggregate the order book
	const OrderBook<T>& AggregateDepth(const string &productId);

};


template<typename T>
MarketDataService<T>::MarketDataService() : connector(new MarketDataConnector<T>(this)), bookDepth(5)
{
}

template<typename T>
OrderBook<T>& MarketDataService<T>::GetData(string key)
{
	// if the order book does not exist, create a new one
	if (orderBookMap.find(key) == orderBookMap.end()) 
	{
	orderBookMap.insert(pair<string, OrderBook<T>>(key, OrderBook<T>(QueryProduct<T>(key),vector<Order>(), vector<Order>())));
	}
	return orderBookMap[key];
}

template<typename T>
void MarketDataService<T>::OnMessage(OrderBook<T>& data)
{
	string key = data.GetProduct().GetProductId();
	if (orderBookMap.find(key) != orderBookMap.end()) 
	{ 
		orderBookMap.erase(key); 
	}
	orderBookMap.insert(pair<string, OrderBook<T>>(key, data));


	for (auto& listener : listeners)
	{
	listener->ProcessAdd(data);
	}
}

template<typename T>
void MarketDataService<T>::AddListener(ServiceListener<OrderBook<T>>* listener)
{
	listeners.push_back(listener);
}

template<typename T>
const vector<ServiceListener<OrderBook<T>>*>& MarketDataService<T>::GetListeners() const
{
	return listeners;
}

template<typename T>
MarketDataConnector<T>* MarketDataService<T>::GetConnector()
{
	return connector;
}

template<typename T>
int MarketDataService<T>::GetBookDepth() const
{
	return bookDepth;
}

template<typename T>
const BidOffer& MarketDataService<T>::BestBidOffer(const string &productId)
{
	return orderBookMap[productId].BestBidOffer();
}

template<typename T>
const OrderBook<T>& MarketDataService<T>::AggregateDepth(const string &productId)
{
	// get the order book
	OrderBook<T>& orderBook = orderBookMap[productId];
	// get the bid stack and offer stack
	vector<Order>& bidStack = orderBook.GetBidStack();
	vector<Order>& offerStack = orderBook.GetOfferStack();
	// aggregate the bid stack
	unordered_map<double, long> aggBidMap;
	for (auto& order : bidStack){
	double price = order.GetPrice();
	long quantity = order.GetQuantity();
	if (aggBidMap.find(price) != aggBidMap.end()) {
		aggBidMap[price] += quantity;
	}
	else {
		aggBidMap.insert(pair<double, long>(price, quantity));
	}
	}
	vector<Order> aggBid;
	for (auto& item : aggBidMap) {
	aggBid.push_back(Order(item.first, item.second, BID));
	}

	// aggregate the offer stack
	unordered_map<double, long> aggOfferMap;
	for (auto& order : offerStack) {
	double price = order.GetPrice();
	long quantity = order.GetQuantity();
	if (aggOfferMap.find(price) != aggOfferMap.end()) {
		aggOfferMap[price] += quantity;
	}
	else {
		aggOfferMap.insert(pair<double, long>(price, quantity));
	}
	}
	vector<Order> aggOffer;
	for (auto& item : aggOfferMap) {
	aggOffer.push_back(Order(item.first, item.second, OFFER));
	}
  
	// update the order book
	orderBook = OrderBook<T>(orderBook.GetProduct(), aggBid, aggOffer);
	return orderBook;
}

/**
* Market Data Connector subscribing data to Market Data Service.
* Type T is the product type.
*/
template<typename T>
class MarketDataConnector : public Connector<OrderBook<T>>
{
private:
	MarketDataService<T>* service;

public:
	// ctor
	MarketDataConnector(MarketDataService<T>* _service);
	// dtor
	~MarketDataConnector() = default;

	// Publish data to the Connector
	void Publish(OrderBook<T>& data) override;

	// Subscribe data
	void Subscribe(ifstream& _data);

};

template<typename T>
MarketDataConnector<T>::MarketDataConnector(MarketDataService<T>* _service) :
  service(_service)
{
}

template<typename T>
void MarketDataConnector<T>::Publish(OrderBook<T>& data)
{
}

template<typename T>
void MarketDataConnector<T>::Subscribe(ifstream& _data)
{
	string line;
	getline(_data, line);

	while (getline(_data, line))
	{
		stringstream rawline(line);
		string block;
		vector<string> splitdata;

		// from the raw text split data into blocks
		while (getline(rawline, block, ','))
		{
			splitdata.push_back(block);
		}

		string timestamp = splitdata[0];
		string productID = splitdata[1];
		OrderBook<T>& orderBook = service->GetData(productID);

		for (int i = 0; i < service->GetBookDepth(); i++)
		{
			double bidPrice = Frac2Price(splitdata[4 * i + 2]);
			long bidQuantity = stol(splitdata[4 * i + 3]);
			double askPrice = Frac2Price(splitdata[4 * i + 4]);
			long askQuantity = stol(splitdata[4 * i + 5]);

			Order bidOrder(bidPrice, bidQuantity, BID);
			Order askOrder(askPrice, askQuantity, OFFER);

			orderBook.GetBidStack().push_back(bidOrder);
			orderBook.GetOfferStack().push_back(askOrder);
		}

		OrderBook<T> aggregatedOrderBook = service->AggregateDepth(productID);
		service->OnMessage(aggregatedOrderBook);
	}
}

#endif
